#include "XTGL.h"

//#include <unistd.h>
#include <malloc.h>
#include <gl/GL.h>

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
 
    
void xtglSwapBuffers(XTGLcontext *ctx)
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
