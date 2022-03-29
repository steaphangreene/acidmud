// *************************************************************************
//  This file is the DArr32 container implementation by Steaphan Greene
//
//  DArr32 is a minimal vector-like container implementation, designed to
//  reduce both the memory and CPU footprint of small vectors of 32-bit
//  objects, such as pointers.
//
//  DArr32 is essentially just a special case of small-vector-optimized
//  vector, where only capacity C is stored within the class, and for all
//  higher capacities, the data is stored in a dynamic array, like vector.
//
//  The length and capacity are limited to a 32-bit unsigned value, which
//  reduces the total size of each DArr32<1> to 16 bytes, making it always
//  smaller than vector, except perhaps in some cases with untuned capacity.
//
//  The capacity of DArr32 is never reduced, except on object destruction,
//  or when being moved from.
//
//  The cap_ member of DArr32 is the capacity, or is zero if the capacity
//  is the default (which is C).  This is so most of the comparisons can be
//  with zero (which is often best optimized), and so this class can be
//  trivially default initialized by setting all memory to zeros.
//
//  DArr32 is not a complete container implementation, as only the elements
//  which have actually been needed so far have been implemented.
//
//  Copyright 2022 Steaphan Greene <steaphan@gmail.com>
//
//  DArr32 is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  DArr32 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with DArr32 (see the file named "COPYING");
//  If not, see <http://www.gnu.org/licenses/>.
//
// *************************************************************************

#ifndef DARR32_HPP
#define DARR32_HPP

#include "darr64.hpp"

template <typename T, std::size_t C = 3>
class alignas(next_pow_2(C * 4)) DArr32 {
 public:
  using value_type = T;
  using iterator = T*;
  using const_iterator = T const*;

  DArr32() = default;
  DArr32(const DArr32& in) {
    if (in.data_.sta.size <= C) {
      data_.sta.size = in.data_.sta.size;
      for (uint32_t idx = 0; idx < data_.sta.size_; ++idx) {
        std::construct_at(data_.sta.val + idx, in.data_.sta.val[idx]);
      }
    } else {
      data_.dyn.cap = in.data_.dyn.cap;
      data_.dyn.size = in.data_.dyn.size;

      data_.arr = std::allocator<T>().allocate(data_.dyn.cap);
      for (uint32_t idx = 0; idx < data_.dyn.size; ++idx) {
        std::construct_at(data_.dyn.arr + idx, in.data_.dyn.arr[idx]);
      }
    }
  };
  DArr32(DArr32&& in) noexcept(noexcept(data_ = std::move(in.data_))) {
    if (in.data_.sta.size <= C) {
      data_.sta.size = in.data_.sta.size;
      for (uint32_t idx = 0; idx < data_.sta.size_; ++idx) {
        std::construct_at(data_.sta.val + idx, std::move(in.data_.sta.val[idx]));
        std::destroy_at(in.data_.sta.val + idx);
      }
    } else {
      data_.dyn.cap = in.data_.dyn.cap;
      data_.dyn.size = in.data_.dyn.size;
      data_.dyn.arr = in.data_.dyn.arr;
    }
    in.data_.sta.size = 0;
  };

  void operator=(const DArr32& in) {
    if (this == &in) {
      return; // Self-assign
    }
    destroy_self();
    if (in.data_.sta.size <= C) {
      data_.sta.size = in.data_.sta.size;
      for (uint32_t idx = 0; idx < data_.sta.size_; ++idx) {
        std::construct_at(data_.sta.val + idx, in.data_.sta.val[idx]);
      }
    } else {
      data_.dyn.cap = in.data_.dyn.cap;
      data_.dyn.size = in.data_.dyn.size;

      data_.arr = std::allocator<T>().allocate(data_.dyn.cap);
      for (uint32_t idx = 0; idx < data_.dyn.size; ++idx) {
        std::construct_at(data_.dyn.arr + idx, in.data_.dyn.arr[idx]);
      }
    }
  };
  void operator=(DArr32&& in) noexcept(noexcept(data_ = std::move(in.data_))) {
    if (this == &in) {
      return; // Self-assign
    }
    destroy_self();
    if (in.data_.sta.size <= C) {
      data_.sta.size = in.data_.sta.size;
      for (uint32_t idx = 0; idx < data_.sta.size_; ++idx) {
        std::construct_at(data_.sta.val + idx, std::move(in.data_.sta.val[idx]));
        std::destroy_at(in.data_.sta.val + idx);
      }
    } else {
      data_.dyn.cap = in.data_.dyn.cap;
      data_.dyn.size = in.data_.dyn.size;
      data_.dyn.arr = in.data_.dyn.arr;
    }
    in.data_.sta.size = 0;
  };

  auto operator==(const DArr32& in) const -> bool {
    if (size() != in.size()) {
      return false;
    }
    if (data_.sta.size <= C && in.data_.sta.size <= C) {
      for (uint32_t idx = 0; idx < size(); ++idx) {
        if (data_.sta.val[idx] != in.data_.sta.val[idx]) {
          return false;
        }
      }
      return true;
    } else if (data_.sta.size <= C) {
      for (uint32_t idx = 0; idx < size(); ++idx) {
        if (data_.sta.val[idx] != in.data_.dyn.arr[idx]) {
          return false;
        }
      }
      return true;
    } else if (in.data_.sta.size <= C) {
      for (uint32_t idx = 0; idx < size(); ++idx) {
        if (data_.dyn.arr[idx] != in.data_.sta.val[idx]) {
          return false;
        }
      }
      return true;
    } else {
      for (uint32_t idx = 0; idx < size(); ++idx) {
        if (data_.dyn.arr[idx] != in.data_.dyn.arr[idx]) {
          return false;
        }
      }
      return true;
    }
  };

  ~DArr32() {
    destroy_self();
  };

  auto operator[](int idx) -> T& {
    if (data_.sta.size <= C) {
      return data_.sta.val[idx];
    } else {
      return data_.dyn.arr[idx];
    }
  };

  [[nodiscard]] auto empty() const -> bool {
    return size() == 0;
  };
  [[nodiscard]] auto size() const -> std::size_t {
    return (data_.sta.size <= C) ? data_.sta.size : data_.dyn.size;
  };
  [[nodiscard]] auto capacity() const -> std::size_t {
    if (data_.sta.size <= C) {
      return C;
    } else {
      return data_.dyn.cap;
    }
  };

  void reserve(std::size_t cap) {
    if (cap > C && cap > data_.dyn.cap) {
      assert(cap <= 0x80000000UL);
      if (data_.sta.size == 0) {
        data_.dyn.size = 0;
        data_.dyn.cap = cap;
        data_.dyn.arr = std::allocator<T>().allocate(data_.dyn.cap);
      } else if (data_.sta.size <= C) {
        auto temp = std::allocator<T>().allocate(cap);
        for (uint32_t idx = 0; idx < data_.sta.size; ++idx) {
          std::construct_at(temp + idx, std::move(data_.sta.val[idx]));
          std::destroy_at(data_.sta.val + idx);
        }
        data_.dyn.size = data_.sta.size;
        data_.dyn.cap = cap;
        data_.dyn.arr = temp;
      } else {
        T* temp = data_.dyn.arr;
        auto old_cap = data_.dyn.cap;
        data_.dyn.cap = cap;
        data_.dyn.arr = std::allocator<T>().allocate(cap);
        for (uint32_t idx = 0; idx < data_.dyn.size; ++idx) {
          std::construct_at(data_.dyn.arr + idx, std::move(temp[idx]));
          std::destroy_at(temp + idx);
        }
        std::allocator<T>().deallocate(temp, old_cap);
      }
    }
  };

  void clear() {
    destroy_self();
    data_.sta.size = 0;
  };

  void erase(auto b) {
    if (data_.sta.size <= C) {
      --data_.sta.size;
    } else {
      --data_.dyn.size;
    }
    for (auto itr = b; itr != end(); ++itr) {
      *itr = std::move(*(itr + 1));
    }
    std::destroy_at(end());
  };

  void erase(auto b, auto e) {
    assert(e == end()); // Only support truncation of end.
    if (data_.sta.size <= C) {
      data_.sta.size = b - begin(); // Assume deleting all the rest.
    } else {
      data_.dyn.size = b - begin(); // Assume deleting all the rest.
    }
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
    if (data_.sta.size <= C) {
      --data_.sta.size;
      std::destroy_at(data_.sta.val + data_.sta.size);
    } else {
      --data_.dyn.size;
      std::destroy_at(data_.dyn.arr + data_.dyn.size);
    }
  };

  template <typename... Args>
  void emplace_back(Args&&... args) {
    if (data_.sta.size < C) {
      std::construct_at(data_.sta.val + data_.sta.size, std::forward<Args>(args)...);
      ++data_.sta.size;
    } else if (data_.sta.size == C) {
      auto temp = std::allocator<T>().allocate(next_pow_2(C));
      std::construct_at(temp + C, std::forward<Args>(args)...);
      for (uint32_t idx = 0; idx < C; ++idx) {
        std::construct_at(temp + idx, std::move(data_.sta.val[idx]));
        std::destroy_at(data_.sta.val + idx);
      }
      data_.dyn.arr = temp;
      data_.dyn.cap = next_pow_2(C);
      data_.dyn.size = C + 1;
    } else if (data_.dyn.size < data_.dyn.cap) {
      std::construct_at(data_.dyn.arr + data_.dyn.size, std::forward<Args>(args)...);
      ++data_.dyn.size;
    } else {
      assert(data_.dyn.cap <= 0x40000000U);
      T* temp = data_.dyn.arr;
      auto old_cap = data_.dyn.cap;
      data_.dyn.cap *= 2;
      data_.dyn.arr = std::allocator<T>().allocate(data_.dyn.cap);
      std::construct_at(data_.dyn.arr + data_.dyn.size, std::forward<Args>(args)...);
      for (uint32_t idx = 0; idx < data_.dyn.size; ++idx) {
        std::construct_at(data_.dyn.arr + idx, std::move(temp[idx]));
        std::destroy_at(temp + idx);
      }
      std::allocator<T>().deallocate(temp, old_cap);
      ++data_.dyn.size;
    }
  };

  void push_back(const T& in) {
    emplace_back(in);
  };

  [[nodiscard]] auto front() -> T& {
    if (data_.sta.size <= C) {
      return data_.sta.val[0];
    } else {
      return data_.dyn.arr[0];
    }
  };
  [[nodiscard]] auto front() const -> T const& {
    if (data_.sta.size <= C) {
      return data_.sta.val[0];
    } else {
      return data_.dyn.arr[0];
    }
  };
  [[nodiscard]] auto back() -> T& {
    if (data_.sta.size <= C) {
      return data_.sta.val[data_.sta.size - 1];
    } else {
      return data_.dyn.arr[data_.dyn.size - 1];
    }
  };
  [[nodiscard]] auto back() const -> T const& {
    if (data_.sta.size <= C) {
      return data_.sta.val[data_.sta.size - 1];
    } else {
      return data_.dyn.arr[data_.dyn.size - 1];
    }
  };

  [[nodiscard]] auto begin() -> T* {
    if (data_.sta.size <= C) {
      return data_.sta.val;
    } else {
      return data_.dyn.arr;
    }
  };
  [[nodiscard]] auto begin() const -> T const* {
    if (data_.sta.size <= C) {
      return data_.sta.val;
    } else {
      return data_.dyn.arr;
    }
  };
  [[nodiscard]] auto cbegin() const -> T const* {
    if (data_.sta.size <= C) {
      return data_.sta.val;
    } else {
      return data_.dyn.arr;
    }
  };
  [[nodiscard]] auto end() -> T* {
    if (data_.sta.size <= C) {
      return data_.sta.val + data_.sta.size;
    } else {
      return data_.dyn.arr + data_.dyn.size;
    }
  };
  [[nodiscard]] auto end() const -> T const* {
    if (data_.sta.size <= C) {
      return data_.sta.val + data_.sta.size;
    } else {
      return data_.dyn.arr + data_.dyn.size;
    }
  };
  [[nodiscard]] auto cend() const -> T const* {
    if (data_.sta.size <= C) {
      return data_.sta.val + data_.sta.size;
    } else {
      return data_.dyn.arr + data_.dyn.size;
    }
  };

 private:
  void destroy_self() {
    if (data_.sta.size > C) {
      for (std::size_t idx = 0; idx < data_.dyn.size; ++idx) {
        std::destroy_at(data_.dyn.arr + idx);
      }
      std::allocator<T>().deallocate(data_.dyn.arr, data_.dyn.cap);
    } else {
      for (std::size_t idx = 0; idx < data_.sta.size; ++idx) {
        std::destroy_at(data_.sta.val + idx);
      }
    }
  }

  union internal {
    internal() {
      sta.size = 0;
    }
    ~internal() {}
    struct {
      uint32_t size; // size > C: this struct is inactive
      T val[C];
    } sta;
    struct {
      uint32_t cap; // cap <= C: capacity is C, and this struct is inactive
      uint32_t size;
      T* arr;
    } dyn;
  } data_;

  // This implementation is optimized for storing only 32-bit types
  static_assert(sizeof(T) == 4);

  // Sanity check - this containter's minimum base capacity is 3
  static_assert(C >= 3);

  // Sanity check - this containter is for SMALL vectors
  static_assert(next_pow_2(C) <= 0x80000000U);
};

// This implementation is optimized assuming its total size is 16
static_assert(sizeof(DArr32<uint32_t>) == 16);

#endif // DARR32_HPP
