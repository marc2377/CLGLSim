//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include "CLGL.hpp"
#include "CLGLSim.hpp"

#define ON 1
#define OFF 0

enum menus {
  PLAY,
  INFO,
  RK1,
  RK2,
  RK4,
  QUIT
};

class CLGLWindow
{
  public:
    // ---------- //
    // Attributes //
    // ---------- //
    // Window Properties
    static int window_height, window_width;
    
    // Mouse Controls
    static int mouse_old_x, mouse_old_y;
    static int mouse_buttons;
    
    // Axis Orientation
    static float rotate_x, rotate_y;
    static float translate_z;
    static float scale[3];
    
    // GLUT Window ID
    static int glutWindowHandle;
 
    // Frames per Second
    static float fps;
    
    // Font and Characters Size
    static bool showInfo;
    static void * font;
    static int fontSize;
    static float stringColor[4];
    
    // If the OpenCL Kernel is running play == ON
    static bool play;
    
    // Number of Particles Beeing Simulated
    static int NumParticles;

    // ------- //
    // Methods //
    // ------- //
    CLGLWindow(void);
    //CLGLWindow(int window_height, int window_width);
    void CLGLRunWindow(void);
};

// ---------------------------- //
// Arguments for GLUT Functions //
// ---------------------------- //
void CLGLWindowRender(void);
void CLGLWindowCreateMenus(void);
void CLGLWindowMenus(int value);
void CLGLWindowDestroy(void);
void CLGLWindowTimerCB(int ms);
void CLGLWindowKeyboard(unsigned char key, int x, int y);
void CLGLWindowSpecialKeys(int key, int x, int y);
void CLGLWindowMouse(int button, int state, int x, int y);
void CLGLWindowMotion(int x, int y);
void CLGLWindowDrawString(const char *str, int x, int y, float color[4], void *font);
void CLGLWindowCalculateFPS(void);
void CLGLWindowDrawInfo(void);

#endif
