#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "MsgQueue.hpp"
#include "backend/RingBuf.hpp"

TEST_CASE("message queue") {
  RingBuffer<int> queue(10);

  MsgQueue mq(queue);

  int a = 10;

  mq.enqueue(a);

  int b = 0;

  CHECK(mq.dequeue(b));
  CHECK(b == a);
}