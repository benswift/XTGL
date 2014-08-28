UNAME := $(shell uname)

LIBNAME := libxtgl

# Liunx
ifeq ($(UNAME), Linux)
CXX = g++
CXXFLAGS += -g -fPIC -Iinclude
LDFLAGS += -lGL -lX11
LIBEXT = so
endif
# OSX
ifeq ($(UNAME), Darwin) 
CXX = clang++
CXXFLAGS +=  -g -ObjC++ -dynamiclib -fPIC  -framework OpenGL -framework Cocoa -Iinclude
LIBEXT = dylib
endif

TARGET := $(LIBNAME).$(LIBEXT)

shared: $(TARGET)

$(LIBNAME).dylib: src/XTGL.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

$(LIBNAME).o: src/XTGL.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(LIBNAME).so: $(LIBNAME).o
	$(CXX) -shared -o $@ $< $(LDFLAGS)

clean:
	rm -f *.o *.so *.dylib *.dll *.exe
