// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

struct HASHMD5 
{
	char md5[16];
	char& operator[](int i) {
		return md5[i];
	}
	bool operator==(const HASHMD5& hash) const {
		for (int i = 0; i < 16; ++i) {
			if (md5[i] != hash.md5[i])
				return false;
		}
		return true;
	}
	bool operator!=(const HASHMD5& hash) const {
		for (int i = 0; i < 16; ++i) {
			if (md5[i] != hash.md5[i])
				return true;
		}
		return false;
	}
	void operator=(const HASHMD5& hash) {
		for (int i = 0; i < 16; ++i)
			md5[i] = hash.md5[i];
	}
	template<class Archive>
	void serialize(Archive & ar, const unsigned int file_version) {
		ar & md5;
	}
};

int GetFileMD5(std::string_view file_name, HASHMD5& md5);
int GetStringMD5(std::string_view str, HASHMD5& md5);
