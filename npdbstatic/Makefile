# libnpbd_static
TARGET = ../build/lib/libnpdb_static.a

CXX = g++
CXXFLAGS = -std=c++20 -O2 -Wall -fPIC -I../npdbclient/include

OBJDIR = obj/
dummy := $(shell mkdir -p $(OBJDIR))

SRCS = $(shell find src/ -name '*.cpp') 
OBJS = $(addprefix obj/,$(addsuffix .o, $(basename $(notdir $(SRCS)))))

$(TARGET): $(OBJS)
	ar rcs $(TARGET) $^

$(OBJDIR)%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

install:
	cp $(TARGET) /usr/bin

clean:
	rm -rf ./obj
	rm -f $(TARGET)

.PHONY: install clean