#include "XTGL.h"

#include <GL/glx.h>
#include <GL/gl.h>

int singleBufferAttributess[] = {
  GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
  GLX_RENDER_TYPE,   GLX_RGBA_BIT,
  GLX_RED_SIZE,      1,   /* Request a single buffered color buffer */
  GLX_GREEN_SIZE,    1,   /* with the maximum number of color bits  */
  GLX_BLUE_SIZE,     1,   /* for each component                     */
  None
};

int doubleBufferAttributes[] = {
  GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
  GLX_RENDER_TYPE,   GLX_RGBA_BIT,
  GLX_DOUBLEBUFFER,  True,  /* Request a double-buffered color buffer with */
  GLX_RED_SIZE,      1,     /* the maximum number of bits per component    */
  GLX_GREEN_SIZE,    1, 
  GLX_BLUE_SIZE,     1,
  None
};

static Bool EXTGLWaitForNotify( Display *dpy, XEvent *event, XPointer arg ) {
  return (event->type == MapNotify) && (event->xmap.window == (Window) arg);
}

void xtglSwapBuffers(XTGLcontext *ctx)
{
  return glXSwapBuffers((Display*)ctx, (GLXDrawable) cptr_value(pair_cadr(args)));
}

void xtglMakeContextCurrent(XTGLcontext* ctx)
{
  args = pair_car(args);
  GLXDrawable glxWin = (GLXDrawable) cptr_value(pair_cadr(args));
  GLXContext context = (GLXContext) cptr_value(pair_caddr(args));
  /* Bind the GLX context to the Window */
  return glXMakeContextCurrent( (Display*) dpy, (GLXDrawable) glxWin, (GLXDrawable) glxWin, (GLXContext) context);    
}

void xtglGetEvent(XTGLcontext* ctx, XTGLevent* event)
{
  args = pair_car(args);
  Display* dpy = (Display*) cptr_value(pair_car(args));
  XEvent event;
  if(XPending(dpy) == 0) return _sc->NIL;
  //only return the LATEST event. DROP eveything earlier
  while(XPending(dpy)) XNextEvent(dpy, &event);
  switch(event.type){
  case ButtonPress: {
    XButtonEvent be = event.xbutton;
    pointer list = _sc->NIL;
    _sc->imp_env->insert(list);
    pointer tlist = cons(_sc,mk_integer(_sc,be.y),list);
    _sc->imp_env->erase(list);
    list = tlist;
    _sc->imp_env->insert(list);
    tlist = cons(_sc,mk_integer(_sc,be.x),list);
    _sc->imp_env->erase(list);
    list = tlist;
    tlist = cons(_sc,mk_integer(_sc,be.button),list);
    _sc->imp_env->erase(list);
    list = tlist;
    tlist = cons(_sc,mk_integer(_sc,be.state),list);
    _sc->imp_env->erase(list);
    list = tlist;
    tlist = cons(_sc,mk_integer(_sc,0),list);
    _sc->imp_env->erase(list);
    list = tlist;
    return list;
  }
  case MotionNotify: {
    XMotionEvent me = event.xmotion;
    pointer list = _sc->NIL;
    _sc->imp_env->insert(list);
    pointer tlist = cons(_sc,mk_integer(_sc,me.y),list);
    _sc->imp_env->erase(list);
    list = tlist;
    _sc->imp_env->insert(list);
    tlist = cons(_sc,mk_integer(_sc,me.x),list);
    _sc->imp_env->erase(list);
    list = tlist;
    tlist = cons(_sc,mk_integer(_sc,me.state),list);
    _sc->imp_env->erase(list);
    list = tlist;
    tlist = cons(_sc,mk_integer(_sc,1),list);
    _sc->imp_env->erase(list);
    list = tlist;
    return list;
  }
  case KeyPress: {
    XKeyEvent ke = event.xkey;
    pointer list = _sc->NIL;
    _sc->imp_env->insert(list);
    pointer tlist = cons(_sc,mk_integer(_sc,ke.y),list);
    _sc->imp_env->erase(list);
    list = tlist;
    _sc->imp_env->insert(list);
    tlist = cons(_sc,mk_integer(_sc,ke.x),list);
    _sc->imp_env->erase(list);
    list = tlist;
    tlist = cons(_sc,mk_integer(_sc,ke.keycode),list);
    _sc->imp_env->erase(list);
    list = tlist;
    tlist = cons(_sc,mk_integer(_sc,ke.state),list);
    _sc->imp_env->erase(list);
    list = tlist;
    tlist = cons(_sc,mk_integer(_sc,2),list);
    _sc->imp_env->erase(list);
    list = tlist;
    return list;
  }
  default:
    return _sc->NIL;
  }
}

bool checkGLXExtension(Display* dpy,const char* extName)
{
  /*
    Search for extName in the extensions string.  Use of strstr()
    is not sufficient because extension names can be prefixes of
    other extension names.  Could use strtok() but the constant
    string returned by glGetString can be in read-only memory.
  */
  char* list = (char*) glXQueryExtensionsString(dpy, DefaultScreen(dpy));
  char* end;
  int extNameLen;
  extNameLen = strlen(extName);
  end = list + strlen(list);
  while (list < end)
    {
      int n = strcspn(list, " ");
      if ((extNameLen == n) && (strncmp(extName, list, n) == 0))
        return true;
      list += (n + 1);
    };
  return false;
}; // bool checkGLXExtension(const char* extName)

  
void* opengl_render_callback(void* a)
{
  Display              *dpy;
  Window                xWin;
  XEvent                event;
  XVisualInfo          *vInfo;
  XSetWindowAttributes  swa;
  GLXFBConfig          *fbConfigs;
  GLXContext            context;
  GLXContext            sharedContext;
  GLXWindow             glxWin;
  int                   swaMask;
  int                   numReturned;
  int                   swapFlag = True;


  pointer args = (pointer) ((void**)a)[0];
  scheme* _sc = (scheme*) ((void**)a)[1];

  long(*callback)(void) = (long(*)(void)) cptr_value(pair_caddr(pair_cddddr(args)));

  //GLXContext util_glctx;     
  dpy = XOpenDisplay (string_value(pair_car(args)));
  if (dpy == NULL) {
    printf("No such X display\n");
    return _sc->F;     
  }

  /* Request a suitable framebuffer configuration - try for a double buffered configuration first */
  fbConfigs = glXChooseFBConfig( dpy, DefaultScreen(dpy), doubleBufferAttributes, &numReturned );

  if ( fbConfigs == NULL ) {  /* no double buffered configs available */
    fbConfigs = glXChooseFBConfig( dpy, DefaultScreen(dpy), singleBufferAttributess, &numReturned );
    swapFlag = False;
  }

  /* Create an X colormap and window with a visual matching the first
  ** returned framebuffer config */
  vInfo = glXGetVisualFromFBConfig( dpy, fbConfigs[0] );

  // attrList[indx] = GLX_USE_GL; indx++; 
  // attrList[indx] = GLX_DEPTH_SIZE; indx++; 
  // attrList[indx] = 1; indx++; 
  // attrList[indx] = GLX_RGBA; indx++; 
  // attrList[indx] = GLX_RED_SIZE; indx++; 
  // attrList[indx] = 1; indx++; 
  // attrList[indx] = GLX_GREEN_SIZE; indx++; 
  // attrList[indx] = 1; indx++; 
  // attrList[indx] = GLX_BLUE_SIZE; indx++; 
  // attrList[indx] = 1; indx++;     
  // attrList[indx] = None;     
  //vinfo = glXChooseVisual(display, DefaultScreen(display), attrList);     
  if (vInfo == NULL) {
    printf ("ERROR: Can't open window\n"); 
    return _sc->F;
  }    
 

  swa.border_pixel = 0;
  swa.override_redirect = (pair_cadr(args) == _sc->T) ? True : False; 
  swa.event_mask = StructureNotifyMask | KeyPressMask | ButtonPressMask | ButtonMotionMask;
  swa.colormap = XCreateColormap( dpy, RootWindow(dpy, vInfo->screen),
                                  vInfo->visual, AllocNone );

  //swaMask = CWBorderPixel | CWColormap | CWEventMask;
  swaMask = CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect;


  //xWin = XCreateWindow( dpy, RootWindow(dpy, vInfo->screen), 0, 0, 1024, 768,
  //                      0, vInfo->depth, InputOutput, vInfo->visual,
  //                      swaMask, &swa );

  xWin = XCreateWindow( dpy, RootWindow(dpy, vInfo->screen), ivalue(pair_caddr(args)), ivalue(pair_cadddr(args)), ivalue(pair_car(pair_cddddr(args))), ivalue(pair_cadr(pair_cddddr(args))),
                        0, vInfo->depth, InputOutput, vInfo->visual,
                        swaMask, &swa );

  // if we are sharing a context
  if(sharedContext) {
    /* Create a GLX context for OpenGL rendering */
    context = glXCreateNewContext( dpy, fbConfigs[0], GLX_RGBA_TYPE, sharedContext, True );    
  }else{ // if we aren't sharing a context
    /* Create a GLX context for OpenGL rendering */
    context = glXCreateNewContext( dpy, fbConfigs[0], GLX_RGBA_TYPE, NULL, True );
  }

  /* Create a GLX window to associate the frame buffer configuration with the created X window */
  glxWin = glXCreateWindow( dpy, fbConfigs[0], xWin, NULL );
    
  /* Map the window to the screen, and wait for it to appear */
  XMapWindow( dpy, xWin );
  XIfEvent( dpy, &event, EXTGLWaitForNotify, (XPointer) xWin );

  /* Bind the GLX context to the Window */
  glXMakeContextCurrent( dpy, glxWin, glxWin, context );

  void (*swapInterval)(int) = 0;

  if (checkGLXExtension(dpy,"GLX_MESA_swap_control")) {
    swapInterval = (void (*)(int)) glXGetProcAddress((const GLubyte*) "glXSwapIntervalMESA");
  } else if (checkGLXExtension(dpy,"GLX_SGI_swap_control")) {
    swapInterval = (void (*)(int)) glXGetProcAddress((const GLubyte*) "glXSwapIntervalSGI");
  } else {
    printf("no vsync?!\n");
  }

  printf("Is Direct:%d\n",glXIsDirect(dpy,context));

  //glxSwapIntervalSGI(1);
  //glXSwapIntervalMESA(1);

  /* OpenGL rendering ... */
  glClearColor( 0.0, 0.0, 0.0, 1.0 );
  glClear( GL_COLOR_BUFFER_BIT );

  glFlush();

  if ( swapFlag )
    glXSwapBuffers(dpy, glxWin);

  printf("Using OPENGL callback render loop at refresh rate!\n");
  if(pair_cdddr(pair_cddddr(args)) != _sc->NIL) {
    long(*glinit)(void) = (long(*)(void)) cptr_value(pair_cadddr(pair_cddddr(args)));
    glinit();
  }
    
  glXSwapBuffers(dpy, glxWin);
  glFlush();
  swapInterval(1);
    
  while(true) {
    callback();
    glXSwapBuffers(dpy, glxWin);
  }
}

  
void* xtglCreateContext(int x, int y, int width, int height, char *displayID, int fullscreen)
{
  Display              *dpy;
  Window                xWin;
  XEvent                event;
  XVisualInfo          *vInfo;
  XSetWindowAttributes  swa;
  GLXFBConfig          *fbConfigs;
  GLXContext            context;
  GLXContext            sharedContext;
  GLXWindow             glxWin;
  int                   swaMask;
  int                   numReturned;
  int                   swapFlag = True;

  // if(pair_cddr(pair_cddddr(args)) != _sc->NIL) {
  //   EXTThread* render_thread = new EXTThread();
  //   void* v[2];
  //   v[0] = args;
  //   v[1] = _sc;
  //   render_thread->create(&opengl_render_callback,v);
  //   return _sc->T;
  // }

  sharedContext = NULL;
  //if(pair_cdr(args) != _sc->NIL) sharedContext = (GLXContext) cptr_value(pair_cadr(args));

  //GLXContext util_glctx;     
  dpy = XOpenDisplay (displayID);
  if (dpy == NULL) {
    printf("No such X display\n");
    return _sc->F;     
  }

  /* Request a suitable framebuffer configuration - try for a double buffered configuration first */
  fbConfigs = glXChooseFBConfig( dpy, DefaultScreen(dpy), doubleBufferAttributes, &numReturned );

  if ( fbConfigs == NULL ) {  /* no double buffered configs available */
    fbConfigs = glXChooseFBConfig( dpy, DefaultScreen(dpy), singleBufferAttributess, &numReturned );
    swapFlag = False;
  }

  /* Create an X colormap and window with a visual matching the first
  ** returned framebuffer config */
  vInfo = glXGetVisualFromFBConfig( dpy, fbConfigs[0] );

  // attrList[indx] = GLX_USE_GL; indx++; 
  // attrList[indx] = GLX_DEPTH_SIZE; indx++; 
  // attrList[indx] = 1; indx++; 
  // attrList[indx] = GLX_RGBA; indx++; 
  // attrList[indx] = GLX_RED_SIZE; indx++; 
  // attrList[indx] = 1; indx++; 
  // attrList[indx] = GLX_GREEN_SIZE; indx++; 
  // attrList[indx] = 1; indx++; 
  // attrList[indx] = GLX_BLUE_SIZE; indx++; 
  // attrList[indx] = 1; indx++;     
  // attrList[indx] = None;     
  //vinfo = glXChooseVisual(display, DefaultScreen(display), attrList);     
  if (vInfo == NULL) {
    printf ("ERROR: Can't open window\n"); 
    return _sc->F;
  }    
 

  swa.border_pixel = 0;
  swa.override_redirect = (pair_cadr(args) == _sc->T) ? True : False; 
  swa.event_mask = StructureNotifyMask | KeyPressMask | ButtonPressMask | ButtonMotionMask;
  swa.colormap = XCreateColormap( dpy, RootWindow(dpy, vInfo->screen),
                                  vInfo->visual, AllocNone );

  //swaMask = CWBorderPixel | CWColormap | CWEventMask;
  swaMask = CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect;


  //xWin = XCreateWindow( dpy, RootWindow(dpy, vInfo->screen), 0, 0, 1024, 768,
  //                      0, vInfo->depth, InputOutput, vInfo->visual,
  //                      swaMask, &swa );

  xWin = XCreateWindow( dpy, RootWindow(dpy, vInfo->screen), ivalue(pair_caddr(args)), ivalue(pair_cadddr(args)), ivalue(pair_car(pair_cddddr(args))), ivalue(pair_cadr(pair_cddddr(args))),
                        0, vInfo->depth, InputOutput, vInfo->visual,
                        swaMask, &swa );

  // if we are sharing a context
  if(sharedContext) {
    /* Create a GLX context for OpenGL rendering */
    context = glXCreateNewContext( dpy, fbConfigs[0], GLX_RGBA_TYPE, sharedContext, True );    
  }else{ // if we aren't sharing a context
    /* Create a GLX context for OpenGL rendering */
    context = glXCreateNewContext( dpy, fbConfigs[0], GLX_RGBA_TYPE, NULL, True );
  }

  /* Create a GLX window to associate the frame buffer configuration with the created X window */
  glxWin = glXCreateWindow( dpy, fbConfigs[0], xWin, NULL );
    
  /* Map the window to the screen, and wait for it to appear */
  XMapWindow( dpy, xWin );
  XIfEvent( dpy, &event, EXTGLWaitForNotify, (XPointer) xWin );

  /* Bind the GLX context to the Window */
  glXMakeContextCurrent( dpy, glxWin, glxWin, context );

  void (*swapInterval)(int) = 0;

  if (checkGLXExtension(dpy,"GLX_MESA_swap_control")) {
    swapInterval = (void (*)(int)) glXGetProcAddress((const GLubyte*) "glXSwapIntervalMESA");
  } else if (checkGLXExtension(dpy,"GLX_SGI_swap_control")) {
    swapInterval = (void (*)(int)) glXGetProcAddress((const GLubyte*) "glXSwapIntervalSGI");
  } else {
    printf("no vsync?!\n");
  }

  printf("Is Direct:%d\n",glXIsDirect(dpy,context));

  //glxSwapIntervalSGI(1);
  //glXSwapIntervalMESA(1);

  /* OpenGL rendering ... */
  glClearColor( 0.0, 0.0, 0.0, 1.0 );
  glClear( GL_COLOR_BUFFER_BIT );

  glFlush();

  if ( swapFlag )
    glXSwapBuffers(dpy, glxWin);

  if(swapInterval!=0)
    swapInterval(0);    

  pointer list = _sc->NIL;
  _sc->imp_env->insert(list);
  pointer tlist = cons(_sc,mk_cptr(_sc,(void*)context),list);
  _sc->imp_env->erase(list);
  list = tlist;
  _sc->imp_env->insert(list);
  tlist = cons(_sc,mk_cptr(_sc,(void*)glxWin),list);
  _sc->imp_env->erase(list);
  list = tlist;
  tlist = cons(_sc,mk_cptr(_sc,(void*)dpy),list);
  _sc->imp_env->erase(list);
  list = tlist;
		
  return list; //_cons(_sc, mk_cptr(_sc, (void*)dpy),mk_cptr(_sc,(void*)glxWin),1);
}
