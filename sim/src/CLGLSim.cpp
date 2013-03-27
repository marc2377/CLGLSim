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
std::vector<cl::Buffer> *CLGLSim::buffer = NULL;

// Kernel
std::vector<cl::Kernel> * CLGLSim::rkx = NULL;
int CLGLSim::curKernel = 0;

/*
 * Debug Function
 */
void printGPUVector(void)
{
  int * vec = new int[10]; // Attention, it is the size of the vector
  CLGLSim::clgl->CLGLGetDataFromDevice(&(*CLGLSim::buffer)[2], CL_TRUE, sizeof(int) * 10, (void*)vec);

  for(int i=0; i < 10; i++){
    std::cout << " | " << vec[i];
  }
  std::cout << " |" << std::endl;

  return;
}

/*
 * Run the kernel Function
 */
void CLGLSim::CLGLRunKernel()
{
  CLGLSim::clgl->CLGLRunKernel((*CLGLSim::rkx)[0], CLGLSim::ParticlesNum);
  bool boolean = true;
  while(boolean == true){
    // Reset Algorithm
    boolean = false;
    CLGLSim::clgl->CLGLModifyBufferOfDevice(&(*CLGLSim::buffer)[4], CL_TRUE, 0, sizeof(bool), (void*)&boolean);

    // Run two times the algorithm
    CLGLSim::clgl->CLGLRunKernel((*CLGLSim::rkx)[4], 10);
    CLGLSim::clgl->CLGLRunKernel((*CLGLSim::rkx)[5], 10);

    // Get back the value of boolean
    CLGLSim::clgl->CLGLGetDataFromDevice(&(*CLGLSim::buffer)[4], CL_TRUE, sizeof(bool), (void*)&boolean);

    //
    //
    // DEBUG FUNCTION
    //
    //
    //
    printGPUVector();
  }

  std::cout << "Got out of CLGLSim.cpp:67" << std::endl;

  return;
}
