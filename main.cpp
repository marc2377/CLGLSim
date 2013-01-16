//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#ifdef WIN32
  #include "CLGLWindows.hpp"
#else
  #include "CLGLLinux.hpp"
#endif
#include "CLGLWindow.hpp"
#include "CLGLError.hpp"
#include "CLGLParser.hpp"
#include "CLGLDataLoader.hpp"

#include <string>
#include <fstream>
#include <CL/cl.hpp>

int main(int argc, char * argv[])
{
  float rungeStep;
  int NUM_PART;
  std::string windowTitle = "CLGLSim 1.0";
  std::string kernelFileName = "rk4.cl";
  std::string dataFileName = "data.sim";
  CLGLParser in = CLGLParser(argc, argv);

  CLGLSim::curKernel = in.curKernel;
  NUM_PART = in.particlesNum;
  rungeStep = in.rungeStep;

  std::cout << "-----------------------------------" << std::endl;
  std::cout << "Using " << in.kernel << " Kernel !" << std::endl;
  std::cout << "-----------------------------------" << std::endl;

  body * hostData;

  // Define Window Height and Width
  CLGLWindow::window_height = 768;
  CLGLWindow::window_width = 1366;
 
  try{
    // Start CLGL
#ifdef WIN32
    CLGLWindows clgl = CLGLWindows();
#else
    CLGLLinux clgl = CLGLLinux();
#endif
    clgl.CLGLStart(argc, argv, windowTitle, CLGLWindow::window_height, CLGLWindow::window_width);
    
    // Get Window ID
    CLGLWindow::glutWindowHandle = clgl.CLGLGetWindowID();
    CLGLWindow window = CLGLWindow();

    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Loading Data" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    hostData = loadDataFromFile(dataFileName, &NUM_PART);
   
    // Set the Number of Particles beeing simulated
    CLGLWindow::NumParticles = NUM_PART;

    // Print info About Current Platform and devices
    clgl.CLGLPrintAllInfo();

    // Build the Source of the kernel
    clgl.CLGLBuildProgramSource(kernelFileName);

    // Build the function in.kernel in the kernel 
    clgl.CLGLBuildKernel(in.kernel);
    in.kernel = "Gravity_rk1";
    clgl.CLGLBuildKernel(in.kernel);
    in.kernel = "Gravity_rk2";
    clgl.CLGLBuildKernel(in.kernel);
    in.kernel = "Gravity_rk4";
    clgl.CLGLBuildKernel(in.kernel);

    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Pushing Data to Device" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    clgl.CLGLLoadVBODataToDevice((hostData->pos.size()+1) * sizeof(vector), &(hostData->pos[0]), CL_MEM_READ_WRITE);
    clgl.CLGLLoadVBODataToDevice(hostData->color.size() * sizeof(vector), &(hostData->color[0]), CL_MEM_READ_WRITE);

    clgl.CLGLLoadDataToDevice(CL_TRUE, NUM_PART * sizeof(GLfloat), hostData->mass, CL_MEM_READ_WRITE);
    clgl.CLGLLoadDataToDevice(CL_TRUE, hostData->vel.size() * sizeof(vector), &(hostData->vel[0]), CL_MEM_READ_WRITE);

    std::vector<cl::Buffer> buff = *clgl.CLGLGetBuffer();
    std::vector<cl::Memory> mem = *clgl.CLGLGetBufferGL();
    std::vector<cl::Kernel> ker = *clgl.CLGLGetKernel();

    // Set the Kernel ARGS
    for(unsigned int i=0; i < ker.size(); i++){
      clgl.CLGLSetArg(0, buff[0], ker[i]);
      clgl.CLGLSetArg(1, buff[1], ker[i]);
      clgl.CLGLSetArg(2, mem[0], ker[i]);
      clgl.CLGLSetArg(3, sizeof(float), &rungeStep, ker[i]);
      clgl.CLGLSetArg(4, sizeof(int), &NUM_PART, ker[i]);
    }

    // Set CLGLSim static members
    CLGLSim::vbo = clgl.CLGLGetVBO();
    CLGLSim::ParticlesNum = NUM_PART;
    CLGLSim::rungeStep = rungeStep;
    CLGLSim::clgl = &(clgl);
    CLGLSim::rkx = &ker;

    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Runing the Window" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    window.CLGLRunWindow();
  }
  catch(cl::Error error)
  {
    std::cout << error.what() << CLGLError::errToStr(error.err())->c_str() << std::endl;
  }

  return 0;
}
