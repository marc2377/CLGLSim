//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#define VERSION "CLGLSim 1.0"

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
  std::string windowTitle = VERSION;
  CLGLParser console = CLGLParser(argc, argv);

  CLGLSim::curKernel = console.curKernel;
  NUM_PART = console.particlesNum;
  rungeStep = console.rungeStep;
  std::string kernelFileName = console.kernelFile;

  std::cout << "-----------------------------------" << std::endl;
  std::cout << "Using " << console.kernel << " Kernel !" << std::endl;
  std::cout << "-----------------------------------" << std::endl;

  data * hostData;

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
    //if(console.isDataFileSet())
      //hostData = loadDataFromFile(console.dataFile, &NUM_PART);
    //else
      hostData = genData(console.NUM_PART_FLUID, console.NUM_PART_SOLID);

    // Set the Number of Particles beeing simulated
    CLGLWindow::NumParticles = NUM_PART;

    // Starts Physics
    CLGLSim::CLGLStartPhysics(&clgl, rungeStep, hostData, &console);

    std::cout << "-----------------------------------" << std::endl;
    std::cout << "Runing the Window" << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    window.CLGLRunWindow();
  }
  catch(cl::Error error)
  {
    std::cout << error.what() << CLGLError::errToStr(error.err())->c_str() << std::endl;
    exit(EXIT_FAILURE);
  }

  return 0;
}
