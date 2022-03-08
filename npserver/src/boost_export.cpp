// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include "avr_virtual.h"
#include <npsys/network.h>
#include <npsys/control_unit.h>
#include <npsys/boost_export.h>
#include <sim/boost_export.h>

BOOST_CLASS_EXPORT_GUID(VirtualAvrController, "virtualavrcontroller");
BOOST_CLASS_EXPORT_GUID(VirtualAvrPCLINK, "virtualavrpclink");