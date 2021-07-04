// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include "Ribbon.h"

// other
#define WM_UPDATE_MYLIST				(WM_USER + 700)
#define WM_SAVE_TREE_STATE			(WM_USER + 701)
#define WM_DATA_UPDATED					(WM_USER + 702)
#define WM_CONNECTION_LOSS			(WM_USER + 703)
#define WM_UPDATE_STATUS_BAR		(WM_USER + 704)

// TREE COMMANDS
#define ID_SERIALIZE							0xC000
#define ID_REMOVE_BRANCH					0xC001
#define ID_GET_CATEGORY_KEEPER		0xC002
#define ID_ADD_EXIST							0xC003
#define ID_CREATE_FILE  					0xC004
#define ID_CREATE_FILTER  				0xC005
#define ID_CHANGE_NAME						0xC006
#define ID_CREATE_ITEM						0xC007
#define ID_ITEM_DELETE						0xC008
#define ID_ITEM_PROPERTIES				0xC009
#define ID_SHOW_ID								0xC00A
// controller
#define ID_SHOW_TT								0xC200
#define ID_SHOW_INTERNAL_DATA			0xC202
#define ID_UPLOAD_CONTROLLER			0xC203
#define ID_UPLOAD_FIRMWARE				0xC204
#define ID_SHOW_RD								0xC205
#define ID_DELETE_RD							0xC206
#define ID_VIRTUAL_START					0xC207
#define ID_VIRTUAL_STOP						0xC208
#define ID_SHOW_AVR_MEMORY				0xC209
#define ID_UPLOAD_IO							0xC20A
#define ID_CREATE_LINK_FROM_PIN		0xC20B
#define ID_DELETE_TT							0xC20C
#define ID_DELETE_I2C							0xC20D
#define ID_ACTIVATE_DEVICE				0xC20E
// Module + Segment
#define ID_UPLOAD_MODULE						0xC303
#define ID_REMOVE_MODULE						0xC304
#define ID_SHOW_ONLINE_LIST					0xC306
#define ID_CREATE_LINK_FROM_MODULE	0xC307
// Algorithm
#define ID_STOP_ALGORITHM						0xC400
#define ID_SHOW_TWI									0xC401
#define ID_ALGORITHM_SHOW_BLOCKS		0xC404
#define ID_UPLOAD_ALL_ALGORITHMS		0xC405
#define ID_DELETE_ADDITIONAL_LIBS		0xC406

// Strings
// EDITOR
#define ID_REMOVE_SLIDER_THING				0xC600
#define ID_SLIDER_ADD_TIME_POINT			0xC601
// WEB
// FROM RIBBON TO TREE
#define RIBBON_OFFSET 0x1000
#define _ID_UPLOAD_ALGORITHM_			(ID_UPLOAD_ALGORITHM+RIBBON_OFFSET)
#define _ID_UNLOAD_ALGORITHM_			(ID_UNLOAD_ALGORITHM+RIBBON_OFFSET)
#define _ID_ALGORITHM_PROPERTIES_		(ID_ALGORITHM_PROPERTIES+RIBBON_OFFSET)