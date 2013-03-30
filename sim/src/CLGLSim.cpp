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
CLGL * CLGLSim::clgl = NULL;
Grid * CLGLSim::dataStruct = NULL;
std::vector<int> *CLGLSim::vbo = NULL;
std::vector<cl::Buffer> *CLGLSim::buffer = NULL;

// Kernel
std::vector<cl::Kernel> * CLGLSim::rkx = NULL;
int CLGLSim::curKernel = 0;

/*
 * Run the kernel Function
 */
void CLGLSim::CLGLRunKernel()
{
  bool isTreeIncorrect = false;

  // Runs the kernel
  CLGLSim::clgl->CLGLRunKernel((*CLGLSim::rkx)[0], CLGLSim::ParticlesNum);

  // Get the isTreeIncorrect from device
  CLGLSim::clgl->CLGLGetDataFromDevice(&(*CLGLSim::buffer)[CLGLSim::dataStruct->getBuffBegin() + rebuildTreeFlag], CL_TRUE, sizeof(bool), &isTreeIncorrect);
  if(isTreeIncorrect)
    CLGLSim::dataStruct->refreshGrid();

  return;
}
