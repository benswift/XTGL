static pointer getEvent(scheme* _sc, pointer args);
static pointer makeGLContext(scheme* _sc, pointer args);
static pointer glSwapBuffers(scheme* _sc, pointer args);
static pointer glMakeContextCurrent(scheme* _sc, pointer args);
#if defined __APPLE__
static pointer makeGLCoreContext(scheme* _sc, pointer args);
#endif
