//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#include "CLGLSim.hpp"

/*
 * Initiate static attributes
 */
int CLGLSim::ParticlesNum = 0;
float CLGLSim::rungeStep = 0.1f;
CLGL *CLGLSim::clgl = NULL;
std::vector<int> *CLGLSim::vbo = NULL;

// Kernel
cl::Kernel * CLGLSim::rkx = NULL;

/*
 * Run the kernel Function
 */
void CLGLSim::CLGLRunKernel()
{
  CLGLSim::clgl->CLGLRunKernel(*CLGLSim::rkx, CLGLSim::ParticlesNum);
}
