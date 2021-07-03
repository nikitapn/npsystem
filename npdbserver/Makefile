# npdbserver
TARGET = ../build/bin/npdbserver
CXX = g++
CXXVER = c++20
CXXFLAGS = -std=$(CXXVER) -O2 -Wall -ftemplate-backtrace-limit=0

SRCS = $(shell find src/ -name '*.cpp') 
OBJS = $(addprefix obj/,$(addsuffix .o, $(basename $(notdir $(SRCS)))))

DEFINES = -DBOOST_LOG_DYN_LINK -DNDEBUG

INCLUDE_DIRS = \
	-I../nplib/include \
	-I../npdbclient/include \
	-I../npc/include -I../nprpc/include

LIB_DIR = -L../build/lib

LIBS = \
	-lpthread \
	-lleveldb \
	-lnpc \
	-lnpdb_static \
	-lboost_log \
	-lnprpc

SLIBS = \
	/usr/lib/libboost_system.a \
	/usr/lib/libboost_serialization.a \
	../build/lib/nprpcst.o

$(TARGET): $(OBJS) 
	$(CXX) $(OBJS) $(LIB_DIR) $(LIBS) $(SLIBS) -o $(TARGET)

obj/%.o: src/%.cpp | obj
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) $(DEFINES) -c $^ -o $@

obj: 
	mkdir -p obj
	 
install:
	cp $(TARGET) /usr/local/bin
	
clean:
	rm -rf ./obj
	rm -f $(TARGET)

.PHONY: install clean
