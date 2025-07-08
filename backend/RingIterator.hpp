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

template <class RingBufferType, bool IsConst = false, bool IsReverse = false>
class RingIterator {
public:
  using iterator_category = std::forward_iterator_tag;
  using value_type        = typename RingBufferType::BufferElement;
  using difference_type   = std::ptrdiff_t;
  using reference         = std::conditional_t<IsConst, const value_type&, value_type&>;
  using pointer           = std::conditional_t<IsConst, const value_type*, value_type*>;

  using RingBufferPtr = std::conditional_t<IsConst, const RingBufferType*, RingBufferType*>;

  RingIterator(RingBufferPtr owner, size_t pos, size_t count)
      : owner_(owner), pos_(pos), count_(count) {}

  reference operator*() const { return owner_->data()[pos_]; }

  pointer operator->() const { return &(operator*()); }

  RingIterator& operator++() {
    if constexpr (IsReverse) {
      pos_ = (pos_ + owner_->capacity_ - 1) % owner_->capacity_;
    } else {
      pos_ = (pos_ + 1) % owner_->capacity_;
    }
    ++count_;
    return *this;
  }

  bool operator==(const RingIterator& other) const { return count_ == other.count_; }

  bool operator!=(const RingIterator& other) const { return !(*this == other); }

private:
  RingBufferPtr owner_;
  size_t pos_;
  size_t count_;
};
