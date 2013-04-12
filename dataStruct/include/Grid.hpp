//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#ifndef GRID_HPP
#define GRID_HPP

#include "CLGL.hpp"
#include "definitions.h"
#include "CLGLDataLoader.hpp"

class Grid
{
  private:
    CLGL * clgl;
    std::vector<cl::Kernel> * kernel;
    std::vector<cl::Buffer> * buff;
    std::vector<cl::Memory> * buffGL;
    int NUM_PART, NUM_PART_FLUID, NUM_PART_SOLID;
    int kerBegin;
    int buffBegin;

  protected:
    void buildGridKernels(void);
    void pushGridDataToDevice(void);
    void setGridKernelArgs(void);
    void startGrid(void);
    void ordenateVector(void);
  public:
    // Contructor : builds grid kernels, allocs 
    // grid memory, set arguments to grids kernels
    Grid(CLGL * clgl, int * NUM_PART);
    
    // Refresh the grid structure. Used when a 
    // particle change of cube in the grid
    void refreshGrid(void);

    int getKerBegin(void);
    int getBuffBegin(void);

    // Debug Function
    void printParameters(void);
};

#endif
