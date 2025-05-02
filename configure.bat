@echo off

rem call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

cmake -S . -B build\win ^
  -G "Visual Studio 17 2022" ^
  -DCMAKE_BUILD_TYPE=Debug ^
  -DOPT_BOOST_INCLUDE_DIR="C:\opt\boost_1_88_0" ^
  -DOPT_BOOST_LIB_DIR="C:\opt\boost_1_88_0\stage_x64\lib" ^
  -DOPT_LEVELDB_INCLUDE_DIR="C:\opt\leveldb\include" ^
  -DOPT_LEVELDB_LIB_DIR="C:\opt\leveldb\lib" ^
  -DOPT_WTL_INCLUDE_DIR="C:\opt\wtl\Include" ^
  -DOPT_SCINTILLA_DIR="C:\opt\scintilla" ^
  -DOPT_LLVM_DIR="C:\opt\llvm"

