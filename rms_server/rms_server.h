#pragma once

#include <vector>
#include <thread>
#include <algorithm>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include "rms_message.h"
#include "buffer_queue.h"

#pragma comment(lib, "Ws2_32")

class rms_server : public buffer_queue {
public:
	~rms_server()
	{
		m_shutdown = true;
		//if (m_eventWake && m_threadSendQueueFeed) {
		SetEvent(m_eventWake);
		auto ret = WaitForSingleObject(m_threadSendQueueFeed, INFINITE);
		CloseHandle(m_threadSendQueueFeed);
		//}

		WSACloseEvent(m_eventSrv);
		closesocket(m_sockSrv);

		WSACloseEvent(m_eventClnt);
		shutdown(m_sockClnt, SD_BOTH);
		closesocket(m_sockClnt);

		ret = WaitForSingleObject(m_threadSockEventFeed, INFINITE);
		CloseHandle(m_threadSockEventFeed);

		WSACleanup();
	}

	static rms_server* Create(const uint16_t& port)
	{
		if (!utility::allow_inbound_on_port(port, L"RMS_Server", true)) {
			return nullptr;
		}

		auto srv = new (std::nothrow) rms_server();
		if (!srv) return nullptr;

		WSADATA wsaData;
		auto ret = WSAStartup(WINSOCK_VERSION, &wsaData);
		if (ret) {
			goto Fail;
		}

		srv->m_sockSrv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (srv->m_sockSrv == INVALID_SOCKET) {
			goto Fail;
		}

		sockaddr_in srv_addr;
		srv_addr.sin_family = AF_INET;
		srv_addr.sin_port = htons(port);
		srv_addr.sin_addr.s_addr = INADDR_ANY;
		ret = bind(srv->m_sockSrv, reinterpret_cast<sockaddr*>(&srv_addr), sizeof(srv_addr));
		if (ret == SOCKET_ERROR) {
			goto Fail;
		}

		srv->m_eventWake = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		srv->m_eventClnt = WSACreateEvent();
		srv->m_eventSrv = WSACreateEvent();
		if (srv->m_eventClnt == WSA_INVALID_EVENT || srv->m_eventSrv == WSA_INVALID_EVENT || !srv->m_eventWake) {
			goto Fail;
		}

		ret = WSAEventSelect(srv->m_sockSrv, srv->m_eventSrv, FD_ACCEPT);
		if (ret == SOCKET_ERROR) {
			goto Fail;
		}

		DWORD threadId;
		srv->m_threadSockEventFeed = CreateThread(nullptr, 0, socket_event_feed_routine, srv, 0, &threadId);
		if (!srv->m_threadSockEventFeed) {
			goto Fail;
		}

		std::cout << "socket_event_feed_thread_id: " << threadId << std::endl;

		ret = listen(srv->m_sockSrv, 1);
		if (ret == SOCKET_ERROR) {
			goto Fail;
		}

		return srv;

	Fail:
		delete srv;

		return nullptr;
	}

	void send_msg(rms_msg::rms_msg_item& msg)
	{
		std::lock_guard<std::recursive_mutex> lg(m_sendQueueLock);
		m_sendQueue.emplace(std::move(msg));
		if (m_sendQueue.size() == 1) {
			SetEvent(m_eventWake);
		}
	}

	bool send_msg(const void* data, const size_t& size)
	{
		std::lock_guard<std::recursive_mutex> lg(m_sendQueueLock);
		rms_msg::rms_msg_item msg_item;
		if (!rms_msg::make_msg(data, size, msg_item)) {
			return false;
		}
		send_msg(msg_item);
		return true;
	}

	bool is_connected() const
	{
		return m_sockClnt != INVALID_SOCKET;
	}

	bool get_client_address(sockaddr_in& addr) const
	{
		if (!is_connected()) return false;

		int addrlen = sizeof(addr);
		return getpeername(m_sockClnt, reinterpret_cast<sockaddr*>(&addr), &addrlen) != SOCKET_ERROR;
	}

private:
	rms_server() : buffer_queue(1024 * 1024), m_sockClnt(INVALID_SOCKET), m_sockSrv(INVALID_SOCKET), m_eventClnt(WSA_INVALID_EVENT), m_eventSrv(WSA_INVALID_EVENT) {}

	bool do_client_event_process()
	{
		WSANETWORKEVENTS events;
		auto ret = WSAEnumNetworkEvents(m_sockClnt, m_eventClnt, &events);
		if (ret == SOCKET_ERROR) {
			return false;
		}

		if (events.lNetworkEvents & FD_READ) {
			if (events.iErrorCode[FD_READ_BIT]) {
				std::cerr << "read error code: " << events.iErrorCode[FD_READ_BIT] << std::endl;
				return true;
			} else {
				//std::ofstream ofs;
				//ofs.open("1.dat", std::ios_base::binary | std::ios_base::app);
				std::vector<char> buf;
				buf.resize(8192);
				ret = recv(m_sockClnt, buf.data(), buf.size(), 0);
				while (ret > 0) {
					//ofs.write(buf.data(), ret);
					if (!push(buf.data(), ret)) {
						return false;
					}
					if (ret != buf.size()) return true;
					ret = recv(m_sockClnt, buf.data(), buf.size(), 0);
				}

				return false;
			}
		} else if (events.lNetworkEvents & FD_CLOSE) {
			std::cerr << "close error code: " << events.iErrorCode[FD_CLOSE_BIT] << std::endl;
			m_shutdown = true;
			SetEvent(m_eventWake);
			WaitForSingleObject(m_threadSendQueueFeed, INFINITE);
			std::cout << "m_sendQueue.size(): " << m_sendQueue.size() << std::endl;
			return false;
		} else return true;
	}

	static DWORD WINAPI send_queue_feed_routine(LPVOID Param)
	{
		auto srv = reinterpret_cast<rms_server*>(Param);

		std::cout << "srv->m_sendQueue.empty(): " << srv->m_sendQueue.empty() << std::endl;
		for (;;) {
			if (srv->m_sendQueue.empty()) {
				if (srv->m_shutdown) {
					srv->m_shutdown = false;
					break;
				}
				WaitForSingleObject(srv->m_eventWake, INFINITE);
				continue;
			}

			srv->m_sendQueueLock.lock();
			auto front = std::move(srv->m_sendQueue.front());
			srv->m_sendQueue.pop();
			srv->m_sendQueueLock.unlock();

			if (srv->m_sockClnt != INVALID_SOCKET) {
				auto ret = send(srv->m_sockClnt, reinterpret_cast<char*>(front.second), front.first, 0);
				if (ret != front.first) {
					shutdown(srv->m_sockClnt, SD_BOTH);
					closesocket(srv->m_sockClnt);
					srv->m_sockClnt = INVALID_SOCKET;
				}
			}
			rms_msg::free_msg(front);
		}

		std::cout << "send_queue_feed_routine quit!" << std::endl;
		ExitThread(0);
		//return 0;
	}

	static DWORD WINAPI socket_event_feed_routine(LPVOID Param)
	{
		auto srv = reinterpret_cast<rms_server*>(Param);
		const HANDLE eventArray[] = { srv->m_eventSrv, srv->m_eventClnt };

		for (;;) {
			auto status = WSAWaitForMultipleEvents(2, eventArray, FALSE, 1000, FALSE);
			if (status == WSA_WAIT_TIMEOUT) {
				continue;
			}
			if (status == WSA_WAIT_FAILED) {
				shutdown(srv->m_sockClnt, SD_BOTH);
				closesocket(srv->m_sockClnt);
				srv->m_sockClnt = INVALID_SOCKET;
				break;
			}

			if (status == WSA_WAIT_EVENT_0 + 1) {
				if (!srv->do_client_event_process()) {
					shutdown(srv->m_sockClnt, SD_BOTH);
					closesocket(srv->m_sockClnt);
					srv->m_sockClnt = INVALID_SOCKET;
				}

				continue;
			} else if (status != WSA_WAIT_EVENT_0) {
				std::cerr << "wrong status: " << status << std::endl;
				break;
			}

			WSANETWORKEVENTS events;
			auto ret = WSAEnumNetworkEvents(srv->m_sockSrv, srv->m_eventSrv, &events);
			if (ret == SOCKET_ERROR) {
				break;
			}

			if (!(events.lNetworkEvents & FD_ACCEPT)) {
				std::cerr << "wrong events returned: " << events.lNetworkEvents << std::endl;
				break;
			}

			if (events.iErrorCode[FD_ACCEPT_BIT]) {
				std::cerr << "error code: " << events.iErrorCode[FD_ACCEPT_BIT] << std::endl;
				continue;
			}

			sockaddr_in clnt_addr;
			int addr_len = sizeof(clnt_addr);
			auto sock = accept(srv->m_sockSrv, reinterpret_cast<sockaddr*>(&clnt_addr), &addr_len);
			if (sock == INVALID_SOCKET) {
				continue;
			}

			if (srv->m_sockClnt != INVALID_SOCKET) {
				shutdown(sock, SD_BOTH);
				closesocket(sock);
				std::cout << "ejected: " << inet_ntoa(clnt_addr.sin_addr) << ":" << ntohs(clnt_addr.sin_port) << std::endl;
				continue;	// eject
			}

			ret = WSAEventSelect(sock, srv->m_eventClnt, FD_READ | FD_CLOSE);
			if (ret == SOCKET_ERROR) {
				shutdown(sock, SD_BOTH);
				closesocket(sock);
				continue;
			}

			DWORD threadId;
			srv->m_threadSendQueueFeed = CreateThread(nullptr, 0, send_queue_feed_routine, srv, 0, &threadId);
			if (!srv->m_threadSendQueueFeed) {
				shutdown(sock, SD_BOTH);
				closesocket(sock);
				//srv->m_sockClnt = INVALID_SOCKET;
				continue;
			}

			srv->m_sockClnt = sock;

			std::cout << "send_queue_feed_routine_thread_id: " << threadId << std::endl;
		}

		std::cout << "socket_event_feed_routine quit!" << std::endl;
		ExitThread(0);
		//return 0;
	}

	SOCKET m_sockSrv, m_sockClnt;
	HANDLE m_eventSrv, m_eventClnt, m_eventWake = nullptr;
	HANDLE m_threadSockEventFeed = nullptr, m_threadSendQueueFeed = nullptr;
	std::queue<rms_msg::rms_msg_item> m_sendQueue;
	std::recursive_mutex m_sendQueueLock;
	bool m_shutdown = false;

	static size_t m_locked;
};

/*
* static bool do_read_header(const SOCKET& sock, rms_msg::rms_msg_item& item)
	{
		std::vector<uint8_t> header_buf;
		header_buf.resize(sizeof(rms_msg));

		auto ret = recv(sock, reinterpret_cast<char*>(header_buf.data()), header_buf.size(), 0);
		if (ret == SOCKET_ERROR) {
			return false;
		}

		if (ret == 0) {
			return false;
		}

		header_buf.resize(header_buf.size() + ret);

		if (ret != header_buf.size()) {
			return true;
		}

		auto bRet = rms_msg::probe_msg_header(header_buf, item);
		if (!bRet) {
			// drop
			ret = recv(sock, reinterpret_cast<char*>(header_buf.data() + header_buf.size()), header_buf.capacity() - header_buf.size(), MSG_WAITALL);
			if (ret == SOCKET_ERROR) {
				return false;
			}
		}

		return true;
	}

	static bool do_client_read(const SOCKET& sock)
	{
		static int phase = 0;

		static std::vector<uint8_t> header_buf, data_buf;
		static std::vector<char> drop_buf;
		rms_msg::rms_msg_item item;
		header_buf.reserve(sizeof(rms_msg));
		drop_buf.resize(sizeof(rms_msg));

		switch (phase) {
		case 0:
			if (do_read_header(sock, item)) {
				phase = 1;
			}
			break;
		case 1:
			break;
		}

		return true;
	}
*/