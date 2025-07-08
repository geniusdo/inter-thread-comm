// Copyright 2025 Chuangye Liu <chuangyeliu0206@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <atomic>
#include <concepts>
#include <memory>

#include "RingIterator.hpp"

template <typename T>
concept SharedPtr = requires(T t) {
  typename T::element_type;  //
  requires std::same_as<T, std::shared_ptr<typename T::element_type>>;
};

// single producer-single consumer ring buffer
// currently only support POD data structre and shared_ptr
template <typename T>
class RingBuffer {
public:
  using BufferElement = T;

  // Old -> New
  using iterator       = RingIterator<RingBuffer<T>, false, false>;
  using const_iterator = RingIterator<RingBuffer<T>, true, false>;

  // New -> Old
  using reverse_iterator       = RingIterator<RingBuffer<T>, false, true>;
  using const_reverse_iterator = RingIterator<RingBuffer<T>, true, true>;

  explicit RingBuffer(size_t capacity = 128)
      : capacity_(capacity + 1), buffer_(std::make_unique<T[]>(capacity + 1)), head_(0), tail_(0) {}

  RingBuffer(RingBuffer&& other) noexcept
      : capacity_(other.capacity_),
        buffer_(std::move(other.buffer_)),
        head_(other.head_.load()),
        tail_(other.tail_.load()) {}

  RingBuffer& operator=(RingBuffer&& other) noexcept {
    if (this != &other) {
      capacity_ = other.capacity_;
      buffer_   = std::move(other.buffer_);
      head_.store(other.head_.load());
      tail_.store(other.tail_.load());
    }
    return *this;
  }

  RingBuffer(const RingBuffer&)            = delete;
  RingBuffer& operator=(const RingBuffer&) = delete;

  template <typename U>
    requires std::is_convertible_v<U&&, T>
  void push(U&& item) {
    size_t current_head = head_.load(std::memory_order_relaxed);
    size_t current_tail = tail_.load(std::memory_order_acquire);
    size_t next_head    = (current_head + 1) % capacity_;
    if (next_head == current_tail) {
      if constexpr (SharedPtr<T>) {  // TODO: reduce redundancy
        buffer_[current_tail].reset();
      }
      current_tail = (current_tail + 1) % capacity_;
      tail_.store(current_tail, std::memory_order_release);
    }
    buffer_[current_head] = std::forward<U>(item);
    head_.store(next_head, std::memory_order_release);
    head_.notify_one();
    return;
  }

  T pop() {
    size_t current_tail = tail_.load(std::memory_order_acquire);
    head_.wait(current_tail, std::memory_order_acquire);
    T item = std::move(buffer_[current_tail]);
    if constexpr (SharedPtr<T>) {
      buffer_[current_tail].reset();
    }
    tail_.store((current_tail + 1) % capacity_, std::memory_order_release);
    return item;
  }

  bool try_pop(T& item) {
    size_t current_tail = tail_.load(std::memory_order_acquire);
    if (current_tail == head_.load(std::memory_order_acquire)) {
      return false;
    }
    item = std::move(buffer_[current_tail]);
    if constexpr (SharedPtr<T>) {  // TODO: reduce redundancy
      buffer_[current_tail].reset();
    }
    tail_.store((current_tail + 1) % capacity_, std::memory_order_release);
    return true;
  }

  size_t size() const {
    return (head_.load(std::memory_order_relaxed) - tail_.load(std::memory_order_relaxed)
            + capacity_)
           % capacity_;
  }

  bool empty() const {
    return head_.load(std::memory_order_relaxed) == tail_.load(std::memory_order_relaxed);
  }

  iterator begin() { return iterator(this, tail_.load(), 0); }

  iterator end() { return iterator(this, head_.load(), this->size()); }

  const_iterator cbegin() const { return const_iterator(this, tail_.load(), 0); }

  const_iterator cend() const { return const_iterator(this, head_.load(), this->size()); }

  reverse_iterator rbegin() {
    return reverse_iterator(this, (head_.load() + capacity_ - 1) % capacity_, 0);
  }

  reverse_iterator rend() {
    return reverse_iterator(this, (tail_.load() + 1) % capacity_, this->size());
  }

  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(this, (head_.load() + capacity_ - 1) % capacity_, 0);
  }

  const_reverse_iterator crend() const {
    return const_reverse_iterator(this, (tail_.load() + 1) % capacity_, this->size());
  }

private:
  template <typename, bool, bool>
  friend class RingIterator;

  T* data() { return buffer_.get(); }

  const T* data() const { return buffer_.get(); }

  const size_t capacity_;
  std::unique_ptr<T[]> buffer_;  // The actual ring buffer
  std::atomic<size_t> head_;     // Points to the next available spot for push
  std::atomic<size_t> tail_;     // Points to the next spot to pop
};