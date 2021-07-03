#pragma once

#include <atlstr.h>

namespace boost {
	namespace serialization {
		/////////////////////////////////////////////////////////////
		// implement serialization for unique_ptr< T >
		// note: this must be added to the boost namespace in order to
		// be called by the library
		template<class Archive>
		inline void save(Archive & ar, const CStringW &t, const unsigned int /*file_version*/) 
		{
			ar << narrow(t, t.GetLength());
		}

		template<class Archive>
		inline void load(Archive & ar, CStringW &t, const unsigned int /*file_version*/) 
		{
			std::string u8str;
			ar >> BOOST_SERIALIZATION_NVP(u8str);
			t = wide(u8str).c_str();
		}

		// split non-intrusive serialization function member into separate
		// non intrusive save/load member functions
		template<class Archive>
		inline void serialize(Archive & ar, CStringW &t, const unsigned int file_version) 
		{
			boost::serialization::split_free(ar, t, file_version);
		}

	} // namespace serialization
} // namespace boost