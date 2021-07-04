// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#ifndef __DBUTILS_H__
#define __DBUTILS_H__

#include <string>
#include "constants.h"
#include <leveldb/slice.h>

leveldb::Slice create_slice(const oid_t* id_ptr) {
	return leveldb::Slice((const char*)id_ptr, sizeof(oid_t));
}

oid_t read_oid(const std::string& slice) {
	assert(slice.length() == sizeof(oid_t));
	return *reinterpret_cast<const oid_t*>(slice.c_str());
}

oid_t read_oid(const leveldb::Slice& slice) {
	assert(slice.size() == sizeof(oid_t));
	return *reinterpret_cast<const oid_t*>(slice.data());
}

#endif // __DBUTILS_H__
