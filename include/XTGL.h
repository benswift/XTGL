#ifdef __cplusplus
extern "C" {
#endif

  typedef void XTGLcontext;
  typedef void XTGLevent;
  /* typedef XTGLwindow* void*; */

  XTGLcontext *xtglCreateContext(int x, int y, int width, int height, char *displayID, int fullscreen);
#if defined __APPLE__
  XTGLcontext *xtglCreateCoreContext(int x, int y, int width, int height, char *displayID, int fullscreen);
#endif
  void xtglSwapBuffers(XTGLcontext *ctx);
  void xtglMakeContextCurrent(XTGLcontext *ctx);

  /* Events: */
  /* 1: key down   */
  /* 2: mouse down */
  /* 3: mouse up   */
  /* 4: mouse drag */
  void xtglGetEvent(XTGLcontext *ctx, XTGLevent* event);
  
#ifdef __cplusplus
}
#endif
