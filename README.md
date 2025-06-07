# ITC: Inter-Thread-Communication
A lock-free, type-erased, SPSC queue for inter-thread communication.  
  
Compiled with C++20.  
Performance is comparable to Boost's SPSC queue.
### Features:
1. A lock-free SPSC ring-buffer as the backend.
2. Type erasure is achieved through external polymorphism.
3. The blocking pop() operation is implemented using atomic wait/notify mechanisms.  

