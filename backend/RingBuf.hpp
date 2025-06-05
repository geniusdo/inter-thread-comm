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
#include <memory>
#include <concepts>

template <typename T>
concept SharedPtr = requires(T t) {
    typename T::element_type; //
    requires std::same_as<T, std::shared_ptr<typename T::element_type>>;
};

// single producer-single consumer ring buffer
// currently only support POD data structre and shared_ptr
template <typename T>
class RingBuffer
{
public:
    explicit RingBuffer(size_t capacity = 128)
        : capacity_(capacity + 1), buffer_(std::make_unique<T[]>(capacity + 1)),
          head_(0), tail_(0) {}
    void push(T item)
    {
        size_t current_head = head_.load(std::memory_order_relaxed);
        size_t current_tail = tail_.load(std::memory_order_acquire);
        size_t next_head = (current_head + 1) % capacity_;
        if (next_head == current_tail)
        {
            if constexpr (SharedPtr<T>)
            {
                buffer_[current_tail].reset();
            }
            current_tail = (current_tail + 1) % capacity_;
            tail_.store(current_tail, std::memory_order_release);
        }
        buffer_[current_head] = item;
        head_.store(next_head, std::memory_order_release);
        head_.notify_one();
        return;
    }

    T pop()
    {
        size_t current_tail = tail_.load(std::memory_order_acquire);
        head_.wait(current_tail, std::memory_order_acquire);
        T item = buffer_[current_tail];
        if constexpr (SharedPtr<T>)
        {
            buffer_[current_tail].reset();
        }
        tail_.store((current_tail + 1) % capacity_, std::memory_order_release);
        return item;
    }

    bool try_pop(T &item)
    {
        size_t current_tail = tail_.load(std::memory_order_acquire);
        if (current_tail == head_.load(std::memory_order_acquire))
        {
            return false;
        }
        item = buffer_[current_tail];
        if constexpr (SharedPtr<T>)
        {
            buffer_[current_tail].reset();
        }
        tail_.store((current_tail + 1) % capacity_, std::memory_order_release);
        return true;
    }

    size_t size() const
    {
        return (head_.load(std::memory_order_relaxed) - tail_.load(std::memory_order_relaxed)) % capacity_;
    }

    bool empty() const
    {
        return head_.load(std::memory_order_relaxed) == tail_.load(std::memory_order_relaxed);
    }

private:
    const size_t capacity_;
    std::unique_ptr<T[]> buffer_; // The actual ring buffer
    std::atomic<size_t> head_;    // Points to the next available spot for push
    std::atomic<size_t> tail_;    // Points to the next spot to pop
};