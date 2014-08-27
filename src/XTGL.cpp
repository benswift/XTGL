#include "XTGL.h"

#ifdef _MSC_VER
#include "XTGL-windows.cpp"
#elif __linux__
#include "XTGL-linux.cpp"
#elif __APPLE__
#include "XTGL-osx.cpp"
#endif
