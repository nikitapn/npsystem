

set(SOURCES
  include/npdatabase/bfsiterator.hpp
  include/npdatabase/btree.hpp
  include/npdatabase/dbtypes.hpp
  include/npdatabase/page.hpp

  include/npdatabase/impl/buffer_pool_manager.hpp
  include/npdatabase/impl/disk_manager.hpp

  src/buffer_pool_manager.cpp
  src/disk_manager.cpp
)

add_library(npdatabase SHARED ${SOURCES})

target_include_directories(npdatabase PRIVATE include)

