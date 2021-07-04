// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#pragma once

namespace boost {
	namespace serialization {
		/////////////////////////////////////////////////////////////
		template<class Archive>
		inline void serialize(Archive & ar, CRect &rc, const unsigned int) {
			ar & rc.left;
			ar & rc.top;
			ar & rc.right;
			ar & rc.bottom;
		}
	} // namespace serialization
} // namespace boost