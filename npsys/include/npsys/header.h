// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <npdb/db.h>

class CFBDBlockComposition;
class CSFCBlockComposition;
class VirtualDevice;
class VirtualController;
class VirtualAvrPCLINK;

namespace npsys {
enum StaticId : oid_t {
	SYSTEM_ID						= 100,
	LIBRARIES_ID				= 200,
	MODULES_ID					= 300,
	I2C_MODULES_ID,
	ALGORITHMS_ID				= 400,
	ALGORITHMS_CATEGORIES_ID,
	CONTROLLERS_ID			= 500,
	SERVER_ID						= 600,
	NETWORK_ID_					= 700,
	VIRTUAL_DEVICES_ID	= 800,
	VIRTUAL_PC_LINK_ID	= 900,
	VIRTUAL_PC_LINK_SERVER_ID,
	HISTORY_LIST_ID			= 1000,
	STRINGS							= 1100,
	WEB									= 1200,
};

// nodes
using system_n = odb::unique_node<class CSystem, SYSTEM_ID>;
using i2c_module_n = odb::shared_node<class CI2CModule>;
using i2c_module_l = odb::node_list<i2c_module_n, I2C_MODULES_ID>;
using i2c_assigned_module_n = odb::shared_node<class CAssignedI2CModule>;
using i2c_assigned_module_l = odb::node_list<i2c_assigned_module_n>;

using categories_n = odb::shared_node<class CAlgorithms, ALGORITHMS_ID>;
using algorithm_category_n = odb::shared_node<class CAlgorithmCat>;
using algorithm_category_l = odb::node_list<npsys::algorithm_category_n, npsys::ALGORITHMS_CATEGORIES_ID>;

// network

using device_n = odb::shared_node<class CNetworkDevice>;
using server_n = odb::shared_node<class CServer, SERVER_ID>;
using virtual_pc_link_n = odb::shared_node<class CVirtualAVRPCLINK, VIRTUAL_PC_LINK_ID>;
template<typename T>
using controller_n_t = odb::shared_node<T>;
using controller_n = controller_n_t<class CController>;
using controller_n_avr = controller_n_t<class CAVRController>;
using controllers_l = odb::node_list<controller_n, CONTROLLERS_ID>;
using avr_pin_n = odb::shared_node<class CAVRPin>;
using avr_port_n = odb::shared_node<class CAVRPort>;
using control_unit_n = odb::shared_node<class CControlUnit>;
using control_unit_l = odb::node_list<control_unit_n>;
using fbd_control_unit_n = odb::shared_node<class CFBDControlUnit>;
using sfc_control_unit_n = odb::shared_node<class CSFCControlUnit>;

using assigned_algorithm_n = odb::shared_node<class CAssignedAlgorithm>;
using avr_assigned_algorithm_n = odb::shared_node<class CAVRAssignedAlgorithm>;
using assigned_algorithms_l = odb::node_list<assigned_algorithm_n>;
using avr_assigned_object_file_n = odb::shared_node<class CAVRAssignedObjectFile>;
using avr_assigned_object_files_l = odb::node_list<avr_assigned_object_file_n>;

using block_composition_n = odb::unique_node<CFBDBlockComposition>;
using sfc_block_composition_n = odb::unique_node<CSFCBlockComposition>;
using fbd_block_n = odb::shared_node<class CFBDBlock, odb::INVALID_ID, odb::special_serialization>;
using fbd_slot_n = odb::shared_node<class CFBDSlot, odb::INVALID_ID, odb::special_serialization>;
using variable_n = odb::shared_node<class variable, odb::INVALID_ID, odb::special_serialization>;
using history_l = odb::node_list<fbd_slot_n, HISTORY_LIST_ID>;

using virtual_device_n = odb::unique_node<VirtualDevice>;
using virtual_controller_n = odb::unique_node<VirtualController>;
using virtual_controllers_l = odb::node_list<virtual_controller_n, VIRTUAL_DEVICES_ID>;
using virtual_pc_link_serv_n = odb::unique_node<VirtualAvrPCLINK, VIRTUAL_PC_LINK_SERVER_ID>;


using strings_n = odb::shared_node<class Strings>;
using strings_l = odb::node_list<strings_n, STRINGS>;

using web_item_n = odb::shared_node<class WebItem>;
using web_item_l = odb::node_list<web_item_n>;

using web_cat_n = odb::shared_node<class WebCategory>;
using web_l = odb::node_list<web_cat_n, WEB>;
} // namespace npsys


