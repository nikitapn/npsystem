// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#define BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE

#include <iostream>
#include <iomanip>
#include <stdint.h>
#include <string>
#include <array>
#include <set>
#include <map>
#include <memory>
#include <ctime>
#include <cassert>

#include <boost/noncopyable.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <boost/serialization/unique_ptr.hpp>

#include <boost/pool/pool.hpp>
#include <boost/asio/buffer.hpp>

#include <thread>
#include <atomic>
