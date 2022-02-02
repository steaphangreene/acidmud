// *************************************************************************
//  This file is the MinVec container implementation by Steaphan Greene
//
//  MinVec is a minimal vector-like container implementation, designed to
//  reduce both the memory and CPU footprint of small vectors of 64-bit
//  objects, such as pointers.
//
//  MinVec is essentially just a special case of small-vector-optimized
//  vector, where only capacity 1 is stored within the class, and for all
//  higher capacities, the data is stored in a dynamic array, like vector.
//
//  The length and capacity are limited to a 32-bit unsigned value, which
//  reduces the total size of each MinVec to 16 bytes, making it always
//  smaller than vector, except perhaps in some cases with untuned capacity.
//
//  The capacity of MinVec is never reduced, except on object destruction,
//  or when being moved from.
//
//  The cap_ member of MinVec is the capacity, or is zero if the capacity
//  is the default (which is 1).  This is so most of the comparisons can be
//  with zero (which is often best optimized), and so this class can be
//  trivially default initialized by setting all memory to zeros.
//
//  MinVec is not a complete container implementation, as only the elements
//  which have actually been needed so far have been implemented.
//
//  Copyright 2022 Steaphan Greene <steaphan@gmail.com>
//
//  MinVec is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  MinVec is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with MinVec (see the file named "COPYING");
//  If not, see <http://www.gnu.org/licenses/>.
//
// *************************************************************************

#include <cassert>

#ifndef MINVEC_HPP
#define MINVEC_HPP

template <typename T>
class MinVec {
 public:
  using iterator = T*;
  using const_iterator = T const*;

  MinVec() = default;
  MinVec(const MinVec& in) {
    cap_ = in.cap_;
    size_ = in.size_;
    if (cap_ == 0) {
      data_ = in.data_;
    } else {
      data_.arr = new T[cap_];
      for (int idx = 0; idx < size_; ++idx) {
        data_.arr[idx] = in.data_.arr[idx];
      }
    }
  };
  MinVec(MinVec&& in) {
    cap_ = in.cap_;
    size_ = in.size_;
    data_ = in.data_;
    in.cap_ = 0;
    in.size_ = 0;
  };

  void operator=(const MinVec& in) {
    if (cap_ != 0) {
      delete[] data_.arr;
    }
    cap_ = in.cap_;
    size_ = in.size_;
    if (cap_ == 0) {
      data_ = in.data_;
    } else {
      data_.arr = new T[cap_];
      for (int idx = 0; idx < size_; ++idx) {
        data_.arr[idx] = in.data_.arr[idx];
      }
    }
  };
  void operator=(MinVec&& in) {
    if (cap_ != 0) {
      delete[] data_.arr;
    }
    cap_ = in.cap_;
    size_ = in.size_;
    data_ = in.data_;
    in.cap_ = 0;
    in.size_ = 0;
  };

  bool operator==(const MinVec& in) const {
    if (cap_ == 0) {
      for (auto idx = 0; idx < size_; ++idx) {
        if (data_.val[idx] != in.data_.val[idx]) {
          return false;
        }
      }
      return true;
    } else {
      for (auto idx = 0; idx < size_; ++idx) {
        if (data_.arr[idx] != in.data_.arr[idx]) {
          return false;
        }
      }
      return true;
    }
  };

  ~MinVec() {
    if (cap_ != 0) {
      delete[] data_.arr;
    }
  };

  T& operator[](int idx) {
    if (cap_ == 0) {
      return data_.val[idx];
    } else {
      return data_.arr[idx];
    }
  };

  bool empty() const {
    return size_ == 0;
  };
  size_t size() const {
    return size_;
  };
  size_t capacity() const {
    if (cap_ == 0) {
      return 1;
    } else {
      return cap_;
    }
  };

  void reserve(size_t cap) {
    if (cap > 1 && cap > cap_) {
      if (cap_ == 0 && size_ == 0) {
        cap_ = cap;
        data_.arr = new T[cap_];
      } else if (cap_ == 0) {
        auto temp = data_;
        cap_ = cap;
        data_.arr = new T[cap_];
        for (int idx = 0; idx < size_; ++idx) {
          data_.arr[idx] = temp.val[idx];
        }
      } else {
        T* temp = data_.arr;
        cap_ = cap;
        data_.arr = new T[cap_];
        for (int idx = 0; idx < size_; ++idx) {
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

  void push_back(T in) {
    if (cap_ == 0 && size_ == 0) {
      data_.val[size_] = in;
    } else if (cap_ == 0) {
      auto temp = data_;
      cap_ = 2;
      data_.arr = new T[cap_];
      for (int idx = 0; idx < size_; ++idx) {
        data_.arr[idx] = temp.val[idx];
      }
      data_.arr[size_] = in;
    } else if (size_ < cap_) {
      data_.arr[size_] = in;
    } else {
      T* temp = data_.arr;
      cap_ *= 2;
      data_.arr = new T[cap_];
      for (int idx = 0; idx < size_; ++idx) {
        data_.arr[idx] = temp[idx];
      }
      delete[] temp;
      data_.arr[size_] = in;
    }
    ++size_;
  };

  T& front() {
    if (cap_ == 0) {
      return data_.val[0];
    } else {
      return data_.arr[0];
    }
  };
  T const& front() const {
    if (cap_ == 0) {
      return data_.val[0];
    } else {
      return data_.arr[0];
    }
  };
  T& back() {
    if (cap_ == 0) {
      return data_.val[size_ - 1];
    } else {
      return data_.arr[size_ - 1];
    }
  };
  T const& back() const {
    if (cap_ == 0) {
      return data_.val[size_ - 1];
    } else {
      return data_.arr[size_ - 1];
    }
  };

  T* begin() {
    if (cap_ == 0) {
      return data_.val;
    } else {
      return data_.arr;
    }
  };
  T const* begin() const {
    if (cap_ == 0) {
      return data_.val;
    } else {
      return data_.arr;
    }
  };
  T const* cbegin() const {
    if (cap_ == 0) {
      return data_.val;
    } else {
      return data_.arr;
    }
  };
  T* end() {
    if (cap_ == 0) {
      return data_.val + size_;
    } else {
      return data_.arr + size_;
    }
  };
  T const* end() const {
    if (cap_ == 0) {
      return data_.val + size_;
    } else {
      return data_.arr + size_;
    }
  };
  T const* cend() const {
    if (cap_ == 0) {
      return data_.val + size_;
    } else {
      return data_.arr + size_;
    }
  };

 private:
  union {
    T val[1];
    T* arr;
  } data_;
  uint32_t cap_ = 0; // 0 means default capacity (of 1)
  uint32_t size_ = 0;

  // This implementation is optimized for storing only 64-bit types
  static_assert(sizeof(T) == 8);
};

// This implementation is optimized assuming its total size is 16
static_assert(sizeof(MinVec<void*>) == 16);

#endif // MINVEC_HPP
