// *************************************************************************
//  This file is the DArr64 container implementation by Steaphan Greene
//
//  DArr64 is a minimal vector-like container implementation, designed to
//  reduce both the memory and CPU footprint of small vectors of 64-bit
//  objects, such as pointers.
//
//  DArr64 is essentially just a special case of small-vector-optimized
//  vector, where only capacity C is stored within the class, and for all
//  higher capacities, the data is stored in a dynamic array, like vector.
//
//  The length and capacity are limited to a 32-bit unsigned value, which
//  reduces the total size of each DArr64<1> to 16 bytes, making it always
//  smaller than vector, except perhaps in some cases with untuned capacity.
//
//  The capacity of DArr64 is never reduced, except on object destruction,
//  or when being moved from.
//
//  The cap_ member of DArr64 is the capacity, or is zero if the capacity
//  is the default (which is C).  This is so most of the comparisons can be
//  with zero (which is often best optimized), and so this class can be
//  trivially default initialized by setting all memory to zeros.
//
//  DArr64 is not a complete container implementation, as only the elements
//  which have actually been needed so far have been implemented.
//
//  Copyright 2022 Steaphan Greene <steaphan@gmail.com>
//
//  DArr64 is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  DArr64 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with DArr64 (see the file named "COPYING");
//  If not, see <http://www.gnu.org/licenses/>.
//
// *************************************************************************

#ifndef DARR64_HPP
#define DARR64_HPP

#include <cassert>
#include <cstdint>
#include <utility>

constexpr auto smear_bits_right(uint32_t val) -> uint32_t {
  uint32_t ret = (val >> 1U);
  if (ret != 0) {
    ret |= smear_bits_right(ret);
  }
  return ret | val;
}
constexpr auto next_pow_2(uint32_t val) -> uint32_t {
  return smear_bits_right(val) + 1;
};
static_assert(next_pow_2(0U) == 1U);
static_assert(next_pow_2(1U) == 2U);
static_assert(next_pow_2(2U) == 4U);
static_assert(next_pow_2(3U) == 4U);
static_assert(next_pow_2(11U) == 16U);
static_assert(next_pow_2(16U) == 32U);
static_assert(next_pow_2(257U) == 512U);
static_assert(next_pow_2(1024U) == 2048U);
static_assert(next_pow_2(0x40000000U) == 0x80000000U);
static_assert(next_pow_2(0x7FFFFFFFU) == 0x80000000U);

template <int C, typename T>
class alignas(next_pow_2(C * 8)) DArr64 {
 public:
  using iterator = T*;
  using const_iterator = T const*;

  DArr64() = default;
  DArr64(const DArr64& in) : cap_(in.cap_), size_(in.size_) {
    if (cap_ == 0) {
      data_ = in.data_;
    } else {
      data_.arr = new T[cap_];
      for (uint32_t idx = 0; idx < size_; ++idx) {
        data_.arr[idx] = in.data_.arr[idx];
      }
    }
  };
  DArr64(DArr64&& in) noexcept(noexcept(data_ = std::move(in.data_)))
      : data_(std::move(in.data_)), cap_(in.cap_), size_(in.size_) {
    in.cap_ = 0;
    in.size_ = 0;
  };

  void operator=(const DArr64& in) {
    if (this == &in) {
      return; // Self-assign
    }
    if (cap_ != 0) {
      delete[] data_.arr;
    }
    cap_ = in.cap_;
    size_ = in.size_;
    if (cap_ == 0) {
      data_ = in.data_;
    } else {
      data_.arr = new T[cap_];
      for (uint32_t idx = 0; idx < size_; ++idx) {
        data_.arr[idx] = in.data_.arr[idx];
      }
    }
  };
  void operator=(DArr64&& in) noexcept(noexcept(data_ = std::move(in.data_))) {
    if (this == &in) {
      return; // Self-assign
    }
    if (cap_ != 0) {
      delete[] data_.arr;
    }
    cap_ = in.cap_;
    size_ = in.size_;
    data_ = std::move(in.data_);
    in.cap_ = 0;
    in.size_ = 0;
  };

  auto operator==(const DArr64& in) const -> bool {
    if (size_ != in.size_) {
      return false;
    }
    if (cap_ == 0 && in.cap_ == 0) {
      for (uint32_t idx = 0; idx < size_; ++idx) {
        if (data_.val[idx] != in.data_.val[idx]) {
          return false;
        }
      }
      return true;
    } else if (cap_ == 0) {
      for (uint32_t idx = 0; idx < size_; ++idx) {
        if (data_.val[idx] != in.data_.arr[idx]) {
          return false;
        }
      }
      return true;
    } else if (in.cap_ == 0) {
      for (uint32_t idx = 0; idx < size_; ++idx) {
        if (data_.arr[idx] != in.data_.val[idx]) {
          return false;
        }
      }
      return true;
    } else {
      for (uint32_t idx = 0; idx < size_; ++idx) {
        if (data_.arr[idx] != in.data_.arr[idx]) {
          return false;
        }
      }
      return true;
    }
  };

  ~DArr64() {
    if (cap_ != 0) {
      delete[] data_.arr;
    }
  };

  auto operator[](int idx) -> T& {
    if (cap_ == 0) {
      return data_.val[idx];
    } else {
      return data_.arr[idx];
    }
  };

  [[nodiscard]] auto empty() const -> bool {
    return size_ == 0;
  };
  [[nodiscard]] auto size() const -> std::size_t {
    return size_;
  };
  [[nodiscard]] auto capacity() const -> std::size_t {
    if (cap_ == 0) {
      return C;
    } else {
      return cap_;
    }
  };

  void reserve(std::size_t cap) {
    if (cap > C && cap > cap_) {
      assert(cap <= 0x80000000UL);
      if (cap_ == 0 && size_ == 0) {
        cap_ = cap;
        data_.arr = new T[cap_];
      } else if (cap_ == 0) {
        auto temp = data_;
        cap_ = cap;
        data_.arr = new T[cap_];
        for (uint32_t idx = 0; idx < size_; ++idx) {
          data_.arr[idx] = temp.val[idx];
        }
      } else {
        T* temp = data_.arr;
        cap_ = cap;
        data_.arr = new T[cap_];
        for (uint32_t idx = 0; idx < size_; ++idx) {
          data_.arr[idx] = temp[idx];
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
    if (cap_ == 0 && size_ < C) {
      data_.val[size_] = in;
    } else if (cap_ == 0) {
      auto temp = data_;
      cap_ = next_pow_2(C);
      data_.arr = new T[cap_];
      for (uint32_t idx = 0; idx < size_; ++idx) {
        data_.arr[idx] = temp.val[idx];
      }
      data_.arr[size_] = in;
    } else if (size_ < cap_) {
      data_.arr[size_] = in;
    } else {
      assert(cap_ <= 0x40000000U);
      T* temp = data_.arr;
      cap_ *= 2;
      data_.arr = new T[cap_];
      for (uint32_t idx = 0; idx < size_; ++idx) {
        data_.arr[idx] = temp[idx];
      }
      delete[] temp;
      data_.arr[size_] = in;
    }
    ++size_;
  };

  [[nodiscard]] auto front() -> T& {
    if (cap_ == 0) {
      return data_.val[0];
    } else {
      return data_.arr[0];
    }
  };
  [[nodiscard]] auto front() const -> T const& {
    if (cap_ == 0) {
      return data_.val[0];
    } else {
      return data_.arr[0];
    }
  };
  [[nodiscard]] auto back() -> T& {
    if (cap_ == 0) {
      return data_.val[size_ - 1];
    } else {
      return data_.arr[size_ - 1];
    }
  };
  [[nodiscard]] auto back() const -> T const& {
    if (cap_ == 0) {
      return data_.val[size_ - 1];
    } else {
      return data_.arr[size_ - 1];
    }
  };

  [[nodiscard]] auto begin() -> T* {
    if (cap_ == 0) {
      return data_.val;
    } else {
      return data_.arr;
    }
  };
  [[nodiscard]] auto begin() const -> T const* {
    if (cap_ == 0) {
      return data_.val;
    } else {
      return data_.arr;
    }
  };
  [[nodiscard]] auto cbegin() const -> T const* {
    if (cap_ == 0) {
      return data_.val;
    } else {
      return data_.arr;
    }
  };
  [[nodiscard]] auto end() -> T* {
    if (cap_ == 0) {
      return data_.val + size_;
    } else {
      return data_.arr + size_;
    }
  };
  [[nodiscard]] auto end() const -> T const* {
    if (cap_ == 0) {
      return data_.val + size_;
    } else {
      return data_.arr + size_;
    }
  };
  [[nodiscard]] auto cend() const -> T const* {
    if (cap_ == 0) {
      return data_.val + size_;
    } else {
      return data_.arr + size_;
    }
  };

 private:
  union {
    T val[C];
    T* arr;
  } data_;
  uint32_t cap_ = 0; // 0 means default capacity (of C)
  uint32_t size_ = 0;

  // This implementation is optimized for storing only 64-bit types
  static_assert(sizeof(T) == 8);

  // Sanity check - this containter is for SMALL vectors
  static_assert(next_pow_2(C) <= 0x80000000U);
};

// This implementation is optimized assuming its total size is 16
static_assert(sizeof(DArr64<1, void*>) == 16);

#endif // DARR64_HPP
