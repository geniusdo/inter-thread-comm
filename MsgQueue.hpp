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
#include <memory>
#include <utility>

// concept for a message queue
template <typename T>
concept ValidBackend = requires(T t) {
  { t.push(std::declval<typename std::decay_t<T>::BufferElement>()) } -> std::same_as<void>;
  { t.try_pop(std::declval<typename std::decay_t<T>::BufferElement&>()) } -> std::same_as<bool>;
  { t.empty() } -> std::same_as<bool>;
  { t.size() } -> std::same_as<size_t>;
};

// External Polymorphism
class MsgQueue {
public:
  template <ValidBackend T>
  MsgQueue(T&& m) : pimpl(std::make_unique<backend<T>>(std::forward<T>(m))) {}

  MsgQueue(MsgQueue&& m) : pimpl(std::move(m.pimpl)){};

  MsgQueue& operator=(MsgQueue&& m) {
    pimpl = std::move(m.pimpl);
    return *this;
  }

  // delete copy constructor
  MsgQueue(const MsgQueue&)            = delete;
  MsgQueue& operator=(const MsgQueue&) = delete;

  ~MsgQueue() = default;

  template <typename U>
  void enqueue(U&& m) {
    pimpl->push(static_cast<void*>(&m));
  }

  template <typename U>
  bool dequeue(U& m) {
    return pimpl->try_pop(static_cast<void*>(&m));
  }

  bool empty() const { return pimpl->empty(); }

  size_t size() const { return pimpl->size(); }

private:
  struct concept_t {
    // virtual destructor
    virtual ~concept_t() = default;
    // enqueue a message
    virtual void push(void* in) = 0;
    // dequeue a message
    virtual bool try_pop(void* out) = 0;
    // check if the queue is empty
    virtual bool empty() const = 0;
    // get the size of the queue
    virtual size_t size() const = 0;
  };

  template <typename T>
  struct backend final : concept_t {
  public:
    using MessageType = typename std::decay_t<T>::BufferElement;
    backend(T&& m) : instance(std::move(m)) {}

    void push(void* in) override {
      MessageType* typed_in = static_cast<MessageType*>(in);
      instance.push(*typed_in);
      return;
    }

    bool try_pop(void* out) override {
      MessageType* typed_out = static_cast<MessageType*>(out);
      return instance.try_pop(*typed_out);
    }

    bool empty() const override { return instance.empty(); }

    size_t size() const override { return instance.size(); }

  private:
    std::decay_t<T> instance;  // the actual queue
  };

  // bridge
  std::unique_ptr<concept_t> pimpl;
};
