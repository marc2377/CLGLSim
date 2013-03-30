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
    __global float * sideSize,
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
    atomicAdd(sideSize, fast_distance(dist1, dist2));
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
    __global float * sideSize,
    int n)
{
  __private uint i = get_global_id(0);
  __private uint4 p;

  if(i < n){
    gridCoord[i] = convert_int4_rtz(pos[i]) / *sideSize; //get the coord in the grid
    gridCoord[i].w = i;           //set the index of the corresponding position in pos array
    p = (abs(gridCoord[i]) * 2) + 2;             // private is faster
    
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
    __global float4 * pos,
    __global int * gridIndex,
    __global int4 * gridCoord, 
    __global int * nGridCubes,
    __global int * nPartPerIndex,
    int n)
{
  __private unsigned int i = get_global_id(0);
  __private int gridCubes = *nGridCubes / 2;
  __private int gridCubes2 = gridCubes * gridCubes;
  __private int gridCubes3 = gridCubes2 * gridCubes;
  __private int4 aux;

  if(i < n){
    aux = gridCoord[i];
    gridIndex[i] = (aux.x * gridCubes2) + (aux.y * gridCubes) + (aux.z) + gridCubes3; 
    atomic_inc(&(nPartPerIndex[gridIndex[i]]));
  }

  return;
}

// Bubble sort for the even index
__kernel void bubbleSort3D_even(
    __global int * gridIndex,
    __global int4 * gridCoord,
    __global bool * modification,
    int n)
{
  __private uint i = get_global_id(0) * 2;
  __private int4 aux;

  if(i < n-1){
    if(gridIndex[i] > gridIndex[i+1]){
      aux.x = gridIndex[i];
      gridIndex[i] = gridIndex[i+1];
      gridIndex[i+1] = aux.x;

      aux = gridCoord[i];
      gridCoord[i] = gridCoord[i+1];
      gridCoord[i+1] = aux;

      *modification = true;
    }
  }
}

// bubble sort for the odd index
__kernel void bubbleSort3D_odd(
    __global int * gridIndex,
    __global int4 * gridCoord,
    __global bool * modification,
    int n)
{
  __private uint i = 1 + get_global_id(0) * 2;
  __private int4 aux;

  if(i < n-1){
    if(gridIndex[i] > gridIndex[i+1]){
      aux.x = gridIndex[i];
      gridIndex[i] = gridIndex[i+1];
      gridIndex[i+1] = aux.x;

      aux = gridCoord[i];
      gridCoord[i] = gridCoord[i+1];
      gridCoord[i+1] = aux;

      *modification = true;
    }
  }
}

/*
 * --------------------------------------------
 *              Helper Functions              |
 * --------------------------------------------
 */

/*
 * Get the begin and end of the same index
 * "searchIndex" and returns
 * index.x = index of beggining
 * index.y = index of end + 1
 */
inline int2 getNeighbors(
    __global int * gridIndex,
    __global int * nPartPerIndex,
    int searchIndex) //Index to search in gridIndex
{
  __private int2 index = 0;
  __private uint i;

  for(i=0; i < searchIndex; i++){
    index.x += nPartPerIndex[i];
  }
  index.y = nPartPerIndex[i];

  return index;
}

/*
 * Implementation for Atomic Add to store a float number
 */
inline void atomicAdd(volatile __global float *source, const float operand) {
  union {
    unsigned int intVal;
    float floatVal;
  } newVal;
  union {
    unsigned int intVal;
    float floatVal;
  } prevVal;
  do {
    prevVal.floatVal = *source;
    newVal.floatVal = prevVal.floatVal + operand;
  } while (atomic_cmpxchg((volatile __global unsigned int *)source, prevVal.intVal, newVal.intVal) != prevVal.intVal);
}
