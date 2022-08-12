#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include "host_probe.h"
#include "rms_server.h"
#include "zip.h"
#include "utility.h"
#include "image_viewer.h"

using json = nlohmann::json;

int main()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	// Initialize GDI+.
	auto status = GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
	if (status != Gdiplus::Ok) {
		utility::print_debug_msg(L"main", { L"GdiplusStartup" }, status);
		return -1;
	}

	auto server = rms_server::Create(10086);
	auto probe_client = host_probe_client::Create(10086);
	auto probe_msg = host_probe_msg::Create();
	auto img_viewer = image_viewer::Create();

	//std::locale::global(std::locale("en_US.UTF-8", std::locale::ctype));

	while (!server->is_connected()) {
		probe_client->send_probe_msg(probe_msg);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	sockaddr_in clnt_addr;
	server->get_client_address(clnt_addr);
	std::cout << "client: " << inet_ntoa(clnt_addr.sin_addr) << ":" << ntohs(clnt_addr.sin_port) << std::endl;

	std::queue<json> json_queue;
	std::mutex json_queue_mutex;

	auto msg_parse_thread = [&server, &json_queue, &json_queue_mutex]() {
		rms_msg msg_hdr;
		for (; server && server->is_connected();) {
			if (!rms_msg::probe_msg_header(server, &msg_hdr)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				continue;
			}

			while (server->get_total() < msg_hdr.data_size && server->is_connected()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				continue;
			}

			if (server->get_total() < msg_hdr.data_size) {
				continue;
			}

			std::vector<uint8_t> data;
			std::vector<uint8_t> body;
			data.resize(msg_hdr.data_size);
			server->get(data.size(), data.data());
			if (msg_hdr.compressed) {
				if (!zip::decompress_zlib(data.data(), data.size(), msg_hdr.raw_size, body)) {
					continue;
				}
			} else {
				body = std::move(data);
			}

			std::ofstream ofs;
			ofs.open(std::to_string(time(nullptr)).append(".json"), std::ios_base::binary);
			ofs.write((char*)body.data(), body.size());
			ofs.close();

			if (!json::accept(body, true)) {
				continue;
			}

			json_queue_mutex.lock();
			json_queue.emplace(std::move(json::parse(body, nullptr, false, true)));
			json_queue_mutex.unlock();
		}
	};

	std::thread(msg_parse_thread).detach();

	json j_cmd;
	j_cmd["type"] = 0;

	while (server->is_connected()) {
		std::cout << "\n>";
		std::string cmd;
		if (!std::getline(std::cin, cmd)) break;
		if (cmd.empty()) continue;
		while (!cmd.empty() && cmd.front() == ' ') {
			cmd.erase(cmd.cbegin());
		}
		if (cmd.empty()) continue;
		while (!cmd.empty() && cmd.back() == ' ') {
			cmd.pop_back();
		}
		if (cmd.empty()) continue;

		if (cmd == "cap_scr") {
			j_cmd["data"]["cmd"] = 1;
		} else if (cmd == "cap_cam") {
			j_cmd["data"]["cmd"] = 2;
		} else if (cmd == "help") {
			std::cout << "help cap_scr cap_cam ls_files exit\n";
			continue;
		} else if (cmd == "exit") {
			break;
		} else {
			auto sep_idx = cmd.find_first_of(' ');
			if (sep_idx == std::string::npos) {
				continue;
			}

			auto param = cmd.substr(sep_idx + 1);
			cmd = cmd.substr(0, sep_idx);
			if (cmd != "ls_files") {
				continue;
			}

			j_cmd["data"]["cmd"] = 3;
			j_cmd["data"]["path"] = param;
		}

		auto s = std::move(j_cmd.dump());
		rms_msg::rms_msg_item msg;
		rms_msg::make_msg(s.data(), s.size(), msg);
		server->send_msg(msg);
		//rms_msg::free_msg(msg);

		while (server->is_connected() && json_queue.empty()) {
			std::this_thread::sleep_for(std::chrono::microseconds(50));
		}

		if (json_queue.empty()) continue;

		json_queue_mutex.lock();
		auto j = std::move(json_queue.front());
		json_queue.pop();
		json_queue_mutex.unlock();

		if (j["type"] != 3) {
			std::cout << j.dump(2) << std::endl;
			continue;
		}

		std::vector<uint8_t> bytes;// = j["data"];
		utility::hex_string_to_bytes(j["data"], bytes);
		std::ofstream ofs;
		auto img_path = std::to_string(time(nullptr)).append(".jpeg");
		ofs.open(img_path, std::ios_base::binary);
		ofs.write((char*)bytes.data(), bytes.size());
		ofs.close();
		img_viewer->show(img_path.c_str());
		//if (j_cmd["data"]["cmd"] == 1) {
			//j_cmd["data"]["cmd"] = 2;
		//} else {
			//j_cmd["data"]["cmd"] = 1;
		//}
		//auto s = std::move(j_cmd.dump());
		//rms_msg::rms_msg_item msg;
		//rms_msg::make_msg(s.data(), s.size(), msg);
		//server->send_msg(msg);
		//rms_msg::free_msg(msg);
	}

	auto tmp = server;
	server = nullptr;
	delete tmp;
	delete probe_client;
	delete img_viewer;

	//std::cin.get();
}
