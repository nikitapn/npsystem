# npc
TARGET = ../build/lib/libnpc.so
CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2 -fPIC
DEFS = -DNDEBUG
INCLUDE = -I./include/npc -I../nprpc/include

$(TARGET): obj/db.o obj/server.o
	$(CXX) $^ -shared -o $@

obj/db.o: src/db.cpp include/npc/db.hpp include/npc/db_m.hpp | obj
	$(CXX) $(CXXFLAGS) $(DEFS) $(INCLUDE) -c src/db.cpp -o $@
	
obj/server.o: src/server.cpp include/npc/server.hpp include/npc/server_m.hpp | obj
	$(CXX) $(CXXFLAGS) $(DEFS) $(INCLUDE) -c src/server.cpp -o $@

obj:
	mkdir -p obj

clean:
	rm -f $(TARGET)
	rm -r obj/

.PHONY: all clean
