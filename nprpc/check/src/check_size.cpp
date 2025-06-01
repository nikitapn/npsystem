// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "utils.hpp"
#include <nprpc_stub/nprpc_base.hpp>

static_assert(sizeof(nprpc::impl::flat::CallHeader) == size_of_call_header);
static_assert(alignof(nprpc::impl::flat::CallHeader) == align_of_call_header);

static_assert(sizeof(nprpc::detail::flat::ObjectId) == size_of_object);
static_assert(alignof(nprpc::detail::flat::ObjectId) == align_of_object);
