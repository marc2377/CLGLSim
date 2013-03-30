//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#ifndef GRID_HPP
#define GRID_HPP

#include "CLGL.hpp"
#include "CLGLDataLoader.hpp"

enum kernelGrid
{
  // Name of each grid's kernel
  getGridSideSize,
  getNGridCubes,
  setGridIndex,
  bubbleSort3D_even,
  bubbleSort3D_odd,
};

enum bufferMemory
{
  gridIndex,              // Index of each Cube
  gridCoord,              // Coord of each particle in the grid
  nGridCubes,             // Number of cubes in each side of the grid
  modificationVectorFlag, // Used in the bubbleSort3D
  sideSize,               // Size of the side of each cube
  rebuildTreeFlag,        // If true needs to rebuild grid
  nPartPerIndex           // Vector of number of particles per cube in the grid
};

class Grid
{
  private:
    CLGL * clgl;
    std::vector<cl::Kernel> * kernel;
    std::vector<cl::Buffer> * buff;
    int NUM_PART;
    int kerBegin;
    int buffBegin;

  protected:
    void buildGridKernels(void);
    void pushGridDataToDevice(void);
    void setGridKernelArgs(cl::Memory pos);
    void startGrid(void);
    void ordenateVector(void);

  public:
    // Contructor : builds grid kernels, allocs 
    // grid memory, set arguments to grids kernels
    Grid(CLGL * clgl, cl::Memory pos, int NUM_PART);
    
    // Refresh the grid structure. Used when a 
    // particle change of cube in the grid
    void refreshGrid(void);

    int getKerBegin(void);
    int getBuffBegin(void);
};

#endif
