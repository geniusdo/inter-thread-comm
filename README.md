# ITC: Inter-Thread-Communication
A lock-free, type-erased, SPSC queue for inter-thread communication.  
  
Compiled with C++20.  
Performance is comparable to Boost's SPSC queue.
### Features:
1. A lock-free SPSC ring-buffer as the backend.
2. Type erasure is achieved through external polymorphism.
3. The blocking pop() operation is implemented using atomic wait/notify mechanisms.  

### Usage:

#### Initialization
I recommend two ways to initialize a MsgQueue
```cpp
#include "MsgQueue.hpp"

//select a backend 
#include "backend/Ringbuf.hpp"

// 1. Directly initialize a MsgQueue using a rvalue

MsgQueue mq1(Ringbuffer<int>{128}); // here RingBuffer is selected as the backend

// 2. Initialize a MsgQueue through an existed backend
Ringbuffer<int>{128} rb;
MsgQueue mq2(std::move(rb));
``` 

#### Enqueue/Dequeue
Note that there is **NO type checking** when enqueueing and dequeueing.  
You should pay attention to the type of the data you enqueue and dequeue.
```cpp
//suppose we have a MsgQueue
MsgQueue mq1(Ringbuffer<int>{128});

mq1.enqueue(1); // enqueue an int   

int a;
mq1.dequeue(a); // dequeue an int
```