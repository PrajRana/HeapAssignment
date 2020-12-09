# Manage Heap
+ Implemented heap management algorithms such as best fit, worst fit, next fit, and first fit. 
+ Implemeted splitting and coalescing to manage free blocks. 
+ Implemented realloc and calloc

# Prerequisites
+ You will need to have virtual machine installed on your computer
  - [VM](https://www.virtualbox.org/wiki/Downloads) is recommended to run this project

# Installation
+ git clone https://github.com/PrajRana/HeapAssignment

# To Run
+ cd HeapAssignment
+ make
+ env LD_PRELOAD=lib/libmalloc-ff.so tests/test (override existing malloc by LD_PRELOAD)

   ## To run first-fit and next-fit,replace libmalloc-ff.so with
   + env LD_PRELOAD=lib/libmalloc-ff.so tests/ffnf
   + First-Fit: libmalloc-ff.so
   + Next-Fit: libmalloc-nf.so
  
   ## To run best-fit and worst-fit,replace libmalloc-ff.so with
   + env LD_PRELOAD=lib/libmalloc-ff.so tests/bfwf
   + Best-Fit: libmalloc-bf.so
   + worst-Fit: libmalloc-wf.so
  
  ## To run other test cases,replace tests/test1 with
  + env LD_PRELOAD=lib/libmalloc-bf.so tests/test1
  + To test malloc and free: tests/test1
  + To test malloc and free: tests/test2
  + To test coalesce: tests/test3
  + To test split and reuse: tests/test4
  



