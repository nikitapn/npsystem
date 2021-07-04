// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "../npserver/src/server.h"

std::string_view TranslateError(uint32_t code) noexcept {
	switch (code) {
	case S_COMMUNICATION_ERROR_SOCKET:
		return "Socket error"sv;
	case S_COMMUNICATION_ERROR_SERVER_STOPPED:
		return "Server is not working"sv;
	case S_COMMUNICATION_ERROR_DEVICE_TIMEOUT:
		return "Device Timeout"sv;
	case S_COMMUNICATION_ERROR_CRC:
		return "Bad crc"sv;
	case S_COMMUNICATION_ERROR_CRC_RECIVE:
		return "Bad crc recived"sv;
	case S_COMMUNICATION_ERROR_NO_BUFFER_SIZE:
		return "No buffer size"sv;
	case S_COMMUNICATION_ERROR_IN_FRAMESTRUCT:
		return "Framestruct"sv;
	case S_COMMUNICATION_ERROR_UNEXPECTED_FRAMELENGHT:
		return "Unexpected frame lenght"sv;
	case S_COMMUNICATION_ERROR_UNEXPECTED_FRAME:
		return "Unexpected frame"sv;
	//
	case S_ERROR_PAGE_NOT_VALID:
		return "Page is not valid"sv;
	case S_ERROR_NEW_ADDRESS_EXISTS:
		return "Address exists"sv;
	case S_ERROR_OLD_ADDRESS_NOT_FOUND:
		return "Old address not found"sv;
	case S_ERROR_ADDRESS_NOT_FOUND:
		return "Address not found"sv;
	case S_ERROR_DEVICE_INACCESSIBLE:
		return "Device is inaccessible"sv;
	case S_ERROR_VERIFYING:
		return "Error Verifying"sv;
	default:
		return "Unknown Error"sv;
	}
}
