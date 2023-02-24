#pragma once
#include "element.h"
#include <cstddef>

struct dummy_vector {
  using T = element<size_t>;
  using iterator = T*;
  using const_iterator = T const*;

  dummy_vector();                                     // O(1)
  dummy_vector(dummy_vector const&);                  // O(N)
  dummy_vector& operator=(dummy_vector const& other); // O(N)

  ~dummy_vector(); // O(N) nothrow

  T& operator[](size_t i);             // O(1)
  T const& operator[](size_t i) const; // O(1)

  T* data();             // O(1)
  T const* data() const; // O(1)
  size_t size() const;   // O(1)

  T& front();             // O(1)
  T const& front() const; // O(1)

  T& back();                // O(1)
  T const& back() const;    // O(1)
  void push_back(T const&); // O(1)*
  void pop_back();          // O(1)

  bool empty() const; // O(1)

  size_t capacity() const; // O(1)
  void reserve(size_t);    // O(N)
  void shrink_to_fit();    // O(N)

  void clear(); // O(N)

  void swap(dummy_vector&); // O(1)

  iterator begin(); // O(1)
  iterator end();   // O(1)

  const_iterator begin() const; // O(1)
  const_iterator end() const;   // O(1)

  iterator insert(const_iterator pos, T const&); // O(N)

  iterator erase(const_iterator pos); // O(N)

  iterator erase(const_iterator first,
                 const_iterator last); // O(N)

private:
  T* data_;
  size_t size_;
  size_t capacity_;
};
