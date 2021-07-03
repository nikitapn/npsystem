# libnpsys
TARGET = ../build/lib/libnpsys.a
CXX = g++
CXXVER = c++20
CXXFLAGS = -std=$(CXXVER) -O2 -Wall -ftemplate-backtrace-limit=0 -fPIC

SRCS = $(shell find src/ -name '*.cpp') 
OBJS = $(addprefix obj/,$(addsuffix .o, $(basename $(notdir $(SRCS)))))
DEFS = -DNDEBUG

INCLUDE_DIRS = \
	-I../npsys/include \
	-I../npdbclient/include \
	-I../npc/include \
	-I../nplib/include \
	-I../nprpc/include

$(TARGET): $(OBJS) 
	ar rcs $(TARGET) $^

obj/%.o: src/%.cpp | obj
	 $(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(DEFS) -c $^ -o $@

obj: 
	mkdir -p obj

install:
	cp $(TARGET) /usr/local/bin
	
clean:
	rm -rf ./obj
	rm -f $(TARGET)

.PHONY: install clean
