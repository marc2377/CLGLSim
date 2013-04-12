//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#ifndef CLGLSIM_HPP
#define CLGLSIM_HPP

#include "CLGL.hpp"
#include "Grid.hpp"
#include "CLGLParser.hpp"
#include "definitions.h"

#include <vector>
#include <stdlib.h>

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

    //Data Struct used in simulation
    static Grid * dataStruct;
    
    //Number of particles beeing simulated
    static int ParticlesNum, NUM_PART_FLUID, NUM_PART_SOLID, indexBufferSize;
    
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
    // Starts the physical kernels and gives them data
    static void CLGLStartPhysics(CLGL * clgl, float rungeStep, data * hostData, CLGLParser * console);
    // Runs the Kernel
    static void CLGLRunKernel(void);
};

#endif
