//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#ifndef CLGLSIM_HPP
#define CLGLSIM_HPP

#include "CLGL.hpp"

#include <vector>
#include <stdlib.h>

enum kernel{
  rk1,
  rk2,
  rk4
};

class CLGLSim
{
  public:
    // ---------- //
    // Attributes //
    // ---------- //
    //Kernel
    static std::vector<cl::Kernel> * rkx;
    static int curKernel;

    //CLGL object
    static CLGL * clgl;
    
    //Number of particles beeing simulated
    static int ParticlesNum;
    
    //Runge Step used
    static float rungeStep;
    
    //Vbo index array
    static std::vector<int> *vbo;
    
    //Vbo buffer array
    static std::vector<cl::Memory> bufferGL;
    
    //Buffer of device
    static std::vector<cl::Buffer>* buffer;

    // ------- //
    // Methods //
    // ------- //
    //Runs the Kernel
    static void CLGLRunKernel(void);
};

#endif
