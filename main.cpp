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
#include "Grid.hpp"

#include <string>
#include <fstream>
#include <CL/cl.hpp>

int main(int argc, char * argv[])
{
  float rungeStep;
  int NUM_PART;
  unsigned int nPhysicalKernels = 0;
  std::string windowTitle = "CLGLSim 1.0";
  CLGLParser console = CLGLParser(argc, argv);

  CLGLSim::curKernel = console.curKernel;
  NUM_PART = console.particlesNum;
  rungeStep = console.rungeStep;
  std::string kernelFileName = console.kernelFile;

  std::cout << "-----------------------------------" << std::endl;
  std::cout << "Using " << console.kernel << " Kernel !" << std::endl;
  std::cout << "-----------------------------------" << std::endl;

  body * hostData;

  // Define Window Height and Width
  CLGLWindow::window_height = 768;
  CLGLWindow::window_width = 1366;
 
  try{
    // Start CLGL
#ifndef WIN32
    CLGLLinux clgl = CLGLLinux();
#else 
    CLGLWindows clgl = CLGLWindows();
#endif
    clgl.CLGLStart(argc, argv, windowTitle, CLGLWindow::window_height, CLGLWindow::window_width);
    
    // Print info About Current Platform and devices
    clgl.CLGLPrintAllInfo();
    
    // Get Window ID
    CLGLWindow::glutWindowHandle = clgl.CLGLGetWindowID();
    CLGLWindow window = CLGLWindow();

    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Loading Data" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    if(console.isDataFileSet())
      hostData = loadDataFromFile(console.dataFile, &NUM_PART);
    else
        hostData = genData(NUM_PART);
   
    // Set the Number of Particles beeing simulated
    CLGLWindow::NumParticles = NUM_PART;

    /*
     * Loads the physical simulation kernels
     */
    // Build the Source of the kernel
    clgl.CLGLBuildProgramSource(console.kernelFile, "-I./kernels");

    // Build the function console.kernel in the kernel 
    clgl.CLGLBuildKernel(console.kernel);
    console.kernel = "Gravity_rk1";
    clgl.CLGLBuildKernel(console.kernel);
    console.kernel = "Gravity_rk2";
    clgl.CLGLBuildKernel(console.kernel);
    console.kernel = "Gravity_rk4";
    clgl.CLGLBuildKernel(console.kernel);
    //Set the number of physical kernels that were charged
    nPhysicalKernels = clgl.CLGLGetKernel()->size();

    /*
     * Pushing data to Device
     */
    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Pushing Data to Device" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    
    // Physical data
    clgl.CLGLLoadVBODataToDevice(hostData->pos.size() * sizeof(vector), &(hostData->pos[0]), CL_MEM_READ_WRITE);
    clgl.CLGLLoadVBODataToDevice(hostData->color.size() * sizeof(vector), &(hostData->color[0]), CL_MEM_READ_WRITE);

    clgl.CLGLLoadDataToDevice(CL_TRUE, NUM_PART * sizeof(GLfloat), hostData->mass, CL_MEM_READ_ONLY);
    clgl.CLGLLoadDataToDevice(CL_TRUE, hostData->vel.size() * sizeof(vector), &(hostData->vel[0]), CL_MEM_READ_WRITE);

    // Local variables for speed
    std::vector<cl::Buffer> buff = *clgl.CLGLGetBuffer();
    std::vector<cl::Memory> mem = *clgl.CLGLGetBufferGL();
    std::vector<cl::Kernel> ker = *clgl.CLGLGetKernel();

    /*
     * Set Arguments for the kernels
     */
    // Set Physical Kernel ARGS.
    for(unsigned int i=0; i < nPhysicalKernels; i++){
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
    CLGLSim::buffer = &buff;
    CLGLSim::dataStruct = new Grid(&clgl, mem[0], NUM_PART); //Add the data struct to the simulator

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
