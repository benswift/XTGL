#include "XTGL.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <GL/glx.h>

typedef struct XTGLGLXcontext
{
  Display*      dpy;
  GLXWindow     glxWin;
  GLXContext    context;
} XTGLGLXcontext;

int singleBufferAttributes[] = {
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

static Bool xtglWaitForNotify( Display *dpy, XEvent *event, XPointer arg ) {
  return (event->type == MapNotify) && (event->xmap.window == (Window) arg);
}

void xtglSwapBuffers(XTGLcontext *ctx)
{
  // glXSwapBuffers((Display*)ctx, (GLXDrawable) cptr_value(pair_cadr(args)));
}

void xtglMakeContextCurrent(XTGLcontext* ctx)
{
  // unpack the struct which is flying around
  XTGLGLXcontext* glxctx = (XTGLGLXcontext*)ctx;
  Display* dpy = glxctx->dpy;
  GLXDrawable glxWin = glxctx->glxWin;
  GLXContext context = glxctx->context;
  /* Bind the GLX context to the Window */
  glXMakeContextCurrent(dpy, glxWin, glxWin, context);    
}

void xtglGetEvent(XTGLcontext* ctx, XTGLevent* event)
{
  // unpack the struct which is flying around
  XTGLGLXcontext* glxctx = (XTGLGLXcontext*)ctx;
  Display* dpy = glxctx->dpy;
  GLXDrawable glxWin = glxctx->glxWin;
  GLXContext context = glxctx->context;
  XEvent xev;
  if(XPending(dpy) == 0) return;
  //only return the LATEST event. DROP eveything earlier
  while(XPending(dpy)) XNextEvent(dpy, &xev);
  // switch(event.type){
  // case ButtonPress: 
  // case MotionNotify: 
  // case KeyPress: 
  // default:
  // }
  int* eventBuf = (int*) event;
  // set event type into first i32 slot of event pointer
  eventBuf[0] = xev.type;
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

XTGLcontext* xtglCreateContext(int x, int y, int width, int height, char *displayID, int fullscreen)
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

  sharedContext = NULL;
  //if(pair_cdr(args) != _sc->NIL) sharedContext = (GLXContext) cptr_value(pair_cadr(args));

  //GLXContext util_glctx;
  dpy = XOpenDisplay (displayID);
  if (dpy == NULL) {
    printf("No such X display\n");
    return NULL;
  }

  /* Request a suitable framebuffer configuration - try for a double buffered configuration first */
  fbConfigs = glXChooseFBConfig( dpy, DefaultScreen(dpy), doubleBufferAttributes, &numReturned );

  if ( fbConfigs == NULL ) {  /* no double buffered configs available */
    fbConfigs = glXChooseFBConfig( dpy, DefaultScreen(dpy), singleBufferAttributes, &numReturned );
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
    return NULL;
  }


  swa.border_pixel = 0;
  swa.override_redirect = (fullscreen == 1) ? True : False;
  swa.event_mask = StructureNotifyMask | KeyPressMask | ButtonPressMask | ButtonMotionMask;
  swa.colormap = XCreateColormap( dpy, RootWindow(dpy, vInfo->screen),
                                  vInfo->visual, AllocNone );

  //swaMask = CWBorderPixel | CWColormap | CWEventMask;
  swaMask = CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect;


  //xWin = XCreateWindow( dpy, RootWindow(dpy, vInfo->screen), 0, 0, 1024, 768,
  //                      0, vInfo->depth, InputOutput, vInfo->visual,
  //                      swaMask, &swa );

  xWin = XCreateWindow( dpy, RootWindow(dpy, vInfo->screen), x, y, width, height,
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
  XIfEvent( dpy, &event, xtglWaitForNotify, (XPointer) xWin );

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

  // create a struct to return
  XTGLGLXcontext* ctx = (XTGLGLXcontext*)malloc(sizeof(XTGLGLXcontext));

  ctx->dpy = dpy;
  ctx->glxWin = glxWin;
  ctx->context = context;

  return (void*)ctx;
}
