#pragma once

#include <iostream>
#include <map>
#include <Windows.h>
#include <gdiplus.h>
#include <wtypes.h>

LPCSTR prop_tag_to_string(PROPID tag)
{
#define MAKE_TAG_MAP_ITEM(_tag) {_tag, #_tag}
	const std::map<PROPID, LPCSTR> tag_map = {
		MAKE_TAG_MAP_ITEM(PropertyTagImageWidth),
		MAKE_TAG_MAP_ITEM(PropertyTagImageHeight),
		MAKE_TAG_MAP_ITEM(PropertyTagBitsPerSample),
		MAKE_TAG_MAP_ITEM(PropertyTagCompression),
		MAKE_TAG_MAP_ITEM(PropertyTagPhotometricInterp),
		MAKE_TAG_MAP_ITEM(PropertyTagOrientation),
		MAKE_TAG_MAP_ITEM(PropertyTagSamplesPerPixel),
		MAKE_TAG_MAP_ITEM(PropertyTagXResolution),
		MAKE_TAG_MAP_ITEM(PropertyTagYResolution),
		MAKE_TAG_MAP_ITEM(PropertyTagResolutionUnit),
		MAKE_TAG_MAP_ITEM(PropertyTagSoftwareUsed),
		MAKE_TAG_MAP_ITEM(PropertyTagREFBlackWhite),
		MAKE_TAG_MAP_ITEM(PropertyTagDateTime),
		MAKE_TAG_MAP_ITEM(PropertyTagNewSubfileType),
		MAKE_TAG_MAP_ITEM(PropertyTagGpsGpsTime),
		MAKE_TAG_MAP_ITEM(PropertyTagGpsVer),
		MAKE_TAG_MAP_ITEM(PropertyTagGpsDate),
		MAKE_TAG_MAP_ITEM(PropertyTagGpsLatitudeRef),
		MAKE_TAG_MAP_ITEM(PropertyTagGpsLatitude),
		MAKE_TAG_MAP_ITEM(PropertyTagGpsLongitudeRef),
		MAKE_TAG_MAP_ITEM(PropertyTagGpsLongitude),
		MAKE_TAG_MAP_ITEM(PropertyTagGpsProcessingMethod),
		MAKE_TAG_MAP_ITEM(PropertyTagExifVer),
		MAKE_TAG_MAP_ITEM(PropertyTagExifColorSpace),
		MAKE_TAG_MAP_ITEM(PropertyTagExifPixXDim),
		MAKE_TAG_MAP_ITEM(PropertyTagExifPixYDim),
		MAKE_TAG_MAP_ITEM(PropertyTagThumbnailData),
		MAKE_TAG_MAP_ITEM(PropertyTagThumbnailCompression),
		MAKE_TAG_MAP_ITEM(PropertyTagThumbnailResolutionX),
		MAKE_TAG_MAP_ITEM(PropertyTagThumbnailResolutionY),
		MAKE_TAG_MAP_ITEM(PropertyTagThumbnailResolutionUnit),
		MAKE_TAG_MAP_ITEM(PropertyTagJPEGInterFormat),
		MAKE_TAG_MAP_ITEM(PropertyTagJPEGInterLength),
		MAKE_TAG_MAP_ITEM(PropertyTagJPEGProc),
		MAKE_TAG_MAP_ITEM(PropertyTagLuminanceTable),
		MAKE_TAG_MAP_ITEM(PropertyTagChrominanceTable),
		MAKE_TAG_MAP_ITEM(PropertyTagImageDescription),
		MAKE_TAG_MAP_ITEM(PropertyTagArtist),
		MAKE_TAG_MAP_ITEM(PropertyTagCopyright),
		MAKE_TAG_MAP_ITEM(PropertyTagEquipMake),
		MAKE_TAG_MAP_ITEM(PropertyTagEquipModel),
		MAKE_TAG_MAP_ITEM(PropertyTagYCbCrPositioning),
		MAKE_TAG_MAP_ITEM(PropertyTagExifExposureTime),
		MAKE_TAG_MAP_ITEM(PropertyTagExifFNumber),
		MAKE_TAG_MAP_ITEM(PropertyTagExifExposureProg),
		MAKE_TAG_MAP_ITEM(PropertyTagExifISOSpeed),
		MAKE_TAG_MAP_ITEM(PropertyTagExifDTOrig),
		MAKE_TAG_MAP_ITEM(PropertyTagExifDTDigitized),
		MAKE_TAG_MAP_ITEM(PropertyTagExifShutterSpeed),
		MAKE_TAG_MAP_ITEM(PropertyTagExifAperture),
		MAKE_TAG_MAP_ITEM(PropertyTagExifBrightness),
		MAKE_TAG_MAP_ITEM(PropertyTagExifExposureBias),
		MAKE_TAG_MAP_ITEM(PropertyTagExifMaxAperture),
		MAKE_TAG_MAP_ITEM(PropertyTagExifMeteringMode),
		MAKE_TAG_MAP_ITEM(PropertyTagExifLightSource),
		MAKE_TAG_MAP_ITEM(PropertyTagExifFlash),
		MAKE_TAG_MAP_ITEM(PropertyTagExifFocalLength),
		MAKE_TAG_MAP_ITEM(PropertyTagExifFocalXRes),
		MAKE_TAG_MAP_ITEM(PropertyTagExifFocalYRes),
		MAKE_TAG_MAP_ITEM(PropertyTagExifFocalResUnit),
		MAKE_TAG_MAP_ITEM(PropertyTagExifFileSource),
		MAKE_TAG_MAP_ITEM(PropertyTagExifSceneType),
		MAKE_TAG_MAP_ITEM(PropertyTagExifCustomRendered),
		MAKE_TAG_MAP_ITEM(PropertyTagExifExposureMode),
		MAKE_TAG_MAP_ITEM(PropertyTagExifWhiteBalance),
		MAKE_TAG_MAP_ITEM(PropertyTagExifDigitalZoomRatio),
		MAKE_TAG_MAP_ITEM(PropertyTagExifFocalLengthIn35mmFilm),
		MAKE_TAG_MAP_ITEM(PropertyTagExifSceneCaptureType),
		MAKE_TAG_MAP_ITEM(PropertyTagExifContrast),
		MAKE_TAG_MAP_ITEM(PropertyTagExifSaturation),
		MAKE_TAG_MAP_ITEM(PropertyTagExifSharpness),
		MAKE_TAG_MAP_ITEM(PropertyTagICCProfile),
		MAKE_TAG_MAP_ITEM(PropertyTagExifCompConfig),
		MAKE_TAG_MAP_ITEM(PropertyTagExifUserComment),
		MAKE_TAG_MAP_ITEM(PropertyTagExifDTSubsec),
		MAKE_TAG_MAP_ITEM(PropertyTagExifDTOrigSS),
		MAKE_TAG_MAP_ITEM(PropertyTagExifDTDigSS),
		MAKE_TAG_MAP_ITEM(PropertyTagExifFPXVer),
		MAKE_TAG_MAP_ITEM(PropertyTagExifSensingMethod),
		MAKE_TAG_MAP_ITEM(PropertyTagExifCfaPattern),
		MAKE_TAG_MAP_ITEM(PropertyTagExifGainControl),
		MAKE_TAG_MAP_ITEM(PropertyTagExifSubjectDistanceRange),
		MAKE_TAG_MAP_ITEM(PropertyTagExifUniqueImageID),
		{0x1001, "RelatedImageWidth"},
		{0x1002, "RelatedImageHeight"},
		{0x4746, "Rating"},
		{0x4749, "RatingPercent"},
		{0x1002, "RelatedImageHeight"},
		{0x8830, "SensitivityType"},
		{0x8832, "RecommendedExposureIndex"},
		{0x9010, "OffsetTime"},
		{0x9011, "OffsetTimeOriginal"},
		{0x9012, "OffsetTimeDigitized"},
		{0x9c9b, "XPTitle"},
		{0x9c9c, "XPComment"},
		{0x9c9d, "XPAuthor"},
		{0x9c9e, "XPKeywords"},
		{0x9c9f, "XPSubject"},
		{0xa430, "OwnerName"},
		{0xa431, "SerialNumber"},
		{0xa432, "LensInfo"},
		{0xa433, "LensMake"},
		{0xa434, "LensModel"},
		{0xa435, "LensSerialNumber"}
	};

	auto item = tag_map.find(tag);
	if (item == tag_map.cend()) return nullptr;

	return item->second;
}

LPCSTR prop_tag_type_to_string(WORD type)
{
#define MAKE_TAG_TYPE_MAP_ITEM(_type) {_type, #_type}
	const std::map<DWORD, LPCSTR> type_map = {
		MAKE_TAG_TYPE_MAP_ITEM(PropertyTagTypeByte),
		MAKE_TAG_TYPE_MAP_ITEM(PropertyTagTypeASCII),
		MAKE_TAG_TYPE_MAP_ITEM(PropertyTagTypeShort),
		MAKE_TAG_TYPE_MAP_ITEM(PropertyTagTypeLong),
		MAKE_TAG_TYPE_MAP_ITEM(PropertyTagTypeRational),
		MAKE_TAG_TYPE_MAP_ITEM(PropertyTagTypeUndefined),
		MAKE_TAG_TYPE_MAP_ITEM(PropertyTagTypeSLONG),
		MAKE_TAG_TYPE_MAP_ITEM(PropertyTagTypeSRational)
	};

	auto item = type_map.find(type);
	if (item == type_map.cend()) return nullptr;

	return item->second;
}

template <typename _t>
void print_prop_item(const Gdiplus::PropertyItem* item)
{
	auto count = item->length / sizeof(_t);
	for (decltype(count) i = 0; i < count; ++i) {
		std::cout << ((_t*)item->value)[i];
		if (i + 1 != count) {
			std::cout << "-";
		}
	}
}

void print_image_info(Gdiplus::Image* image)
{
	UINT totalSize, propCount;

	if (image->GetPropertySize(&totalSize, &propCount) != Gdiplus::Ok || !totalSize || !propCount) return;

	Gdiplus::PropertyItem* allItems = (Gdiplus::PropertyItem*)new(std::nothrow) char[totalSize];
	if (!allItems) return;

	if (image->GetAllPropertyItems(totalSize, propCount, allItems) != Gdiplus::Ok) {
		delete[] allItems;
		return;
	}

	for (decltype(propCount) i = 0; i < propCount; ++i) {
		auto item = allItems[i];

		std::cout << i << ": ";
		auto strId = prop_tag_to_string(item.id);
		if (strId) {
			std::cout << strId;
		} else {
			std::cout << std::showbase << std::hex << item.id << std::dec << std::noshowbase;
		}

		std::cout << ", ";
		if (item.type == PropertyTagTypeASCII) {
			std::cout << (char*)item.value;
		} else if (item.type == PropertyTagTypeShort) {
			print_prop_item<uint16_t>(&item);
		} else if (item.type == PropertyTagTypeLong) {
			print_prop_item<uint32_t>(&item);
		} else if (item.type == PropertyTagTypeSLONG) {
			print_prop_item<int32_t>(&item);
		} else if (item.type == PropertyTagTypeRational) {
			//item.type = PropertyTagTypeLong;
			auto ptrULong = (uint32_t*)item.value;
			std::cout << ptrULong[0] << "/" << ptrULong[1];
			//print_prop_item<long>(propItem);
		} else if(item.type == PropertyTagTypeSRational) {
			auto ptrLong = (int32_t*)item.value;
			std::cout << ptrLong[0] << "/" << ptrLong[1];
		} else if (item.id == PropertyTagExifVer) {
			auto ptrChar = (char*)item.value;
			std::cout << ptrChar[0] << ptrChar[1] << ptrChar[2] << ptrChar[3];
		} else if (item.type == PropertyTagTypeUndefined) {
			auto ptrChar = (char*)item.value;
			for (DWORD i = 0; i < item.length; ++i) {
				std::cout << (unsigned)ptrChar[i];
				if (i + 1 != item.length) {
					std::cout << "-";
				}
			}
		} else {
			auto strType = prop_tag_type_to_string(item.type);
			if (strType) {
				std::cout << strType;
			} else {
				std::cout << item.type;
			}
			std::cout << ", " << item.length;
		}

		std::cout << std::endl;
	}

	delete[] allItems;
}
