CXX=clang++

shared:
	$(CXX) -ObjC++ -g -dynamiclib -fPIC  -framework OpenGL -framework Cocoa -Iinclude -o libxtgl.dylib src/XTGL.cpp
