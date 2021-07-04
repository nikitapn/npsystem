// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

struct REQUEST
{
	int ID_MENU;
	void* pItem;
	union {
		void* param;
		oid_t db_id;
	};
	union
	{
		void* result_ptr;
		int64_t result;
		class CTreeItemAbstract* item;
	};
	inline REQUEST(int ID_REQUEST, void* pItem = NULL, void* param = NULL) {
		this->ID_MENU = ID_REQUEST;
		this->pItem = pItem;
		this->param = param;
		this->result = 0;
	}
};