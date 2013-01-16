//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#include "CLGLParser.hpp"

void printHelp(void)
{
  std::cout << "====================================================================" << std::endl;
  std::cout << "|                           CLGLSim 1.0                            |" << std::endl;
  std::cout << "====================================================================" << std::endl;
  std::cout << "|-> OPTIONS                                                        |" << std::endl;
  std::cout << "====================================================================" << std::endl;
  std::cout << "|  --kernel : Defines what kernel to start with, it was implemented|" << std::endl;
  std::cout << "|         Gravity_rk1 Gravity_rk2 and Gravity_rk4 kernels          |" << std::endl;
  std::cout << "--------------------------------------------------------------------" << std::endl;
  std::cout << "|  --num : Defines how many stars to start with                    |" << std::endl;
  std::cout << "--------------------------------------------------------------------" << std::endl;
  std::cout << "|  --precision : Set the precision for simulation                  |" << std::endl; 
  std::cout << "--------------------------------------------------------------------" << std::endl;
  std::cout << "|  --help : Show this help message                                 |" << std::endl; 
  std::cout << "====================================================================" << std::endl;
}

CLGLParser::CLGLParser(int argc, char * argv[])
{
  this->curKernel = 2;
  this->kernel = "Gravity_rk2";
  this->particlesNum = 1000;
  this->rungeStep = 0.001;

  int i;
  for(i=0; i < argc-1; i++){
    // IF --kernel
    if(!strcmp(argv[i], "--kernel")){
      this->kernel = argv[i+1];
      if(argv[i+1][10] == '1')
        this->curKernel = 1;
      else if(argv[i+1][10] == '2')
        this->curKernel = 2;
      else
        this->curKernel = 4;
    }
    // IF --num
    else if(!strcmp(argv[i], "--num"))
      sscanf(argv[i+1], " %d", &(this->particlesNum));
    else if(!strcmp(argv[i], "--precision"))
      sscanf(argv[i+1], " %f", &(this->rungeStep));
    // IF --kernel-file
    if(!strcmp(argv[i], "--help")){
      printHelp();
      exit(0);
    }
  }
  // IF --kernel-file
  if(!strcmp(argv[i], "--help")){
    printHelp();
    exit(0);
  }
}
