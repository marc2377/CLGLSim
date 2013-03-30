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

inline int2 getNeighbors(
    __global int * gridIndex,
    __global int * nPartPerIndex,
    int searchIndex); //Index to search in gridIndex

inline void atomicAdd(
    volatile __global float *source, 
    const float operand);

#endif
