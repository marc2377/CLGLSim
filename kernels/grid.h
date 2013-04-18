//------------------------------------//
//                                    //
//  Author : Tiago Lobato Gimenes     //
//  email : tlgimenes at gmail.com    //
//                                    //
//------------------------------------//

#ifndef GRID_H
#define GRID_H

#define FLUID 0
#define SOLID 1
#define GRAVITY 2
#define ELETRIC 4

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


 /* Get the begin and end of the same index
 * "searchIndex" and returns
 * index.x = index of beggining
 * index.y = index of end + 1
 */
/*inline int2 getNeighbors(
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
}*/

inline int2 getNeighbors(
    __global int2 * gridIndex,
    int searchIndex,
    int n)
{
  int i = n/2;
  int bigger = n;
  int smaller = -1;
  int j = i;
  int id = gridIndex[i].x;
  int2 index;
  index.x = index.y = 0;

 // while((i > 0) && (i < n) && (id != searchIndex || gridIndex[i-1].x == searchIndex)){
  while(i > 0 && i < n){
    if(id == searchIndex && gridIndex[i-1].x != searchIndex){
      break;
    }
    if(bigger - 1 == smaller){
      return index;
    }
    if(id < searchIndex){
      smaller = i;
      i = i + (bigger - i)/2;
    }
    else{
      bigger = i;
      i = i - (i - smaller)/2;
    }
    id = gridIndex[i].x;
 }
 if(gridIndex[i].x != searchIndex){
   return index;
 }
 smaller = j = i;
 bigger = n;
 // while((j < (n - 1)) && (j > 0) && (id != searchIndex || gridIndex[j+1].x == searchIndex)){
 while(j < n-1 && j >= 0){
   if(id == searchIndex && gridIndex[j+1].x != searchIndex){
     break;
   }
   if(id <= searchIndex){
     smaller = j;
     j = j + (bigger-j)/2;
   }
   else{
     bigger = j;
     j = j - (j - smaller)/2;
   }
   id = gridIndex[j].x;
 }
 if(gridIndex[j].x != searchIndex)
   return index;
 index.x = i;
 index.y = j;

 return index;
}

/*
 * Implementation for Atomic Add to store a float number
 */
inline void atomicAdd(
    __global float *source,
    const float operand)
{
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
  } while (atomic_cmpxchg((__global unsigned int *)source, prevVal.intVal, newVal.intVal) != prevVal.intVal);
}

#endif
