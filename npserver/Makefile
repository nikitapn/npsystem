# npserver
TARGET = ../build/bin/npserver
CXX = g++
CXXVER = c++20
CXXFLAGS := -std=$(CXXVER) -O2 -Wall -ftemplate-backtrace-limit=0

SRCS = $(shell find src/ -name '*.cpp') 
OBJS = $(addprefix obj/,$(addsuffix .o, $(basename $(notdir $(SRCS)))))
DEFS = -DBOOST_LOG_DYN_LINK -DNDEBUG -DNPSYSTEM_FIRMWARE_API_CPP

INC_DIR = \
	-I../../mylib/include \
	-I../npc/include \
	-I../avr_info/include \
	-I../sim/include \
	-I../nplib/include \
	-I../npdbclient/include \
	-I../npsys/include \
	-I../avr_firmware/include \
	-I../nprpc/include

LIB_DIR = -L../build/lib

LIBS = \
	-lpthread \
	-lleveldb \
	-lnpdb_static \
	-lnpdbclient \
	-lsim \
	-lnplib \
	-lnpsys \
	-lavr_info \
	-lnprpc

SLIBS = \
	/usr/lib/libboost_system.a \
	/usr/lib/libboost_thread.a \
	/usr/lib/libboost_serialization.a \
	/usr/lib/libboost_log.a \
	../build/lib/nprpcst.o \
	../npc/obj/server.o \
	../npc/obj/db.o 

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
