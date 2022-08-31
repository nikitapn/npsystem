// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <iostream>
#include <utility>
#include <assert.h>
#include <random>
#include <nplib/utils/helper.h>
#include "btree.h"
#include "bplustree.h"
#include "npdb.h"
#include "lru_cache.h"
#include <Windows.h>

template<typename Node>
void print_node(npdb::mapped_ptr<Node> node, npdb::Database& db) {
	auto it = npdb::_bfs_iterator{node};
	size_t level = 0;
	do {
		auto node = it.value();
		if (node.second != level) {
			level = node.second;
			std::cerr << '\n';
		}
		std::cerr << *node.first << ' ';
	} while (it.next());
	std::cerr << '\n';
}

template<typename BTree>
bool insert(BTree& root, uint64_t key, npdb::Database& db) {
	static int n = 1;
	//std::cout << "->" << n++ <<" insert: " << key << '\n';
	npdb::TreeMeta val;
	//auto s = "table_with_id:" + std::to_string(key);
	//std::memcpy(val.tree_name, s.c_str(), s.length()+1);
	val.ai.payload_size = sizeof(npdb::TreeMeta);
	if (root.insert(typename BTree::KeyValue{ key, val }) == false) {
		std::cerr << "Warning: key {" << key << "} exists...\n";
		return false;
	} else {
		//print_node(root.root(), db);
	}

	return true;
}

std::vector<uint32_t> load_data(const int max, const char* filename) {
	std::ifstream file(filename);
	std::vector<uint32_t> data;
	data.reserve(max);
	if (file) {
		std::copy(std::istream_iterator<uint32_t>(file), std::istream_iterator<uint32_t>(), std::back_inserter(data));
	} else {
		data.resize(max);
		for (int i = 0; i < max; ++i) data[i] = i;

		std::random_device rd;
    std::mt19937 g(rd());
		std::shuffle(data.begin(), data.end(), g);

		std::ofstream file(filename);
		std::copy(data.begin(), data.end(), std::ostream_iterator<uint32_t>(file, "\n"));
	}
	return data;
}

#include <conio.h>

int test_tsdb() {
	auto wait_any_button = []() { std::cout << "press any button to return to the previous screen..." << std::endl; _getch(); };
	 
	try {
		npdb::Database db("d:/temp/my.db");
		
		auto show_main_tree = [&]() {
			std::cout << "Main tree:\n";
			auto the_tree = db.main_tree();
			print_node(the_tree.root(), db);
		};


		int state = -1;
		bool quit = false;

		while (!quit) {
			std::system("cls");
			
			switch (state) {
			case 1: {
				std::cout << "Enter new table id: " << std::endl;
				
				uint64_t tree_id;
				std::cin >> tree_id;

				if (!db.create_tree<npdb::BTree<uint64_t, uint64_t, 25>>(tree_id)) {
					wait_any_button();
					break;
				}

				show_main_tree();
				wait_any_button();

				state = -1;

				break;
			}
			case 2: {
				auto i = db.select_tree<npdb::TreeMeta>(100);
				std::cerr << *i << '\n';
				int f = 0;
				
				break;
			}
			case 100:
				quit = true;
				break;
			default:
				std::cout <<
					"1. Create table\n"
					"2. Select table\n"
					"q. quit\n"
					<< std::endl;

				show_main_tree();

			select_action:
				auto c = _getch();
				if (c == '1') { state = 1; break; }
				if (c == '2') { state = 2; break; }
				if (c == 'q') { state = 100; break; }
				goto select_action;
			}
		}


		/*
		constexpr size_t size = 128;
		auto data = load_data(size, "d:/temp/data.bin");

		auto the_tree = db.main_tree();

		//std::random_device rd;
    //std::mt19937 g(rd());
		
		for (size_t i = 1; i <= size; ++i) {
			auto key = data[i - 1];
			auto value = i;
			//insert_print(the_tree, data[i - 1], db);
			insert_print(the_tree, key, value, db);
			for (size_t j = 0; j < i; ++j) {
				if (!the_tree.find(key)) {
					std::cerr << "Error: " << key << " was not found...\n";
					assert(false);
				}
			}
		}
		
		print_node(the_tree.root(), db);

		if (npdb::verify(the_tree.root(), {}, {}) == false) {
			std::cerr << "Error: verification failed...\n";
			assert(false);
		}
		*/
	} catch (std::exception& e) {
		std::cerr << e.what() << '\n';
		return -1;
	}
	

	return 0;
}