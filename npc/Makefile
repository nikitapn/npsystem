# npc
TARGET = ../build/lib/libnpc.a
CXX = g++
CXXFLAGS = -std=c++20 -Wall -O2 -fPIC
DEFS = -DNDEBUG
INCLUDE = -I./include/npc -I../nprpc/include
OBJDIR = obj
dummy := $(shell mkdir -p $(OBJDIR))

$(TARGET): $(OBJDIR)/db.o $(OBJDIR)/server.o
	ar rcs $(TARGET) $^

$(OBJDIR)/db.o: src/db.cpp include/npc/db.hpp include/npc/db_m.hpp 
	$(CXX) $(CXXFLAGS) $(DEFS) $(INCLUDE) -c src/db.cpp -o $@
	
$(OBJDIR)/server.o: src/db.cpp include/npc/server.hpp include/npc/server_m.hpp 
	$(CXX) $(CXXFLAGS) $(DEFS) $(INCLUDE) -c src/db.cpp -o $@

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	rm -f $(TARGET)
	rm -r obj/

.PHONY: all clean
