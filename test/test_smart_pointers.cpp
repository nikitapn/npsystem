#include "stdafx.h"
#include <npdb/db.h>

using node_1_n = odb::shared_node<class Node1>;
using node_2_n = odb::shared_node<class Node2>;

class Node1 : public odb::common_interface<odb::no_interface> {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {}
public:
	int id = 1;
	node_2_n ref;
};

class Node2 : public odb::common_interface<odb::no_interface> {
	friend boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version) {}
public:
	int id = 2;
	odb::weak_node<node_1_n> ref;
};


int test_smart_pointers() {
	//db_connect();
	{
		node_1_n n1;
		n1.create();
		n1->id = 5;
		node_2_n n2;
		n2.create();
		n1->ref = n2;
		n2->ref = n1;
		auto n = n2->ref.fetch();
		std::cout << n->id << std::endl;
	}
#ifdef DEBUG
	odb::check_memory_leak();
#endif
	return 0;
}