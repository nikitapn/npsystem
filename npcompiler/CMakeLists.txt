add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/lex.yy.cc 
  COMMAND bash -c ${CMAKE_CURRENT_SOURCE_DIR}/flex.sh
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/lang.l
)

add_custom_command(
  OUTPUT 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/stack.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/lang.tab.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/location.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/position.hh
    ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/lang.tab.cc
  COMMAND bash -c "bison -r all -t -Wempty-rule -d -v -b ${CMAKE_CURRENT_SOURCE_DIR}/src/generated ${CMAKE_CURRENT_SOURCE_DIR}/src/lang.y"
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/lang.y
)

add_library(npcompiler SHARED 
  src/generated/stack.hh
  src/generated/lang.tab.hh
  src/generated/location.hh
  src/generated/lang.tab.cc
  src/generated/position.hh
  src/generated/lex.yy.cc
  src/ast.hpp
  src/ast.cpp
  src/ast_utils.hpp
  src/typer.hpp
  src/typer.cpp
  src/utils.hpp
  src/utils.cpp
  src/builder_llvm.hpp
  src/builder_llvm.cpp
  src/number.hpp
  src/npcompiler.cpp
)

add_definitions(
  -DNPCOMPILER_EXPORTS
  -DWIN32
  -D_WINDOWS
  -D_SILENCE_CXX20_CISO646_REMOVED_WARNING
  -D_CRT_SECURE_NO_DEPRECATE
  -D_CRT_SECURE_NO_WARNINGS
  -D_CRT_NONSTDC_NO_DEPRECATE
  -D_CRT_NONSTDC_NO_WARNINGS
  -D_SCL_SECURE_NO_DEPRECATE
  -D_SCL_SECURE_NO_WARNINGS
  -D__STDC_VERSION__=_DEBUG
  -D_LIB
)

target_include_directories(npcompiler PRIVATE
  ./include
  ../npdbclient/include
  ../npsys/include
  ../nplib/include
  C:\\opt\\llvm\\llvm\\include
  C:\\opt\\llvm\\build\\include
)


target_link_directories(npcompiler PRIVATE
  C:\\opt\\llvm\\build\\Debug\\lib
)

target_link_libraries(npcompiler PRIVATE
  ntdll
  LLVMCore
  LLVMMC
  LLVMMCJIT
  LLVMExecutionEngine
  LLVMObject
  LLVMOrcJIT
  LLVMSupport
  LLVMCodeGenTypes
  LLVMX86CodeGen
  LLVMX86AsmParser
  LLVMX86Desc
  LLVMX86Disassembler
  LLVMX86Info
  LLVMRuntimeDyld
  LLVMPasses
  LLVMCoroutines
  LLVMipo
  LLVMFrontendOpenMP
  LLVMIRReader
  LLVMAsmParser
  LLVMLinker
  LLVMObjCARCOpts
  LLVMVectorize
  LLVMJITLink
  LLVMOrcTargetProcess
  LLVMOrcShared
  LLVMAsmPrinter
  LLVMDebugInfoMSF
  LLVMInstrumentation
  LLVMGlobalISel
  LLVMSelectionDAG
  LLVMCodeGen
  LLVMTarget
  LLVMTargetParser
  LLVMBitWriter
  LLVMScalarOpts
  LLVMAggressiveInstCombine
  LLVMInstCombine
  LLVMTransformUtils
  LLVMAnalysis
  LLVMProfileData
  LLVMDebugInfoDWARF
  LLVMBitReader
  LLVMTextAPI
  LLVMCFGuard
  LLVMRemarks
  LLVMBitstreamReader
  LLVMMCParser
  LLVMMCDisassembler
  LLVMBinaryFormat
  LLVMDebugInfoCodeView
  LLVMAVRAsmParser
  LLVMAVRCodeGen
  LLVMAVRDesc
  LLVMAVRDisassembler
  LLVMAVRInfo
  LLVMARMAsmParser
  LLVMARMCodeGen
  LLVMARMDesc
  LLVMARMDisassembler
  LLVMARMInfo
  LLVMARMUtils
  LLVMDemangle
)

