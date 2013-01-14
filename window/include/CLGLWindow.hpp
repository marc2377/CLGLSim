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

class CLGLWindow
{
  public:
    // ---------- //
    // Attributes //
    // ---------- //
    // Window Properties
    static int window_height, window_width;
    
    // mouse controls
    static int mouse_old_x, mouse_old_y;
    static int mouse_buttons;
    
    // Axis orientation
    static float rotate_x, rotate_y;
    static float translate_z;
    static float scale[3];
    
    // Glut Window ID
    static int glutWindowHandle;
    
    // Frames per Second
    static float fps;
    
    // Font and characters size
    static bool showInfo;
    static void * font;
    static int fontSize;
    static float stringColor[4];
    
    // If the OpenCL Kernel is running play == ON
    static bool play;
    
    // Number of particles beeing simulated
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
