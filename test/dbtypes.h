// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <stdint.h>
#include <limits>

namespace npdb {
using offset_t = uint64_t;
using block_size_t = uint64_t;

constexpr auto invalid_offset = std::numeric_limits<offset_t>::max();
constexpr auto invalid_block_size = std::numeric_limits<block_size_t>::max();

class Database;
struct chunk_meta;

}