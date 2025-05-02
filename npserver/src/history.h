// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by
// LICENSING file in the topmost directory

#pragma once

#include "protocol_base.h"
#include "listener.h"

class HistoryImpl;
class History
{
  std::unique_ptr<HistoryImpl> impl_;

 public:
  protocol::listener* get_listener() noexcept;
  void                add_parameter(oid_t param_id);
  void                remove_parameter(oid_t param_id);

  History();
  ~History();
};

extern std::unique_ptr<History> g_hist;
