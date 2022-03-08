# npwebserver
TARGET = ../build/bin/npwebserver
CXX = g++
CXXVER = c++20
CXXFLAGS := -std=$(CXXVER) -O2 -Wall -ftemplate-backtrace-limit=0

SRCS = $(shell find src/ -name '*.cpp') 
OBJS = $(addprefix obj/,$(addsuffix .o, $(basename $(notdir $(SRCS)))))
DEFS = -DNDEBUG -DNPSYSTEM_FIRMWARE_API_CPP

INC_DIR = \
	-I../avr_info/include \
	-I../npc/include \
	-I../nplib/include \
	-I../npdbclient/include \
	-I../npsys/include \
	-I../nprpc/include

LIB_DIR = -L../build/lib

LIBS = \
	-lpthread \
	-lnpdb_static \
	-lnpdbclient \
	-lnplib \
	-lnpsys \
	-lnprpc \
	-lnpc

SLIBS = \
	/usr/lib/libboost_system.a \
	/usr/lib/libboost_thread.a \
	/usr/lib/libboost_serialization.a \
	../build/lib/nprpcst.a

$(TARGET): $(OBJS) 
	$(CXX) $(OBJS) $(LIB_DIR) $(LIBS) $(SLIBS) -o $(TARGET)

obj/%.o: src/%.cpp | obj
	 $(CXX) $(CXXFLAGS) $(INC_DIR) $(DEFS) -c $^ -o $@

obj: 
	mkdir -p obj
	
install:
	cp $(TARGET) /usr/local/bin
	
clean:
	rm -rf ./obj
	rm -f $(TARGET)

.PHONY: install clean
