//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

// Allow Exceptions
#ifndef __CL_ENABLE_EXCEPTIONS
#define __CL_ENABLE_EXCEPTIONS
#endif

#ifndef CLGLWINDOWS_HPP
#define CLGLWINDOWS_HPP

#include <GL/glew.h>
#include <GL/glut.h>
#include <CL/cl.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <math.h>
#include <string>

#include "CLGLError.hpp"
#include "CLGL.hpp"

class CLGLWindows: public CLGL
{
  protected:
    void CLGLCreateContext(void);
    void CLGLCreateCommandQueue(void);
  public:
    CLGLWindows(void);
};

#endif
