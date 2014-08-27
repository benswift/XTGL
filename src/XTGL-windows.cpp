#include "XTGL.h"

//#include <unistd.h>
#include <malloc.h>
#include <gl/GL.h>

typedef struct XTGLWGLcontext
{
  HDC  hdc;
  HGLRC hglrc;
} XTGLWGLcontext;

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

int EXT_WIN_MSG_MASK = PM_REMOVE;
 
void xtglGetEvent(XTGLcontext* ctx, XTGLevent* event)
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
    // switch(msg.message) {
    // case WM_CHAR:        
    // case WM_MOUSEMOVE:  
    // default:
    //   TranslateMessage(&msg);
    //   DispatchMessage(&msg);
    // }
  }
  int* eventBuf = (int*)event;
  // set event type into first i32 slot of event pointer
  eventBuf[0] = msg.message;
}
 
    
void xtglSwapBuffers(XTGLcontext *ctx)
{
  XTGLWGLcontext* wglctx = (XTGLWGLcontext*)ctx;
  HDC hdc = wglctx->hdc;
  // HGLRC hglrc = wglctx->hglrc;
  SwapBuffers(hdc);
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
}

  
// void* xtglSwapBuffers(scheme* _sc, pointer args)
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

void xtglMakeContextCurrent(XTGLcontext* ctx)
{
  XTGLWGLcontext* wglctx = (XTGLWGLcontext*)ctx;
  HDC hdc = wglctx->hdc;
  HGLRC hglrc = wglctx->hglrc;

  wglMakeCurrent (hdc, hglrc);
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


void* xtglCreateContext(int x, int y, int width, int height, char *displayID, int fullscreen)
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

  // create a struct to return
  XTGLWGLcontext* ctx = (XTGLWGLcontext*)malloc(sizeof(XTGLWGLcontext));

  ctx->hdc = hdc;
  ctx->hglrc = hglrc;

  return (void*)ctx;
}
