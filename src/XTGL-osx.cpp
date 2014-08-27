#include "XTGL.h"

#include <Cocoa/Cocoa.h>
#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

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

void xtglGetEvent(XTGLcontext* ctx, XTGLevent* event)
{
  BasicOpenGLView* view = (BasicOpenGLView*) ctx;
  int* eventBuf = (int*) event;
  // set event type into first i32 slot of event pointer
  eventBuf[0] = [view getEventType];
  // switch([view getEventType]) {
  // case 1: 
  // case 2:
  // case 3:
  // case 4: 
  // default: 
  // }
}

void xtglSwapBuffers(XTGLcontext *ctx)
{    
  //return objc_glSwapBuffers(_sc, args);
  BasicOpenGLView* view = (BasicOpenGLView*) ctx;
  CGLContextObj cgctx = (CGLContextObj) [[view openGLContext] CGLContextObj];
  CGLLockContext(cgctx);
  [[view openGLContext] flushBuffer];
  CGLUnlockContext(cgctx);
  //CGLContextObj ctx = CGLGetCurrentContext();
  //CGLFlushDrawable(ctx);
}

void xtglMakeContextCurrent(XTGLcontext* ctx)
{
  //    return objc_glMakeContextCurrent(_sc, args);
  CGLContextObj cgctx = CGLGetCurrentContext();
  BasicOpenGLView* view = (BasicOpenGLView*) ctx;
  //NSOpenGLView* view = (NSOpenGLView*) cptr_value(pair_car(args));
  cgctx = (CGLContextObj) [[view openGLContext] CGLContextObj];
  //CGLLockContext(cgctx);
  CGLSetCurrentContext(cgctx);
  //CGLUnlockContext(cgctx);    
}




XTGLcontext* xtglCreateContext(int x, int y, int width, int height, char *displayID, int fullscreen)
{
  //return objc_makeGLContext(_sc, args);

  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

  NSRect frameRect = NSMakeRect(x,y,width,height);

  // Get the screen rect of our main display
  NSArray* screens = [NSScreen screens];
  NSScreen* scrn = [NSScreen mainScreen]; // [screens objectAtIndex:0];
    
  NSRect screenRect = frameRect;
  if(fullscreen) screenRect = [scrn frame];
    
  //NSSize size = screenRect.size;
  NSPoint position = screenRect.origin;	
  NSSize size = screenRect.size;
    
  if(fullscreen) {
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
  if(fullscreen){
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
  if(fullscreen) {
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

  return (XTGLcontext*)view; //list; //_cons(_sc, mk_cptr(_sc, (void*)dpy),mk_cptr(_sc,(void*)glxWin),1);
}


XTGLcontext* xtglCreateCoreContext(int x, int y, int width, int height, char *displayID, int fullscreen)
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

  NSRect frameRect = NSMakeRect(x,y,width,height);

  // Get the screen rect of our main display
  NSArray* screens = [NSScreen screens];
  NSScreen* scrn = [NSScreen mainScreen]; // [screens objectAtIndex:0];

  NSRect screenRect = frameRect;
  if(fullscreen) screenRect = [scrn frame];

  //NSSize size = screenRect.size;
  NSPoint position = screenRect.origin;
  NSSize size = screenRect.size;

  if(fullscreen) {
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
  if(fullscreen){
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
  if(fullscreen) {
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
    
    return (XTGLcontext*)view; //list; //_cons(_sc, mk_cptr(_sc, (void*)dpy),mk_cptr(_sc,(void*)glxWin),1);
}
