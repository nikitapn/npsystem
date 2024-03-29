// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#define MAX_CTRLS					32
#define MAX_REG_IN_REQ		13

// Communication Errors
#define S_COMMUNICATION_ERROR_DEVICE_NOT_EXIST				-7
#define S_COMMUNICATION_ERROR_SOCKET									-8
#define S_COMMUNICATION_ERROR_SERVER_STOPPED					-9
#define S_COMMUNICATION_ERROR_DEVICE_TIMEOUT					-10
#define S_COMMUNICATION_ERROR_PC_LINK_TIMEOUT					-11
#define S_COMMUNICATION_ERROR_CRC											-12
#define S_COMMUNICATION_ERROR_CRC_RECIVE							-13
#define S_COMMUNICATION_ERROR_IN_FRAMESTRUCT					-14
#define S_COMMUNICATION_ERROR_UNEXPECTED_FRAMELENGHT	-15
#define S_COMMUNICATION_ERROR_UNEXPECTED_FRAME				-16
#define S_COMMUNICATION_ERROR_NO_BUFFER_SIZE					-17
// SendPage
#define	S_ERROR_PAGE_NOT_VALID												-18
// Replace
#define S_ERROR_NEW_ADDRESS_EXISTS										-19
#define S_ERROR_OLD_ADDRESS_NOT_FOUND									-20
#define S_ERROR_ALGORITHM_NOT_FOUND										-21
// Remove
#define S_ERROR_ADDRESS_NOT_FOUND											-22
// 
#define S_ERROR_DEVICE_INACCESSIBLE										-23
#define S_ERROR_VERIFYING															-24