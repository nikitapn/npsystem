# libavr_info
TARGET = ../build/lib/libavr_info.so
CXX = g++
CXXVER = c++20
CXXFLAGS := -std=$(CXXVER) -O2 -Wall -ftemplate-backtrace-limit=0 -fPIC

SRCS = $(shell find src/ -name '*.cpp') 
OBJS = $(addprefix obj/,$(addsuffix .o, $(basename $(notdir $(SRCS)))))

DEFS = -DNDEBUG

INCS = \
	-I../avr_info/include \
	-I../nplib/include

LIB_DIR = -L../build/lib

$(TARGET): $(OBJS) 
	$(CXX) $(OBJS) $(LIB_DIR) -shared -o $@

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