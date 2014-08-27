#ifdef __cplusplus
extern "C" {
#endif

void *getEvent(void* args);
void *makeGLContext(void* args);
void *glSwapBuffers(void* args);
void *glMakeContextCurrent(void* args);
#if defined __APPLE__
void *makeGLCoreContext(void* args);
#endif

#ifdef __cplusplus
}
#endif
