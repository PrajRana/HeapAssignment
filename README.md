# Manage Heap
+ Implemented heap management algorithms such as best fit, worst fit, next fit, and first fit. 
+ Implemeted splitting and coalescing to manage free blocks. 
+ Implemented realloc and calloc

# Installation
+ git clone https://github.com/PrajRana/HeapAssignment

# To Run
+ cd HeapAssignment
+ make
+ env LD_PRELOAD=lib/libmalloc-ff.so tests/test1  (override existing malloc by LD_PRELOAD)
 ## To run other heap management schemes replace libmalloc-ff.so with
  + Best-Fit: libmalloc-bf.so
  + First-Fit: libmalloc-ff.so
  + Next-Fit: libmalloc-nf.so
  + worst-Fit: libmalloc-wf.so
 ## To run other test cases replace test1 with
 + tests/test2
 + tests/test3
 + tests/test4
  



