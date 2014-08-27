UNAME := $(shell uname)

LIBNAME := libxtgl

# Liunx
ifeq ($(UNAME), Linux)
CXX = g++
CXXFLAGS += -g -shared -fPIC -lGL -lX11 -Iinclude
LIBEXT = so
endif
# OSX
ifeq ($(UNAME), Darwin) 
CXX = clang++
CXXFLAGS +=  -g -ObjC++ -dynamiclib -fPIC  -framework OpenGL -framework Cocoa -Iinclude
LIBEXT = dylib
endif

TARGET := $(LIBNAME).$(LIBEXT)

$(TARGET): src/XTGL.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f *.o *.so *.dylib *.dll *.exe
