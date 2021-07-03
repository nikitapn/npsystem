# libsim
TARGET := ../build/lib/libsim.so
CXX := g++
CXXVER := c++20
CXXFLAGS := -std=$(CXXVER) -O2 -Wall -ftemplate-backtrace-limit=0 -fPIC

SRCS = $(shell find src/ -name '*.cpp') 
OBJS = $(addprefix obj/,$(addsuffix .o, $(basename $(notdir $(SRCS)))))

DEFS = -DBOOST_LOG_DYN_LINK -DNDEBUG -DNPSYSTEM_FIRMWARE_API_CPP

INCS = \
	-I../sim/include \
	-I../nplib/include \
	-I../npdb/include \
	-I../npdbclient/include \
	-I../avr_firmware/include

LIB_DIR := -L../build/lib

LIBS := -lpthread \
	-lnplib \
	-lnpdb_static \
	-lnpdbclient

SLIBS := /usr/lib/libboost_system.a

$(TARGET): $(OBJS) 
	$(CXX) $(OBJS) $(LIB_DIR) $(LIBS) $(SLIBS) -shared -o $(TARGET)

obj/%.o: src/%.cpp | obj
	 $(CXX) $(CXXFLAGS) $(INCS) $(DEFS) -c $^ -o $@
	 
obj: 
	mkdir -p obj

install:
	cp $(TARGET) /usr/local/bin
	
clean:
	rm -rf ./obj
	rm -f $(TARGET)

.PHONY: install clean