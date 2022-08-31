// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"
#include <npdb/db.h>
#include <npdb/memento_manager.h>
#include <npdb/debug.h>
#include <npsys/other/uploadable.h>

class A : public IUploadable {
	friend int test_memento();
public:
	virtual std::unique_ptr<odb::IMemento> CreateMemento_Uploadable() noexcept {
		return odb::MakeMemento(*this, x);
	}
private:
	int x;
	int y;
};


class Node
	: public::odb::common_interface<odb::no_interface> {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {

	}
public:
	A a;
};

using node_n = odb::shared_node<Node>;
using node_l = odb::node_list<node_n>;

int test_memento() {
	std::cout << "test_memento" << std::endl;
	//db_connect();

	node_n n1 = odb::create_node<node_n>();
	node_n n2 = odb::create_node<node_n>();

	node_l l;
	l.init();

	odb::NodeHolder holder;
	odb::NodeMementoManager<IUploadable> mm1(holder);

	A a;
	
	a.x = 0;
	a.y = 0;

	l.add_node(n1);

	mm1.AddMemento(n1, &a);
	mm1.AddMemento(l);

	l.add_node(n2);

	a.x = 666;
	a.y = 777;

	l.store();
	std::cout << l << std::endl;

//	holder.StoreAll();
	mm1.RestoreAll();

	std::cout << l << std::endl;

	std::cout << "x: " << a.x << std::endl;
	std::cout << "y: " << a.y << std::endl;
	return 0;
}