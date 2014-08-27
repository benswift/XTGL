UNAME := $(shell uname)

LIBNAME := libxtgl

# Liunx
ifeq ($(UNAME), Linux)
CXX = g++
CXXFLAGS += -g -shared -fPIC -lGL -Iinclude
LIBEXT = so
endif
# OSX
ifeq ($(UNAME), Darwin) 
CXX = clang++
CXXFLAGS +=  -g -ObjC++ -dynamiclib -fPIC  -framework OpenGL -framework Cocoa -Iinclude
LIBEXT = dylib
endif

TARGET := $(LIBNAME).$(LIBEXT)

$(TARGET): $(LIBNAME).o
	$(CXX) $(CXXFLAGS) -o $@ $^

$(LIBNAME).o: src/XTGL.cpp
	$(CXX) $(CXXFLAGS)  -o $@ $<

shared: $(TARGET)

clean:
	rm -f *.o *.so *.dll *.exe
