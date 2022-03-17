// *************************************************************************
//  This file is the Vec32 container implementation by Steaphan Greene
//
//  Vec32 is a minimal vector-like container implementation, designed to
//  reduce both the memory and CPU footprint of small vectors of 64-bit
//  objects, such as pointers.
//
//  Vec32 is essentially just a special case of small-vector-optimized
//  vector, where only capacity C is stored within the class, and for all
//  higher capacities, the data is stored in a dynamic array, like vector.
//
//  The length and capacity are limited to a 32-bit unsigned value, which
//  reduces the total size of each Vec32<1> to 16 bytes, making it always
//  smaller than vector, except perhaps in some cases with untuned capacity.
//
//  The capacity of Vec32 is never reduced, except on object destruction,
//  or when being moved from.
//
//  The cap_ member of Vec32 is the capacity, or is zero if the capacity
//  is the default (which is C).  This is so most of the comparisons can be
//  with zero (which is often best optimized), and so this class can be
//  trivially default initialized by setting all memory to zeros.
//
//  Vec32 is not a complete container implementation, as only the elements
//  which have actually been needed so far have been implemented.
//
//  Copyright 2022 Steaphan Greene <steaphan@gmail.com>
//
//  Vec32 is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  Vec32 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Vec32 (see the file named "COPYING");
//  If not, see <http://www.gnu.org/licenses/>.
//
// *************************************************************************

#include <cassert>
#include <cstdint>

#ifndef VEC32_HPP
#define VEC32_HPP

#include "minvec.hpp"

template <typename T>
class Vec32 {
 public:
  using iterator = T*;
  using const_iterator = T const*;

  Vec32() = default;
  Vec32(const Vec32& in) {
    cap_ = in.cap_;
    size_ = in.size_;
    data_ = new T[cap_];
    for (uint32_t idx = 0; idx < size_; ++idx) {
      data_[idx] = in.data_[idx];
    }
  };
  Vec32(Vec32&& in) noexcept(noexcept(data_ = in.data_)) {
    cap_ = in.cap_;
    size_ = in.size_;
    data_ = in.data_;
    in.cap_ = 0;
    in.size_ = 0;
  };

  void operator=(const Vec32& in) {
    if (cap_ != 0) {
      delete[] data_;
    }
    cap_ = in.cap_;
    size_ = in.size_;
    data_ = new T[cap_];
    for (uint32_t idx = 0; idx < size_; ++idx) {
      data_[idx] = in.data_[idx];
    }
  };
  void operator=(Vec32&& in) {
    if (cap_ != 0) {
      delete[] data_;
    }
    cap_ = in.cap_;
    size_ = in.size_;
    data_ = in.data_;
    in.cap_ = 0;
    in.size_ = 0;
  };

  bool operator==(const Vec32& in) const {
    if (size_ != in.size_) {
      return false;
    } else {
      for (uint32_t idx = 0; idx < size_; ++idx) {
        if (data_[idx] != in.data_[idx]) {
          return false;
        }
      }
      return true;
    }
  };

  ~Vec32() {
    if (cap_ != 0) {
      delete[] data_;
    }
  };

  T& operator[](int idx) {
    return data_[idx];
  };

  bool empty() const {
    return size_ == 0;
  };
  std::size_t size() const {
    return size_;
  };
  std::size_t capacity() const {
    return cap_;
  };

  void reserve(std::size_t cap) {
    if (cap > cap_) {
      assert(cap <= 0x80000000UL);
      if (cap_ == 0) {
        auto temp = data_;
        cap_ = cap;
        data_ = new T[cap_];
      } else {
        T* temp = data_;
        cap_ = cap;
        data_ = new T[cap_];
        for (uint32_t idx = 0; idx < size_; ++idx) {
          data_[idx] = temp[idx];
        }
        delete[] temp;
      }
    }
  };
  void clear() {
    size_ = 0;
  };
  void erase(auto b) {
    --size_;
    for (auto itr = b; itr != end(); ++itr) {
      *itr = *(itr + 1);
    }
  };
  void erase(auto b, auto e) {
    assert(e == end()); // Only support truncation of end.
    size_ = b - begin(); // Assume deleting all the rest.
  };
  void emplace(auto to, auto b, auto e) {
    insert(to, b, e);
  }
  void insert(auto to, auto b, auto e) {
    assert(to == end()); // Only support appending to end.
    for (auto idx = b; idx != e; ++idx) {
      push_back(*idx); // Assuming appending to the end.
    }
  };
  void emplace(auto to, const T& in) {
    insert(to, in);
  }
  void insert(auto to, const T& in) {
    if (to == end()) {
      push_back(in);
    } else {
      T temp = back();
      for (auto itr = end() - 1; itr != to; --itr) {
        *itr = *(itr - 1);
      }
      *to = in;
      push_back(temp);
    }
  };

  void pop_back() {
    --size_;
  };

  void push_back(T in) {
    if (cap_ == 0) {
      cap_ = 1;
      data_ = new T[cap_];
      data_[size_] = in;
    } else if (size_ < cap_) {
      data_[size_] = in;
    } else {
      assert(cap_ <= 0x40000000U);
      T* temp = data_;
      cap_ *= 2;
      data_ = new T[cap_];
      for (uint32_t idx = 0; idx < size_; ++idx) {
        data_[idx] = temp[idx];
      }
      delete[] temp;
      data_[size_] = in;
    }
    ++size_;
  };

  T& front() {
    return data_[0];
  };
  T const& front() const {
    return data_[0];
  };
  T& back() {
    return data_[size_ - 1];
  };
  T const& back() const {
    return data_[size_ - 1];
  };

  T* begin() {
    return data_;
  };
  T const* begin() const {
    return data_;
  };
  T const* cbegin() const {
    return data_;
  };
  T* end() {
    return data_ + size_;
  };
  T const* end() const {
    return data_ + size_;
  };
  T const* cend() const {
    return data_ + size_;
  };

 private:
  T* data_;
  uint32_t cap_ = 0;
  uint32_t size_ = 0;
};

// This implementation is optimized assuming its total size is 16
static_assert(sizeof(Vec32<void*>) == 16);

#endif // VEC32_HPP
