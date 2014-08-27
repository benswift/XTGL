CXX=clang++

shared:
	$(CXX) -ObjC++ -g -dynamiclib -fPIC src/XTGL.cpp -Iinclude -o libxtgl.dylib
