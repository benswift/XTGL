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


// this is the new stuff

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

static bool isExtensionSupported(const char *extList, const char *extension)
{
  const char *start;
  const char *where, *terminator;

  /* Extension names should not have spaces. */
  where = strchr(extension, ' ');
  if (where || *extension == '\0')
    return false;

  /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string. Don't be fooled by sub-strings,
     etc. */
  for (start=extList;;) {
    where = strstr(start, extension);

    if (!where)
      break;

    terminator = where + strlen(extension);

    if ( where == start || *(where - 1) == ' ' )
      if ( *terminator == ' ' || *terminator == '\0' )
        return true;

    start = terminator;
  }

  return false;
}

static bool ctxErrorOccurred = false;
static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
    ctxErrorOccurred = true;
    return 0;
}

XTGLcontext* xtglCreateContext(int x, int y, int width, int height, char *displayID, int fullscreen)
{
  Display *display = XOpenDisplay(displayID);

  if (!display)
  {
    printf("Failed to open X display\n");
    return NULL;
  }

  // Get a matching FB config
  static int visual_attribs[] =
    {
      GLX_X_RENDERABLE    , True,
      GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
      GLX_RENDER_TYPE     , GLX_RGBA_BIT,
      GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
      GLX_RED_SIZE        , 8,
      GLX_GREEN_SIZE      , 8,
      GLX_BLUE_SIZE       , 8,
      GLX_ALPHA_SIZE      , 8,
      GLX_DEPTH_SIZE      , 24,
      GLX_STENCIL_SIZE    , 8,
      GLX_DOUBLEBUFFER    , True,
      //GLX_SAMPLE_BUFFERS  , 1,
      //GLX_SAMPLES         , 4,
      None
    };

  int glx_major, glx_minor;

  // FBConfigs were added in GLX version 1.3.
  if ( !glXQueryVersion( display, &glx_major, &glx_minor ) ||
       ( ( glx_major == 1 ) && ( glx_minor < 3 ) ) || ( glx_major < 1 ) )
  {
    printf("Invalid GLX version");
    return NULL;
  }

  printf( "Getting matching framebuffer configs\n" );
  int fbcount;
  GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);
  if (!fbc)
  {
    printf( "Failed to retrieve a framebuffer config\n" );
    return NULL;
  }
  printf( "Found %d matching FB configs.\n", fbcount );

  // Pick the FB config/visual with the most samples per pixel
  printf( "Getting XVisualInfos\n" );
  int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

  int i;
  for (i=0; i<fbcount; ++i)
  {
    XVisualInfo *vi = glXGetVisualFromFBConfig( display, fbc[i] );
    if ( vi )
    {
      int samp_buf, samples;
      glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
      glXGetFBConfigAttrib( display, fbc[i], GLX_SAMPLES       , &samples  );

      printf( "  Matching fbconfig %d, visual ID 0x%2x: SAMPLE_BUFFERS = %d,"
              " SAMPLES = %d\n",
              i, vi -> visualid, samp_buf, samples );

      if ( best_fbc < 0 || samp_buf && samples > best_num_samp )
        best_fbc = i, best_num_samp = samples;
      if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
        worst_fbc = i, worst_num_samp = samples;
    }
    XFree( vi );
  }

  GLXFBConfig bestFbc = fbc[ best_fbc ];

  // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
  XFree( fbc );

  // Get a visual
  XVisualInfo *vi = glXGetVisualFromFBConfig( display, bestFbc );
  printf( "Chosen visual ID = 0x%x\n", vi->visualid );

  printf( "Creating colormap\n" );
  XSetWindowAttributes swa;
  Colormap cmap;
  swa.colormap = cmap = XCreateColormap( display,
                                         RootWindow( display, vi->screen ),
                                         vi->visual, AllocNone );
  swa.background_pixmap = None ;
  swa.border_pixel      = 0;
  swa.event_mask        = StructureNotifyMask;

  printf( "Creating window\n" );
  Window win = XCreateWindow( display, RootWindow( display, vi->screen ),
                              0, 0, 100, 100, 0, vi->depth, InputOutput,
                              vi->visual,
                              CWBorderPixel|CWColormap|CWEventMask, &swa );
  if ( !win )
  {
    printf( "Failed to create window.\n" );
    return NULL;
  }

  // Done with the visual info data
  XFree( vi );

  XStoreName( display, win, "GL 3.0 Window" );

  printf( "Mapping window\n" );
  XMapWindow( display, win );

  // Get the default screen's GLX extension list
  const char *glxExts = glXQueryExtensionsString( display,
                                                  DefaultScreen( display ) );

  // NOTE: It is not necessary to create or make current to a context before
  // calling glXGetProcAddressARB
  glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
  glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
           glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );

  GLXContext ctx = 0;

  // Install an X error handler so the application won't exit if GL 3.0
  // context allocation fails.
  //
  // Note this error handler is global.  All display connections in all threads
  // of a process use the same error handler, so be sure to guard against other
  // threads issuing X commands while this code is running.
  ctxErrorOccurred = false;
  int (*oldHandler)(Display*, XErrorEvent*) =
      XSetErrorHandler(&ctxErrorHandler);

  // Check for the GLX_ARB_create_context extension string and the function.
  // If either is not present, use GLX 1.3 context creation method.
  if ( !isExtensionSupported( glxExts, "GLX_ARB_create_context" ) ||
       !glXCreateContextAttribsARB )
  {
    printf( "glXCreateContextAttribsARB() not found"
            " ... using old-style GLX context\n" );
    ctx = glXCreateNewContext( display, bestFbc, GLX_RGBA_TYPE, 0, True );
  }

  // If it does, try to get a GL 3.0 context!
  else
  {
    int context_attribs[] =
      {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 0,
        //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        None
      };

    printf( "Creating context\n" );
    ctx = glXCreateContextAttribsARB( display, bestFbc, 0,
                                      True, context_attribs );

    // Sync to ensure any errors generated are processed.
    XSync( display, False );
    if ( !ctxErrorOccurred && ctx )
      printf( "Created GL 3.0 context\n" );
    else
    {
      // Couldn't create GL 3.0 context.  Fall back to old-style 2.x context.
      // When a context version below 3.0 is requested, implementations will
      // return the newest context version compatible with OpenGL versions less
      // than version 3.0.
      // GLX_CONTEXT_MAJOR_VERSION_ARB = 1
      context_attribs[1] = 1;
      // GLX_CONTEXT_MINOR_VERSION_ARB = 0
      context_attribs[3] = 0;

      ctxErrorOccurred = false;

      printf( "Failed to create GL 3.0 context"
              " ... using old-style GLX context\n" );
      ctx = glXCreateContextAttribsARB( display, bestFbc, 0,
                                        True, context_attribs );
    }
  }

  // Sync to ensure any errors generated are processed.
  XSync( display, False );

  // Restore the original error handler
  XSetErrorHandler( oldHandler );

  if ( ctxErrorOccurred || !ctx )
  {
    printf( "Failed to create an OpenGL context\n" );
    return NULL;
  }

  // Verifying that context is a direct context
  if ( ! glXIsDirect ( display, ctx ) )
  {
    printf( "Indirect GLX rendering context obtained\n" );
  }
  else
  {
    printf( "Direct GLX rendering context obtained\n" );
  }

  printf( "Making context current\n" );
  glXMakeCurrent( display, win, ctx );

  glClearColor( 0, 0.5, 1, 1 );
  glClear( GL_COLOR_BUFFER_BIT );
  glXSwapBuffers ( display, win );

  // sleep( 1 );

  glClearColor ( 1, 0.5, 0, 1 );
  glClear ( GL_COLOR_BUFFER_BIT );
  glXSwapBuffers ( display, win );

  // sleep( 1 );

  glXMakeCurrent( display, 0, 0 );
  glXDestroyContext( display, ctx );

  XDestroyWindow( display, win );
  XFreeColormap( display, cmap );
  XCloseDisplay( display );

  return NULL;
}

// old stuff starts here

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

// XTGLcontext* xtglCreateContext(int x, int y, int width, int height, char *displayID, int fullscreen)
// {
//   Display              *dpy;
//   Window                xWin;
//   XEvent                event;
//   XVisualInfo          *vInfo;
//   XSetWindowAttributes  swa;
//   GLXFBConfig          *fbConfigs;
//   GLXContext            context;
//   GLXContext            sharedContext;
//   GLXWindow             glxWin;
//   int                   swaMask;
//   int                   numReturned;
//   int                   swapFlag = True;

//   sharedContext = NULL;
//   //if(pair_cdr(args) != _sc->NIL) sharedContext = (GLXContext) cptr_value(pair_cadr(args));

//   //GLXContext util_glctx;
//   dpy = XOpenDisplay (displayID);
//   if (dpy == NULL) {
//     printf("No such X display\n");
//     return NULL;
//   }

//   /* Request a suitable framebuffer configuration - try for a double buffered configuration first */
//   fbConfigs = glXChooseFBConfig( dpy, DefaultScreen(dpy), doubleBufferAttributes, &numReturned );

//   if ( fbConfigs == NULL ) {  /* no double buffered configs available */
//     fbConfigs = glXChooseFBConfig( dpy, DefaultScreen(dpy), singleBufferAttributes, &numReturned );
//     swapFlag = False;
//   }

//   /* Create an X colormap and window with a visual matching the first
//   ** returned framebuffer config */
//   vInfo = glXGetVisualFromFBConfig( dpy, fbConfigs[0] );

//   // attrList[indx] = GLX_USE_GL; indx++;
//   // attrList[indx] = GLX_DEPTH_SIZE; indx++;
//   // attrList[indx] = 1; indx++;
//   // attrList[indx] = GLX_RGBA; indx++;
//   // attrList[indx] = GLX_RED_SIZE; indx++;
//   // attrList[indx] = 1; indx++;
//   // attrList[indx] = GLX_GREEN_SIZE; indx++;
//   // attrList[indx] = 1; indx++;
//   // attrList[indx] = GLX_BLUE_SIZE; indx++;
//   // attrList[indx] = 1; indx++;
//   // attrList[indx] = None;
//   //vinfo = glXChooseVisual(display, DefaultScreen(display), attrList);
//   if (vInfo == NULL) {
//     printf ("ERROR: Can't open window\n");
//     return NULL;
//   }
 

//   swa.border_pixel = 0;
//   swa.override_redirect = (fullscreen == 1) ? True : False;
//   swa.event_mask = StructureNotifyMask | KeyPressMask | ButtonPressMask | ButtonMotionMask;
//   swa.colormap = XCreateColormap( dpy, RootWindow(dpy, vInfo->screen),
//                                   vInfo->visual, AllocNone );

//   //swaMask = CWBorderPixel | CWColormap | CWEventMask;
//   swaMask = CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect;


//   //xWin = XCreateWindow( dpy, RootWindow(dpy, vInfo->screen), 0, 0, 1024, 768,
//   //                      0, vInfo->depth, InputOutput, vInfo->visual,
//   //                      swaMask, &swa );

//   xWin = XCreateWindow( dpy, RootWindow(dpy, vInfo->screen), x, y, width, height,
//                         0, vInfo->depth, InputOutput, vInfo->visual,
//                         swaMask, &swa );

//   // if we are sharing a context
//   if(sharedContext) {
//     /* Create a GLX context for OpenGL rendering */
//     context = glXCreateNewContext( dpy, fbConfigs[0], GLX_RGBA_TYPE, sharedContext, True );
//   }else{ // if we aren't sharing a context
//     /* Create a GLX context for OpenGL rendering */
//     context = glXCreateNewContext( dpy, fbConfigs[0], GLX_RGBA_TYPE, NULL, True );
//   }

//   /* Create a GLX window to associate the frame buffer configuration with the created X window */
//   glxWin = glXCreateWindow( dpy, fbConfigs[0], xWin, NULL );
    
//   /* Map the window to the screen, and wait for it to appear */
//   XMapWindow( dpy, xWin );
//   XIfEvent( dpy, &event, xtglWaitForNotify, (XPointer) xWin );

//   /* Bind the GLX context to the Window */
//   glXMakeContextCurrent( dpy, glxWin, glxWin, context );

//   void (*swapInterval)(int) = 0;

//   if (checkGLXExtension(dpy,"GLX_MESA_swap_control")) {
//     swapInterval = (void (*)(int)) glXGetProcAddress((const GLubyte*) "glXSwapIntervalMESA");
//   } else if (checkGLXExtension(dpy,"GLX_SGI_swap_control")) {
//     swapInterval = (void (*)(int)) glXGetProcAddress((const GLubyte*) "glXSwapIntervalSGI");
//   } else {
//     printf("no vsync?!\n");
//   }

//   printf("Is Direct:%d\n",glXIsDirect(dpy,context));

//   //glxSwapIntervalSGI(1);
//   //glXSwapIntervalMESA(1);

//   /* OpenGL rendering ... */
//   glClearColor( 0.0, 0.0, 0.0, 1.0 );
//   glClear( GL_COLOR_BUFFER_BIT );

//   glFlush();

//   if ( swapFlag )
//     glXSwapBuffers(dpy, glxWin);

//   if(swapInterval!=0)
//     swapInterval(0);

//   // create a struct to return
//   XTGLGLXcontext* ctx = (XTGLGLXcontext*)malloc(sizeof(XTGLGLXcontext));

//   ctx->dpy = dpy;
//   ctx->glxWin = glxWin;
//   ctx->context = context;
		
//   return (void*)ctx;
// }
