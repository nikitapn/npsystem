// Copyright (c) 2021 nikitapnn1@gmail.com
// This file is a part of npsystem (Distributed Control System) and covered by LICENSING file in the topmost directory

#include <iostream>
#include <avr_info/avr_info.h>
#include <cstdlib>
#include <fstream>
#include <boost/serialization/array.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/program_options.hpp>
#include <queue>

namespace m8 { extern void init(int); }
namespace m8v { extern void init(int); }
namespace m16 { extern void init(int); }
namespace m16v { extern void init(int); }

extern avrinfo::FirmwareInfo* info_m8, * info_m8v, * info_m16, * info_m16v;
extern avrinfo::PCLinkInfo* info_pclink;
extern avrinfo::VirtualPCLinkInfo* info_pclink_virtual;

int main(int argc, char** argv) {
  namespace po = boost::program_options;
  namespace fs = std::filesystem;
  try {
    fs::path out_dir;
    po::options_description desc("Allowed options");

    desc.add_options()
      ("help", "produce help message")
      ("out-dir", po::value<fs::path>(&out_dir), "Directory for generated files")
      ;

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << "\n";
      return 0;
    }

    if (!vm.count("out-dir")) {
      std::cerr << "no out-dir specified.\n";
      return 1;
    }

    std::priority_queue<std::uint32_t> versions;
    const auto ctrl_dir = out_dir / "avr" / "ctl_ver";

    fs::create_directories(ctrl_dir);
   
    for (const auto& entry : fs::directory_iterator(ctrl_dir)) {
      if (!entry.is_regular_file()) continue;
      std::stringstream ss;
      ss << entry.path().filename();
      std::uint32_t version;
      ss >> version;
      versions.push(version);
    }

    std::uint32_t version = 0;
    if (!versions.empty()) {
      version = versions.top() + 1;
    }

    std::cout << "Making info for Firmware version: " << version << std::endl;

    m8::init(version);
    m8v::init(version);
    m16::init(version);
    m16v::init(version);

    auto write_binary = [](fs::path path, const auto& data) {
      std::ofstream ofs(path, std::ios_base::binary);
      boost::archive::binary_oarchive oa(ofs, boost::archive::no_header | boost::archive::no_tracking);
      oa << data;
      };

    write_binary(ctrl_dir / std::to_string(version),
      avrinfo::avr_controller_info_lst_t{ *info_m8 , *info_m8v, *info_m16, *info_m16v });
    write_binary(out_dir / "avr" / "pclink", *info_pclink);
    write_binary(out_dir / "avr" / "pclink_virtual", *info_pclink_virtual);

    return 0;
  } catch (po::unknown_option& e) {
    std::cerr << "Unknown option: " << e.what() << '\n';
  } catch (std::exception& ex) {
    std::cerr << ex.what() << '\n';
  }

  return 1;
}
