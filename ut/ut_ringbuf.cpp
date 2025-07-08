#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "backend/RingBuf.hpp"

TEST_CASE("RingBuffer move constructor") {
  RingBuffer<int> rb1(5);
  rb1.push(1);
  rb1.push(2);

  RingBuffer<int> rb2(std::move(rb1));

  CHECK(rb1.size() == 2);

  int val;
  CHECK(rb2.try_pop(val));
  CHECK(val == 1);
  CHECK(rb2.try_pop(val));
  CHECK(val == 2);
}

TEST_CASE("RingBuffer<std::shared_ptr<int>> push/pop and refcount") {
  RingBuffer<std::shared_ptr<int>> rb(3);

  auto sp1 = std::make_shared<int>(100);
  auto sp2 = std::make_shared<int>(200);

  CHECK(sp1.use_count() == 1);
  CHECK(sp2.use_count() == 1);

  rb.push(sp1);  // copy push
  CHECK(sp1.use_count() == 2);

  rb.push(std::move(sp2));
  CHECK(sp2 == nullptr);

  CHECK(rb.size() == 2);

  std::shared_ptr<int> out_sp;

  CHECK(rb.try_pop(out_sp));
  CHECK(out_sp != nullptr);
  CHECK(*out_sp == 100);
  CHECK(out_sp.use_count() == 2);

  CHECK(rb.try_pop(out_sp));
  CHECK(out_sp != nullptr);
  CHECK(*out_sp == 200);

  CHECK_FALSE(rb.try_pop(out_sp));
}

TEST_CASE("RingBuffer<std::shared_ptr<int>> overwrite oldest element") {
  RingBuffer<std::shared_ptr<int>> rb(2);

  auto sp1 = std::make_shared<int>(1);
  auto sp2 = std::make_shared<int>(2);
  auto sp3 = std::make_shared<int>(3);

  rb.push(sp1);
  rb.push(sp2);
  CHECK(rb.size() == 2);

  rb.push(sp3);

  CHECK(sp1.use_count() == 1);

  std::shared_ptr<int> out_sp;

  CHECK(rb.try_pop(out_sp));
  CHECK(*out_sp == 2);

  CHECK(rb.try_pop(out_sp));
  CHECK(*out_sp == 3);

  CHECK(rb.empty());
}

TEST_CASE("Iterator") {
  RingBuffer<int> rb1(5);
  size_t cnt = 1;
  rb1.push(1);
  rb1.push(2);
  rb1.push(3);
  rb1.push(4);
  rb1.push(5);
  for (auto& x : rb1) {
    CHECK(x == cnt++);
  }
  rb1.push(6);
  rb1.push(7);
  cnt = 3;
  for (auto& x : rb1) {
    CHECK(x == cnt++);
  }
  rb1.pop();
  rb1.pop();
  cnt = 5;
  for (auto& x : rb1) {
    CHECK(x == cnt++);
  }
  rb1.push(8);
  rb1.push(9);
  cnt = 5;
  for (auto& x : rb1) {
    CHECK(x == cnt++);
  }

  RingBuffer<int> rb2(5);
  cnt = 3;
  rb2.push(7);
  rb2.push(6);
  rb2.push(5);
  rb2.push(4);
  rb2.push(3);
  for (auto it = rb2.rbegin(); it != rb2.rend(); ++it) {
    CHECK(*it == cnt++);
  }
  cnt = 1;
  rb2.push(2);
  rb2.push(1);
  for (auto it = rb2.rbegin(); it != rb2.rend(); ++it) {
    CHECK(*it == cnt++);
  }
  rb2.pop();
  rb2.pop();
  cnt = 1;
  for (auto it = rb2.rbegin(); it != rb2.rend(); ++it) {
    CHECK(*it == cnt++);
  }
  rb1.push(0);
  rb1.push(-1);
  cnt = 1;
  for (auto it = rb2.rbegin(); it != rb2.rend(); ++it) {
    CHECK(*it == cnt++);
  }

  RingBuffer<int> rb3(5);
  cnt = 1;
  rb3.push(1);
  rb3.push(2);
  rb3.push(3);
  rb3.push(4);
  rb3.push(5);
  for (auto it = rb3.cbegin(); it != rb3.cend(); ++it) {
    CHECK(*it == cnt++);
  }
}