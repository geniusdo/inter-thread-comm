#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "MsgQueue.hpp"
#include "backend/RingBuf.hpp"

TEST_CASE("message queue") {
  MsgQueue mq(RingBuffer<int>{10});

  int a = 10;

  mq.enqueue(a);

  int b = 0;

  CHECK(mq.dequeue(b));
  CHECK(b == a);
}

TEST_CASE("message queue multiple enqueue and dequeue") {
  MsgQueue mq(RingBuffer<int>{10});

  for (int i = 0; i < 5; ++i) {
    mq.enqueue(i);
  }

  for (int i = 0; i < 5; ++i) {
    int val      = -1;
    bool success = mq.dequeue(val);
    CHECK(success);
    CHECK(val == i);
  }

  int val      = 0;
  bool success = mq.dequeue(val);
  CHECK(!success);
}

bool float_equal(double a, double b, double epsilon = 1e-5) { return std::abs(a - b) <= epsilon; }

TEST_CASE("MsgQueue with shared_ptr<double>, no Approx") {
  RingBuffer<std::shared_ptr<double>> rb(5);
  MsgQueue mq(std::move(rb));

  auto sp1 = std::make_shared<double>(3.14159);
  mq.enqueue(sp1);

  std::shared_ptr<double> sp_out;
  bool success = mq.dequeue(sp_out);

  CHECK(success);
  REQUIRE(sp_out != nullptr);
  CHECK(float_equal(*sp_out, 3.14159));

  for (int i = 0; i < 3; ++i) {
    mq.enqueue(std::make_shared<double>(i * 1.1));
  }

  for (int i = 0; i < 3; ++i) {
    std::shared_ptr<double> val;
    bool ok = mq.dequeue(val);
    CHECK(ok);
    REQUIRE(val != nullptr);
    CHECK(float_equal(*val, i * 1.1));
  }

  std::shared_ptr<double> empty_out;
  bool empty_pop = mq.dequeue(empty_out);
  CHECK(!empty_pop);
}

TEST_CASE("RingBuffer overwrite old data when full (shared_ptr<double>)") {
  constexpr int capacity = 3;
  RingBuffer<std::shared_ptr<double>> rb(capacity);
  MsgQueue mq(std::move(rb));

  for (int i = 0; i < capacity; ++i) {
    mq.enqueue(std::make_shared<double>(i * 1.0));
  }

  mq.enqueue(std::make_shared<double>(99.9));

  {
    std::shared_ptr<double> val;

    bool ok = mq.dequeue(val);
    CHECK(ok);
    REQUIRE(val != nullptr);
    CHECK(float_equal(*val, 1.0));

    ok = mq.dequeue(val);
    CHECK(ok);
    REQUIRE(val != nullptr);
    CHECK(float_equal(*val, 2.0));

    ok = mq.dequeue(val);
    CHECK(ok);
    REQUIRE(val != nullptr);
    CHECK(float_equal(*val, 99.9));
  }

  std::shared_ptr<double> empty_val;
  bool success = mq.dequeue(empty_val);
  CHECK(!success);
}