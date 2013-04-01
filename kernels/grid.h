//------------------------------------//
//                                    //
//  Author : Tiago Lobato Gimenes     //
//  email : tlgimenes at gmail.com    //
//                                    //
//------------------------------------//

#ifndef GRID_H
#define GRID_H

// Enable ATOMIC Functions
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable

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
  index.y = index.x + nPartPerIndex[i];

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

#endif
