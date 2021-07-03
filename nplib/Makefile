# libnplib.so

TARGET := ../build/lib/libnplib.so
CXX := g++
CXXVER := c++20
CXXFLAGS = -std=$(CXXVER) -fPIC -O2 -Wall -ftemplate-backtrace-limit=0 $(INCLUDE_DIRS)

SRCS = $(shell find src/ -name '*.cpp') 
OBJS = $(addprefix obj/,$(addsuffix .o, $(basename $(notdir $(SRCS)))))

INCLUDE_DIRS := -I./include

$(TARGET): $(OBJS) 
	$(CXX) $(OBJS) -shared -o $(TARGET)

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