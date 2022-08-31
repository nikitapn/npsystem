// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include "stdafx.h"

#include <nprpc/serialization/serialization.h>
#include <nprpc/serialization/oarchive.h>

struct ClassC {
	std::string innocent_string = "innocent_string";

	template<typename Archive>
	void serialize(Archive& ar) {
		ar& NVP(innocent_string);
	}
};

struct ClassB {
	std::string other_str = "help";
	ClassC class_c;

	template<typename Archive>
	void serialize(Archive& ar) {
		ar& NVP(other_str);
		ar& NVP(class_c);
	}
};

struct ClassA {
	int a = 10;
	int b = 20;
	float c = 3.14f;
	std::string str = "test";
	ClassB d;
	
	template<typename Archive>
	void serialize(Archive& ar) {
		ar & NVP(a);
		ar & NVP(b);
		ar & NVP(c);
		ar & NVP(str);
		ar & NVP(d);
	}
};



int test_stuff() {
	ClassA a;

	std::stringstream ss;
	{
		nprpc::serialization::json_oarchive oa(ss);
		oa & a;
	}

	std::cerr << ss.str();

	return 0;
}