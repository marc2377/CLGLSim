//------------------------------------//
//                                    //
//  Author : Tiago Lobato Gimenes     //
//  email : tlgimenes at gmail.com    //
//                                    //
//------------------------------------//

#include "grid.h"

// Add the main distance to the sideSize, after the
// execution of the kernel we must divide sideSize 
// by n
__kernel void getGridSideSize(
    __global float4 * pos,
    __global int * sideSize,
    int n)
{
  __private uint i = get_global_id(0);
  __private uint j = (i+1) % n;
  __private float4 dist1, dist2;

  if(i < n){
    dist1 = pos[i];
    dist2 = pos[j];
    dist1.w = 0;
    dist2.w = 0;
    atomic_add(sideSize, (int)fast_distance(dist1, dist2));
//    atomicAdd(sideSize, (int)fast_distance(dist1, dist2));
  }

  return;
}

// Get the ngridCubes values and set the gridCoord index. This 
// function should be used to initialize the nGridCubes and 
// gridCoord, and for anything else
__kernel void getNGridCubes(
    __global float4 * pos,
    __global int4 * gridCoord,
    __global int * nGridCubes,
    __global int * sideSize,
    int offset,
    int n)
{
  __private uint i = offset + get_global_id(0);
  __private uint4 p;

  if(i-offset < n){
    gridCoord[i] = convert_int4_rtz(pos[i - offset]) / *sideSize; //get the coord in the grid
    gridCoord[i].w = i - offset;           //set the index of the corresponding position in pos array
    p = (abs(gridCoord[i]) * 2) + 3; // +2 for padding
    
    // Atomic functions
    atomic_max(nGridCubes, p.x);  
    atomic_max(nGridCubes, p.y);
    atomic_max(nGridCubes, p.z);
  }
  
  return;
}

__kernel void getNGridCubesRefresh(
    __global float4 * SolidPos,
    __global float4 * FluidPos,
    __global int4 * gridCoord,
    __global int2 * gridIndex,
    __global int * nGridCubes,
    __global int * sideSize,
    int n)
{
  __private int i = get_global_id(0);

  __private float4 pos;
  __private uint4 p;
  __private int4 gCoord;
  __private int index;

  if(i < n){
    gCoord = gridCoord[i];
    if(gridIndex[i].y == SOLID){
      pos = SolidPos[gCoord.w];
    }
    else if(gridIndex[i].y == FLUID){
      pos = FluidPos[gCoord.w];
    }
    else{
      return;
    }
    index = gridCoord[i].w;
    gridCoord[i] = convert_int4_rtz(pos) / (*sideSize); //get the coord in the grid
    gridCoord[i].w = index;
    p = (abs(gridCoord[i]) * 2) + 3; // +2 for padding
    
    // Atomic functions
    atomic_max(nGridCubes, p.x);  
    atomic_max(nGridCubes, p.y);
    atomic_max(nGridCubes, p.z);
  }
  return;
}

 
// This function set the index values and can be used more than
// one time
__kernel void setGridIndex(
    __global int2 * gridIndex,
    __global int4 * gridCoord, 
    __global int * nGridCubes,
    int n)
{
  __private unsigned int i = get_global_id(0);
  __private int gridCubes = *nGridCubes;
  __private int gridCubes2 = gridCubes * gridCubes;
  __private int4 aux;

  if(i < n){
    aux = gridCoord[i] + gridCubes / 2;
    gridIndex[i].x = (aux.x * gridCubes2) + (aux.y * gridCubes) + (aux.z); 
  }

  return;
}

// Bubble sort for the even index
__kernel void bubbleSort3D_even(
    __global int2 * gridIndex,
    __global int4 * gridCoord,
    __global bool * modification,
    int n)
{
  __private uint i = get_global_id(0) * 2;
  __private int4 aux;
  __private int2 aux2;

  if(i < n-1){
    if(gridIndex[i].x > gridIndex[i+1].x){
      aux2 = gridIndex[i];
      gridIndex[i] = gridIndex[i+1];
      gridIndex[i+1] = aux2;

      aux = gridCoord[i];
      gridCoord[i] = gridCoord[i+1];
      gridCoord[i+1] = aux;

      *modification = true;
    }
  }
}

// bubble sort for the odd index
__kernel void bubbleSort3D_odd(
    __global int2 * gridIndex,
    __global int4 * gridCoord,
    __global bool * modification,
    int n)
{
  __private uint i = 1 + get_global_id(0) * 2;
  __private int4 aux;
  __private int2 aux2;

  if(i < n-1){
    if(gridIndex[i].x > gridIndex[i+1].x){
      aux2 = gridIndex[i];
      gridIndex[i] = gridIndex[i+1];
      gridIndex[i+1] = aux2;

      aux = gridCoord[i];
      gridCoord[i] = gridCoord[i+1];
      gridCoord[i+1] = aux;

      *modification = true;
    }
  }
}


