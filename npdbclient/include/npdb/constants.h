// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#ifndef __NPDB_CONSTANTS_H__
#define __NPDB_CONSTANTS_H__

#include <stdint.h>

using oid_t = uint64_t;

#undef min
#undef max

namespace odb {

static constexpr oid_t INVALID_ID		= 0ull;
static constexpr oid_t COUNTER_ID		= 1ull;
static constexpr oid_t NEW_NODE_ID	= 2ull;
static constexpr oid_t DB_GUID_ID		= 3ull;
static constexpr oid_t STATIC_ID_START	= 100ull;
static constexpr oid_t FIRST_ID					= 5000ull;

enum class NodeEvent {
	CHANGED,
	DELETED
};

enum {
	F_REPLICATED	= (1 << 0),
	F_MAPPED			= (1 << 1),
	F_CHANGED			= (1 << 2),
	F_REMOVED			= (1 << 3)
};

enum class NodeError {
	E_NO_ERROR,
	E_FETCH_FAILED_NOT_EXISTS,
	E_FETCH_FAILED,
	E_STORE_FAILED,
	E_REMOVE_FAILED,
	E_ARCHIVE_CORRUPT,
	E_CONNECTION_LOSS,
	E_UNKNOWN_ERROR
};

} // namespace odb

#endif // __NPDB_CONSTANTS_H__
