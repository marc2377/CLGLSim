//--------------------------------//
//                                //
//  Author: Tiago Lobato Gimenes  //
//  email: tlgimenes@gmail.com    //
//                                //
//--------------------------------//

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define FLUID 0
#define SOLID 1
#define GRAVITY 2
#define ELETRIC 4

/*
 * Data definitions
 */

typedef struct t_int2
{
  int x,y;
} int2;

typedef struct t_int4
{
  int x,y,z,w;
} int4;

typedef struct t_int8
{
  float s0, s1, s2, s3, s4, s5, s6, s7;
} int8;


typedef struct t_float4
{
  float x,y,z,w;
} float4;


/*
 * Physics definitions
 */

typedef struct t_billard
{
  int size;
  GLfloat * mass;
  GLfloat * radius;
  std::vector<float4> * pos;
  std::vector<float4> * vel;
  std::vector<float4> * color;
} billard;

typedef struct t_solid
{
  int size;
  GLfloat * mass;
  GLfloat * radius;
  std::vector<float4> * pos;
  std::vector<float4> * vel;
  std::vector<float4> * accel;
  std::vector<float4> * color;
  std::vector<float> * lZero;
  std::vector<int> * neighbors;
} solid;

typedef struct t_fluid{
  int size;
  GLfloat * mass;
  GLfloat * radius;
  std::vector<float4> * pos;
  std::vector<float4> * vel;
  std::vector<float4> * color;
} fluid;

typedef struct t_data{
  fluid * f;
  solid * s;
} data;

enum GLBuffer{
  fluidPos,
  fluidColor,
  
  solidPos,
  solidColor,

  indexBuffer
};

enum CLBuffer{
  fluidMass,
  fluidVel,
  fluidDensity,
  
  solidMass,
  solidVel,
  solidAccel,
  lZero,

  debug
};

enum kernel{
  def,
  rk1,
  rk2,
  rk4,
  Memset,
  Solid_accel,
  Solid_rk1,
  Fluid_Density,
  Fluid_rk1,

  copyVBO,
};

/*
 * Grid Definitions
 */

enum kernelGrid
{
  // Name of each grid's kernel
  getGridSideSizeFluid,
  getNGridCubesFluid,

  getGridSideSizeSolid,
  getNGridCubesSolid,
 
  getNGridCubesRefresh,
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
};

#endif
