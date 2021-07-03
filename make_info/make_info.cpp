#include <iostream>
#include <avr_info/avr_info.h>
#include <cstdlib>
#include <fstream>
#include <boost/serialization/array.hpp>
#include <boost/archive/binary_oarchive.hpp>


namespace m8 { extern void init(int); }
namespace m8v { extern void init(int); }
namespace m16 { extern void init(int); }
namespace m16v { extern void init(int); }

extern avrinfo::FirmwareInfo				*info_m8, *info_m8v, *info_m16, *info_m16v;
extern avrinfo::PCLinkInfo					*info_pclink;
extern avrinfo::VirtualPCLinkInfo		*info_pclink_virtual;

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cerr << "Firmware version is not specified...\n";
		return -1;
	}

	auto version_str = std::string(argv[1]);
	auto version_n = std::atoi(argv[1]);

	std::cout << "Making info for Firmware version: " << version_n << std::endl;

	m8::init(version_n);
	m8v::init(version_n);
	m16::init(version_n);
	m16v::init(version_n);

	try {
		{
			std::ofstream ofs("../data/avr/ctl_ver/" + version_str, std::ios_base::binary);
			boost::archive::binary_oarchive oa(ofs, boost::archive::no_header | boost::archive::no_tracking);
			avrinfo::avr_controller_info_lst_t lst;
			lst[0] = *info_m8;	lst[1] = *info_m8v;
			lst[2] = *info_m16; lst[3] = *info_m16v;
			oa << lst;
		}

		{
			std::ofstream ofs("../data/avr/pclink", std::ios_base::binary);
			boost::archive::binary_oarchive oa(ofs, boost::archive::no_header | boost::archive::no_tracking);
			oa << *info_pclink;
		}

		{
			std::ofstream ofs("../data/avr/pclink_virtual", std::ios_base::binary);
			boost::archive::binary_oarchive oa(ofs, boost::archive::no_header | boost::archive::no_tracking);
			oa << *info_pclink_virtual;
		}
	} catch (std::exception& ex) {
		std::cerr << ex.what() << '\n';
		return -1;
	}

	std::cout << "done." << std::endl;

	return 0;
}
