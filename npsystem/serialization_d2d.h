// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

#include <d2d1helper.h>

namespace boost {
	namespace serialization {
		/////////////////////////////////////////////////////////////
		template<class Archive>
		inline void serialize(Archive & ar, D2D1::Matrix3x2F &t, const unsigned int file_version) {
			ar & t._11;
			ar & t._12;
			ar & t._21;
			ar & t._22;
			ar & t._31;
			ar & t._32;
		}
	} // namespace serialization
} // namespace boost