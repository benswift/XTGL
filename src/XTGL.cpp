#include "XTGL.h"

#ifdef _MSC_VER
//#include <unistd.h>
#include <malloc.h>
#include <gl/GL.h>
#elif __linux__
#include <GL/glx.h>
#include <GL/gl.h>
#elif __APPLE__
#include <Cocoa/Cocoa.h>
#include <Foundation/Foundation.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#endif

#ifdef __APPLE__

// BasicOpenGLView
@interface BasicOpenGLView : NSOpenGLView
{
  NSMutableDictionary * stanStringAttrib;
  int eventType;
  float x;
  float y;
  int mbutton;
  char c;
}

- (id)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat *)format;
- (float) getX;
- (float) getY;
- (int) getMButton;
- (char) getC;
- (int) getEventType;
- (void) setEventType:(int)t;
- (void)keyDown:(NSEvent *)theEvent;
- (void) mouseDown:(NSEvent *)theEvent;
- (void) rightMouseDown:(NSEvent *)theEvent;
- (void) otherMouseDown:(NSEvent *)theEvent;
- (void) mouseUp:(NSEvent *)theEvent;
- (void) rightMouseUp:(NSEvent *)theEvent;
- (void) otherMouseUp:(NSEvent *)theEvent;
- (void) mouseDragged:(NSEvent *)theEvent;
- (void) scrollWheel:(NSEvent *)theEvent;
- (void) rightMouseDragged:(NSEvent *)theEvent;
- (void) otherMouseDragged:(NSEvent *)theEvent;
- (BOOL) acceptsFirstResponder;
- (BOOL) becomeFirstResponder;
- (BOOL) resignFirstResponder;
@end

@implementation BasicOpenGLView

- (id)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat *)format
{
  self = [super initWithFrame: frameRect pixelFormat: format];
  x = 0.0f;
  y = 0.0f;
  mbutton = 0; // 1 2 or 3 (0 for no mouse)
  c = '0';
  eventType = -1; // key down (1)  mouse down (2)  mouse up (3)
  // mouse drag (4) 
  return self;
}

- (float) getX { return x; }
- (float) getY { return y; }
- (int) getMButton {return mbutton; }
- (char) getC { return c; }
- (int) getEventType { return eventType; }
- (void) setEventType:(int)t { eventType = t; return; }

// STUFF FOR BasicOpenGLView
-(void)keyDown:(NSEvent *)theEvent
{
  NSString *characters = [theEvent characters];
  if ([characters length]) {
    unichar character = [characters characterAtIndex:0];
    eventType = 1;
    c = (char) character;
  }    
  return;
}

- (void)mouseDown:(NSEvent *)theEvent // trackball
{
  if ([theEvent modifierFlags] & NSControlKeyMask) // send to pan
    [self rightMouseDown:theEvent];
  else if ([theEvent modifierFlags] & NSAlternateKeyMask) // send to dolly
    [self otherMouseDown:theEvent];
  else {
    NSPoint location = [theEvent locationInWindow];
    eventType = 2;
    x = location.x;
    y = location.y;
    mbutton = 1;
  }
  return;
}
 
- (void)rightMouseDown:(NSEvent *)theEvent // pan
{
  NSPoint location = [theEvent locationInWindow];
  eventType = 2;
  x = location.x;
  y = location.y;     
  mbutton = 2;
  return;
}
 
- (void)otherMouseDown:(NSEvent *)theEvent //dolly
{
  NSPoint location = [theEvent locationInWindow];
  eventType = 2;
  x = location.x;
  y = location.y;     
  mbutton = 3;
  return;
}
 
- (void)mouseUp:(NSEvent *)theEvent
{
  NSPoint location = [theEvent locationInWindow];
  eventType = 3;
  x = location.x;
  y = location.y;
  mbutton = 1;
  return;
}
 
- (void)rightMouseUp:(NSEvent *)theEvent
{
  NSPoint location = [theEvent locationInWindow];
  eventType = 3;
  x = location.x;
  y = location.y;
  mbutton = 2;
  return;
}
 
- (void)otherMouseUp:(NSEvent *)theEvent
{
  NSPoint location = [theEvent locationInWindow];
  eventType = 3;
  x = location.x;
  y = location.y;
  mbutton = 3;
  return;
}
 
- (void)mouseDragged:(NSEvent *)theEvent
{
  NSPoint location = [theEvent locationInWindow];
  eventType = 4;
  x = location.x;
  y = location.y;
  mbutton = 1;
  return;
}
 
- (void)scrollWheel:(NSEvent *)theEvent
{
  float wheelDelta = [theEvent deltaX] +[theEvent deltaY] + [theEvent deltaZ];
  return;
}
 
- (void)rightMouseDragged:(NSEvent *)theEvent
{
  NSPoint location = [theEvent locationInWindow];
  eventType = 4;
  x = location.x;
  y = location.y;
  mbutton = 2;
  return;
}
 
- (void)otherMouseDragged:(NSEvent *)theEvent
{
  NSPoint location = [theEvent locationInWindow];
  eventType = 4;
  x = location.x;
  y = location.y;
  mbutton = 3;
  return;
}

- (BOOL)acceptsFirstResponder
{
  return YES;
}
 
- (BOOL)becomeFirstResponder
{
  return  YES;
}
 
- (BOOL)resignFirstResponder
{
  return YES;
}
@end
#endif 


#if defined __linux__

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

Bool glSwapBuffers(Display* dpy)
{
  return glXSwapBuffers(dpy, (GLXDrawable) cptr_value(pair_cadr(args)));
}

Bool glMakeContextCurrent(Display* dpy)
{
  args = pair_car(args);
  GLXDrawable glxWin = (GLXDrawable) cptr_value(pair_cadr(args));
  GLXContext context = (GLXContext) cptr_value(pair_caddr(args));
  /* Bind the GLX context to the Window */
  return glXMakeContextCurrent( (Display*) dpy, (GLXDrawable) glxWin, (GLXDrawable) glxWin, (GLXContext) context);    
}

void* getEvent(Display* dpy)
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

  
void* makeGLContext(void* args)
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

  if(pair_cddr(pair_cddddr(args)) != _sc->NIL) {
    EXTThread* render_thread = new EXTThread();
    void* v[2];
    v[0] = args;
    v[1] = _sc;
    render_thread->create(&opengl_render_callback,v);
    return _sc->T;
  }

  sharedContext = NULL;
  //if(pair_cdr(args) != _sc->NIL) sharedContext = (GLXContext) cptr_value(pair_cadr(args));

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

#elif _MSC_VER

//int singleBufferAttributess[] = {
//  GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
//  GLX_RENDER_TYPE,   GLX_RGBA_BIT,
//  GLX_RED_SIZE,      1,   /* Request a single buffered color buffer */
//  GLX_GREEN_SIZE,    1,   /* with the maximum number of color bits  */
//  GLX_BLUE_SIZE,     1,   /* for each component                     */
//  None
//};

//int doubleBufferAttributes[] = {
//  GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
//  GLX_RENDER_TYPE,   GLX_RGBA_BIT,
//  GLX_DOUBLEBUFFER,  True,  /* Request a double-buffered color buffer with */
//  GLX_RED_SIZE,      1,     /* the maximum number of bits per component    */
//  GLX_GREEN_SIZE,    1, 
//  GLX_BLUE_SIZE,     1,
//  None
//};

//static Bool EXTGLWaitForNotify( Display *dpy, XEvent *event, XPointer arg ) {
//  return (event->type == MapNotify) && (event->xmap.window == (Window) arg);
//}

void* addGLExtension(void* args)
{
  using namespace llvm;
  char* ext_name = string_value(pair_car(args));
  Module* M = EXTLLVM::I()->M;		
  llvm::GlobalValue* gv = M->getNamedValue(std::string(ext_name));
  if( gv == 0 ) {
    printf("Attempting to bind to a non-existant global LLVM variable\n");
    return _sc->F;
  }
  void* ptr = (void*) wglGetProcAddress(ext_name); //cptr_value(pair_cadr(args));
  if( ptr ) {
    EXTLLVM::I()->EE->lock.acquire();	
    EXTLLVM::I()->EE->updateGlobalMapping(gv,ptr); 
    EXTLLVM::I()->EE->lock.release();	
    printf("successfully bound extension:%s\n",ext_name);
    return _sc->T;
  } else {
    printf("Cannot find opengl extension %s\n",ext_name);
    return _sc->F;
  }
}

int EXT_WIN_MSG_MASK = PM_REMOVE;
 
void* getEvent(void* args)
{
  //std::cout << "GET EVENTS!" << std::endl;
  // this here to stop opengl swap buffer code from pinching out input events before we get to them    
  if(EXT_WIN_MSG_MASK==PM_REMOVE) { // this is a temporary hack!
    EXT_WIN_MSG_MASK = PM_REMOVE | PM_QS_PAINT | PM_QS_POSTMESSAGE | PM_QS_SENDMESSAGE;
  }
     
  MSG msg;
 
  // std::cout << "GOING INTO GET MESSAGES" << std::endl;
  // int res = PeekMessage(&msg,NULL,0,0,PM_REMOVE);
  // std::cout << "MSG RES: " << res <<  std::endl;
  while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_QS_INPUT)) {
    //if(res){      
    //std::cout << "GRABBED MSG" << std::endl;
    WPARAM wParam = msg.wParam;
    LPARAM lParam = msg.lParam;
    //std::cout << "GOT MESSAGE: " << &msg << std::endl;
    switch(msg.message) {
    case WM_CHAR: {
      pointer list = _sc->NIL;
      _sc->imp_env->insert(list);
      pointer tlist = cons(_sc,mk_integer(_sc,wParam),list);
      _sc->imp_env->erase(list);
      list = tlist;
      return list;
    }       
    case WM_MOUSEMOVE: {
      pointer list = _sc->NIL;
      _sc->imp_env->insert(list);
      pointer tlist = cons(_sc,mk_integer(_sc,GET_X_LPARAM(lParam)),list);
      _sc->imp_env->erase(list);
      list = tlist;
      _sc->imp_env->insert(list);
      tlist = cons(_sc,mk_integer(_sc,GET_Y_LPARAM(lParam)),list);
      _sc->imp_env->erase(list);
      list = tlist;
      return list;       
    } 
    default:
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      return _sc->NIL;
    }
  }
  return _sc->NIL;
}
 
    
void* glSwapBuffers(void* args)
{
  args = pair_car(args);
  SwapBuffers((HDC)cptr_value(pair_car(args)));
  //EXT_WIN_MSG_MASK = PM_REMOVE | PM_QS_PAINT | PM_QS_POSTMESSAGE | PM_QS_SENDMESSAGE;  
  MSG msg;
  
  //std::cout << "GOING INTO GET MESSAGES" << std::endl;
  while(PeekMessage(&msg, NULL, 0, 0, EXT_WIN_MSG_MASK)) {
    //std::cout << "GOT MESSAGE" << std::endl;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    //std::cout << "msg: " << &msg << std::endl;
  }
  //std::cout << "DONE PROCESSING WIN MESSAGES" << std::endl;
  return _sc->T;
}

  
// void* glSwapBuffers(scheme* _sc, pointer args)
// {
//   args = pair_car(args);
//   SwapBuffers((HDC)cptr_value(pair_car(args)));

//   MSG msg;

//   //std::cout << "GOING INTO GET MESSAGES" << std::endl;
//   while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
//     //std::cout << "GOT MESSAGE" << std::endl;
//     TranslateMessage(&msg);
//     DispatchMessage(&msg);
//     //std::cout << "msg: " << &msg << std::endl;
//   }
//   //std::cout << "DONE PROCESSING WIN MESSAGES" << std::endl;

//   //glXSwapBuffers((Display*) cptr_value(pair_car(args)), (GLXDrawable) cptr_value(pair_cadr(args)));
//   return _sc->T;
// }

void* glMakeContextCurrent(void* args)
{
  wglMakeCurrent ( (HDC)cptr_value(pair_car(args)), (HGLRC)cptr_value(pair_cadr(args)) );
  return _sc->T;
}

// window handler
 
// Step 4: the Window Procedure
LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
    {
    case WM_CLOSE:
      DestroyWindow(hwnd);
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
    }
  return 0;
}


void* makeGLContext(void* args)
{
  PIXELFORMATDESCRIPTOR pfd = { 
    sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd  
    1,                     // version number  
    PFD_DRAW_TO_WINDOW |   // support window  
    PFD_SUPPORT_OPENGL |   // support OpenGL  
    PFD_DOUBLEBUFFER,      // double buffered  
    PFD_TYPE_RGBA,         // RGBA type  
    32,                    // 32-bit color depth  
    0, 0, 0, 0, 0, 0,      // color bits ignored  
    0,                     // no alpha buffer  
    0,                     // shift bit ignored  
    0,                     // no accumulation buffer  
    0, 0, 0, 0,            // accum bits ignored  
    24,                    // 24-bit z-buffer      
    8,                     // no stencil buffer  
    0,                     // no auxiliary buffer  
    PFD_MAIN_PLANE,        // main layer  
    0,                     // reserved  
    0, 0, 0                // layer masks ignored  
  }; 
    
  HDC  hdc;
  HGLRC hglrc;
  int  iPixelFormat; 
  int  posx = ivalue(pair_caddr(args));
  int  posy = ivalue(pair_cadddr(args));
  int  width = ivalue(pair_car(pair_cddddr(args)));
  int  height = ivalue(pair_cadr(pair_cddddr(args)));
  bool  forcetop = (pair_cddr(pair_cddddr(args))==_sc->NIL) ? 0 : 1;    
    
  MSG msg;
  WNDCLASSEX ex;
  HINSTANCE hinstance;
    
  LPCWSTR WNDCLASSNAME = L"GLClass";
  LPCWSTR WNDNAME = L"OpenGL base code";
    
  ex.cbSize = sizeof(WNDCLASSEX);
  ex.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
  ex.lpfnWndProc = WinProc;
  ex.cbClsExtra = 0;
  ex.cbWndExtra = 0;
  ex.hInstance = hinstance;
  ex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  ex.hCursor = LoadCursor(NULL, IDC_ARROW);
  ex.hbrBackground = NULL;
  ex.lpszMenuName = NULL;
  ex.lpszClassName = (LPCWSTR)WNDCLASSNAME;
  ex.hIconSm = NULL;

  RegisterClassEx(&ex);
    
  // center position of the window
  //int posx = (GetSystemMetrics(SM_CXSCREEN) / 2) - (width / 2);
  //int posy = (GetSystemMetrics(SM_CYSCREEN) / 2) - (height / 2);

  // set up the window for a windowed application by default
  long wndStyle; // = WS_OVERLAPPEDWINDOW;
  long dwExStyle; // = WS_OVERLAPPEDWINDOW;
  int screenmode = 0; // NOT FULLSCREEN
  bool fullScreen = false;

  if (pair_cadr(args) == _sc->T){ // if fullscreen
    screenmode = 1;
    fullScreen = true;
  }

  /*      Check if fullscreen is on*/
  if (fullScreen) {
    DEVMODE dmScreenSettings;
    memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
    dmScreenSettings.dmSize = sizeof(dmScreenSettings);
    dmScreenSettings.dmPelsWidth = width;   //screen width
    dmScreenSettings.dmPelsHeight = height; //screen height
    dmScreenSettings.dmBitsPerPel = 32; //bits;   //bits per pixel
    dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	
    if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN != DISP_CHANGE_SUCCESSFUL)) {
      /*      Setting display mode failed, switch to windowed*/
      //MessageBox(NULL, (LPCWSTR)L"Display mode failed", NULL, MB_OK);
      fullScreen = true; //false; 
      screenmode = 1;
    }
  }


  if (fullScreen){ // if fullscreen
    dwExStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
    wndStyle = WS_POPUP;
    screenmode = 1; //FULLSCREEN
    posx = 0;
    posy = 0;
       
    ShowCursor(FALSE);
    // change resolution before the window is created
    //SysSetDisplayMode(screenw, screenh, SCRDEPTH);
  }else{
    dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_TOPMOST;
    wndStyle = WS_OVERLAPPEDWINDOW; //windows style         
    screenmode = 0; //not fullscreen
  }

  RECT    windowRect;

  windowRect.left =(long)0;               //set left value to 0
  windowRect.right =(long)posx;  //set right value to requested width
  windowRect.top =(long)0;                //set top value to 0
  windowRect.bottom =(long)posy;//set bottom value to requested height

  AdjustWindowRectEx(&windowRect, wndStyle, FALSE, dwExStyle);	

  // create the window
  HWND hwnd = CreateWindowEx(NULL,
                             WNDCLASSNAME, //"GLClass",
                             WNDNAME, //"Extempore OpenGL",
                             wndStyle|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
                             posx, posy,
                             width, height,
                             NULL,
                             NULL,
                             hinstance,
                             NULL);

  hdc = GetDC(hwnd);
  if(hdc == NULL) {
    std::cout << "Failed to Get the Window Device Context" << std::endl;
    return _sc->F;
  }
    
	// set pixel format
  iPixelFormat = ChoosePixelFormat(hdc, &pfd);
  SetPixelFormat(hdc, iPixelFormat, &pfd);

  // create a rendering context  
  hglrc = wglCreateContext (hdc); 
  if(hglrc == NULL) {
    std::cout << "Failed to create an OpenGL rendering Context" << std::endl;
    return _sc->F;
  }
    
  // make it the calling thread's current rendering context 
  wglMakeCurrent (hdc, hglrc);
    
  ShowWindow(hwnd, SW_SHOW);		// everything went OK, show the window
  UpdateWindow(hwnd);
  if (forcetop) SetWindowPos(hwnd,HWND_TOPMOST,posx,posy,width,height,SWP_NOOWNERZORDER);

    
  /* Bind the GLX context to the Window */
  wglMakeCurrent( hdc,hglrc );

  std::cout << "hwnd:" << hwnd << " hdc:" << hdc << " hglrc:" << hglrc << std::endl;

  /* OpenGL rendering ... */
  glClearColor( 0.0, 0.0, 0.0, 1.0 );
  glClear( GL_COLOR_BUFFER_BIT );

  glFlush();

  //if ( swapFlag )
  SwapBuffers(hdc);

  //swapInterval(0);    

  pointer list = _sc->NIL;
  _sc->imp_env->insert(list);
  pointer tlist = cons(_sc,mk_cptr(_sc,(void*)hglrc),list);
  _sc->imp_env->erase(list);
  list = tlist;
  _sc->imp_env->insert(list);
  tlist = cons(_sc,mk_cptr(_sc,(void*)hdc),list);
  _sc->imp_env->erase(list);
  list = tlist;
		
  return list; //_cons(_sc, mk_cptr(_sc, (void*)dpy),mk_cptr(_sc,(void*)glxWin),1);
}


#elif __APPLE__

void* getEvent(void* args)
{
  BasicOpenGLView* view = (BasicOpenGLView*) cptr_value(pair_car(args));
  switch([view getEventType]) {
  case 1: {
    pointer list = _sc->NIL;
    _sc->imp_env->insert(list);
    pointer tlist = cons(_sc,mk_integer(_sc,[view getC]),list);
    _sc->imp_env->erase(list);
    list = tlist;
    _sc->imp_env->insert(list);
    tlist = cons(_sc,mk_integer(_sc,[view getEventType]),list);
    _sc->imp_env->erase(list);
    list = tlist;
    [view setEventType:-1];
    return list;
  }
  case 2:
  case 3:
  case 4: {
    pointer list = _sc->NIL;
    _sc->imp_env->insert(list);
    pointer tlist = cons(_sc,mk_integer(_sc,[view getY]),list);
    _sc->imp_env->erase(list);
    list = tlist;
    _sc->imp_env->insert(list);
    tlist = cons(_sc,mk_integer(_sc,[view getX]),list);
    _sc->imp_env->erase(list);
    list = tlist;
    tlist = cons(_sc,mk_integer(_sc,[view getMButton]),list);
    _sc->imp_env->erase(list);
    list = tlist;
    tlist = cons(_sc,mk_integer(_sc,[view getEventType]),list);
    _sc->imp_env->erase(list);
    list = tlist;
    [view setEventType:-1];
    return list;
  }
  default: {      
    [view setEventType:-1];
    return _sc->NIL;
  }
  }
}

void* glSwapBuffers(void* args)
{    
  //return objc_glSwapBuffers(_sc, args);
  BasicOpenGLView* view = (BasicOpenGLView*) cptr_value(pair_car(args));
  CGLContextObj ctx = (CGLContextObj) [[view openGLContext] CGLContextObj];
  CGLLockContext(ctx);
  [[view openGLContext] flushBuffer];
  CGLUnlockContext(ctx);
  //CGLContextObj ctx = CGLGetCurrentContext();
  //CGLFlushDrawable(ctx);
  return _sc->T;
}

void* glMakeContextCurrent(void* args)
{
  //    return objc_glMakeContextCurrent(_sc, args);
  CGLContextObj ctx = CGLGetCurrentContext();
  BasicOpenGLView* view = (BasicOpenGLView*) cptr_value(pair_car(args));
  //NSOpenGLView* view = (NSOpenGLView*) cptr_value(pair_car(args));
  ctx = (CGLContextObj) [[view openGLContext] CGLContextObj];
  //CGLLockContext(ctx);
  CGLSetCurrentContext(ctx);
  //CGLUnlockContext(ctx);    
  return _sc->T;
}




void* makeGLContext(void* args)
{
  //return objc_makeGLContext(_sc, args);
     
  bool fullscrn = (pair_cadr(args) == _sc->T) ? 1 : 0; 
  int  posx = ivalue(pair_caddr(args));
  int  posy = ivalue(pair_cadddr(args));
  int  width = ivalue(pair_car(pair_cddddr(args)));
  int  height = ivalue(pair_cadr(pair_cddddr(args)));

  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

  NSRect frameRect = NSMakeRect(posx,posy,width,height);

  // Get the screen rect of our main display
  NSArray* screens = [NSScreen screens];
  NSScreen* scrn = [NSScreen mainScreen]; // [screens objectAtIndex:0];
    
  NSRect screenRect = frameRect;
  if(fullscrn) screenRect = [scrn frame];
    
  //NSSize size = screenRect.size;
  NSPoint position = screenRect.origin;	
  NSSize size = screenRect.size;
    
  if(fullscrn) {
    position.x = 0;
    position.y = 0;      
    screenRect.origin = position;
  }
      
  NSOpenGLPixelFormatAttribute array[] = 		
    {
      NSOpenGLPFAWindow,
      NSOpenGLPFACompliant,
      NSOpenGLPFANoRecovery,

      // Add these back for multsampling	
      // NSOpenGLPFAMultisample,
      // NSOpenGLPFASampleBuffers, (NSOpenGLPixelFormatAttribute)1,
      // NSOpenGLPFASamples, (NSOpenGLPixelFormatAttribute)4,							
	
      NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute)24,			
      NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)24,
      NSOpenGLPFAAccumSize, (NSOpenGLPixelFormatAttribute)24,
      NSOpenGLPFAAlphaSize, (NSOpenGLPixelFormatAttribute)8,
      NSOpenGLPFAStencilSize, (NSOpenGLPixelFormatAttribute)8,
	
      NSOpenGLPFADoubleBuffer,
      NSOpenGLPFAAccelerated,
      (NSOpenGLPixelFormatAttribute)0				
    };

  NSOpenGLPixelFormat* fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes: (NSOpenGLPixelFormatAttribute*) array]; 
  //NSOpenGLView* view = [[NSOpenGLView alloc] initWithFrame:screenRect pixelFormat:fmt];
  BasicOpenGLView* view = [[BasicOpenGLView alloc] initWithFrame:screenRect pixelFormat:fmt];
    
  int windowStyleMask;
  if(fullscrn){
    windowStyleMask = NSBorderlessWindowMask;
  }else{
    windowStyleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
  }
  NSWindow* window = [[NSWindow alloc] initWithContentRect:screenRect
                      styleMask:windowStyleMask
                      backing:NSBackingStoreBuffered
                      defer:YES screen:scrn];
    
  [window setContentView:view];
  [window useOptimizedDrawing:YES];
  [window setOpaque:YES];
  [window setBackgroundColor:[NSColor colorWithDeviceRed:0.0 green:0.0 blue:0.0 alpha:1.0]];
  if(fullscrn) {
    [window setHasShadow:NO];
    [window makeKeyAndOrderFront:nil];
  }else{	
    [window setTitle:@"Extempore OpenGL Compatibility Profile Window"];
    [window makeKeyAndOrderFront:nil];
  }

  [window display];

  GLint swapInt = 1;
  [[view openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];

  CGLContextObj ctx = (CGLContextObj) [[view openGLContext] CGLContextObj];

  CGLSetCurrentContext(ctx);
  CGLLockContext(ctx);

  // glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  // int glerrors = glGetError();
  // printf("OpenGL Context Errors: %d\n",glerrors);
  CGLEnable( ctx, kCGLCEMPEngine);

  CGLUnlockContext(ctx);

  // pointer list = _sc->NIL;
  // _sc->imp_env->insert(list);
  // pointer tlist = cons(_sc,mk_cptr(_sc,(void*)v),list);
  // _sc->imp_env->erase(list);
  // list = tlist;
  // _sc->imp_env->insert(list);
  // tlist = cons(_sc,mk_cptr(_sc,(void*)hdc),list);
  // _sc->imp_env->erase(list);
  // list = tlist;

  //[pool release];

  return mk_cptr(_sc, view); //list; //_cons(_sc, mk_cptr(_sc, (void*)dpy),mk_cptr(_sc,(void*)glxWin),1);
}


void* makeGLCoreContext(void* args)
{
  //return objc_makeGLContext(_sc, args);
  
  bool fullscrn = (pair_cadr(args) == _sc->T) ? 1 : 0;
  int  posx = ivalue(pair_caddr(args));
  int  posy = ivalue(pair_cadddr(args));
  int  width = ivalue(pair_car(pair_cddddr(args)));
  int  height = ivalue(pair_cadr(pair_cddddr(args)));
  
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

  NSRect frameRect = NSMakeRect(posx,posy,width,height);

  // Get the screen rect of our main display
  NSArray* screens = [NSScreen screens];
  NSScreen* scrn = [NSScreen mainScreen]; // [screens objectAtIndex:0];

  NSRect screenRect = frameRect;
  if(fullscrn) screenRect = [scrn frame];

  //NSSize size = screenRect.size;
  NSPoint position = screenRect.origin;
  NSSize size = screenRect.size;

  if(fullscrn) {
    position.x = 0;
    position.y = 0;
    screenRect.origin = position;
  }

  NSOpenGLPixelFormatAttribute array[] =
    {
      // Add these back for multsampling
      // NSOpenGLPFAMultisample,
      // NSOpenGLPFASampleBuffers, (NSOpenGLPixelFormatAttribute)1,
      // NSOpenGLPFASamples, (NSOpenGLPixelFormatAttribute)4,

      // if these are set, we don't get a Core profile GL canvas
      // NSOpenGLPFAColorSize, (NSOpenGLPixelFormatAttribute)24,
      // NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)24,
      // NSOpenGLPFAAccumSize, (NSOpenGLPixelFormatAttribute)24,
      // NSOpenGLPFAAlphaSize, (NSOpenGLPixelFormatAttribute)8,
      // NSOpenGLPFAStencilSize, (NSOpenGLPixelFormatAttribute)8,

      // for Core Profile (OpenGL 3.2+)
      NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,

      NSOpenGLPFADoubleBuffer,
      NSOpenGLPFAAccelerated,
      (NSOpenGLPixelFormatAttribute)0
    };

  NSOpenGLPixelFormat* fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes: (NSOpenGLPixelFormatAttribute*) array];
  //NSOpenGLView* view = [[NSOpenGLView alloc] initWithFrame:screenRect pixelFormat:fmt];
  BasicOpenGLView* view = [[BasicOpenGLView alloc] initWithFrame:screenRect pixelFormat:fmt];

  int windowStyleMask;
  if(fullscrn){
    windowStyleMask = NSBorderlessWindowMask;
  }else{
    windowStyleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
  }
  NSWindow* window = [[NSWindow alloc] initWithContentRect:screenRect
                      styleMask:windowStyleMask
                      backing:NSBackingStoreBuffered
                      defer:YES screen:scrn];

  [window setContentView:view];
  [window useOptimizedDrawing:YES];
  [window setOpaque:YES];
  [window setBackgroundColor:[NSColor colorWithDeviceRed:0.0 green:0.0 blue:0.0 alpha:1.0]];
  if(fullscrn) {
    [window setHasShadow:NO];
    [window makeKeyAndOrderFront:nil];
  }else{
    [window setTitle:@"Extempore OpenGL Core Profile Window"];
    [window makeKeyAndOrderFront:nil];				
  }	
    
  [window display];
  
  GLint swapInt = 1;
  [[view openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
  
  CGLContextObj ctx = (CGLContextObj) [[view openGLContext] CGLContextObj];
    
  CGLSetCurrentContext(ctx);
  CGLLockContext(ctx);
    
  // glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  // int glerrors = glGetError();
  // printf("OpenGL Context Errors: %d\n",glerrors);
  CGLEnable( ctx, kCGLCEMPEngine);				
    
  CGLUnlockContext(ctx);
    
  // pointer list = _sc->NIL;
  // _sc->imp_env->insert(list);
  // pointer tlist = cons(_sc,mk_cptr(_sc,(void*)v),list);
  // _sc->imp_env->erase(list);
  // list = tlist;
  // _sc->imp_env->insert(list);
  // tlist = cons(_sc,mk_cptr(_sc,(void*)hdc),list);
  // _sc->imp_env->erase(list);
  // list = tlist;

  //[pool release];
    
  return mk_cptr(_sc, view); //list; //_cons(_sc, mk_cptr(_sc, (void*)dpy),mk_cptr(_sc,(void*)glxWin),1);
}

#endif
