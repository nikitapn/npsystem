
add_library(nplib
  src/hexparser.cpp
  src/log.cpp  
  include/nplib/import_export.h
  include/nplib/utils/helper.h
  include/nplib/utils/thread_pool.hpp
  include/nplib/utils/binary_search.hpp
  include/nplib/utils/thread_safe_set.h
  include/nplib/utils/value_parser.h
  include/nplib/utils/ext.h
  include/nplib/utils/task_queue.h
  include/nplib/utils/omembuf.hpp
  include/nplib/utils/win_service.h
  include/nplib/utils/colored_cout.h
  include/nplib/utils/variant.h
  include/nplib/utils/log.h
  include/nplib/utils/utf8.h
  include/nplib/utils/format.h
  include/nplib/utils/config.h
  include/nplib/utils/types.h
  include/nplib/utils/spinlock.h
  include/nplib/utils/singleton.hpp
  include/nplib/utils/hexparser.h
  include/nplib/windows/console.hpp
)

target_include_directories(nplib PRIVATE ./include)
target_compile_options(nplib PRIVATE -fPIC)
