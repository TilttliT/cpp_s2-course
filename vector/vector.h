#pragma once
#include <cstddef>

template <typename T>
struct vector {
  using iterator = T*;
  using const_iterator = T const*;

  vector();                               // O(1) nothrow
  vector(vector const&);                  // O(N) strong
  vector& operator=(vector const& other); // O(N) strong

  ~vector(); // O(N) nothrow

  T& operator[](size_t i);             // O(1) nothrow
  T const& operator[](size_t i) const; // O(1) nothrow

  T* data();             // O(1) nothrow
  T const* data() const; // O(1) nothrow
  size_t size() const;   // O(1) nothrow

  T& front();             // O(1) nothrow
  T const& front() const; // O(1) nothrow

  T& back();                // O(1) nothrow
  T const& back() const;    // O(1) nothrow
  void push_back(T const&); // O(1)* strong
  void pop_back();          // O(1) nothrow

  bool empty() const; // O(1) nothrow

  size_t capacity() const; // O(1) nothrow
  void reserve(size_t);    // O(N) strong
  void shrink_to_fit();    // O(N) strong

  void clear(); // O(N) nothrow

  void swap(vector&); // O(1) nothrow

  iterator begin(); // O(1) nothrow
  iterator end();   // O(1) nothrow

  const_iterator begin() const; // O(1) nothrow
  const_iterator end() const;   // O(1) nothrow

  iterator insert(const_iterator pos, T const&); // O(N) strong

  iterator erase(const_iterator pos); // O(N) nothrow(swap)

  iterator erase(const_iterator first,
                 const_iterator last); // O(N) nothrow(swap)

private:
  void ensure_capacity(size_t);

private:
  T* data_;
  size_t size_;
  size_t capacity_;
};

template <typename T>
void remove_elements(T* data, const size_t size) {
  for (size_t i = 0; i < size; ++i) {
    data[i].~T();
  }
}

template <typename T>
void copy_elements(T const* from, T* to, const size_t size, T const* element = nullptr) {
  size_t ind = 0;
  try {
    for (; ind < size; ++ind) {
      new (to + ind) T(from[ind]);
    }
    if (element != nullptr)
      new (to + size) T(*element);
  } catch (...) {
    remove_elements(to, ind);
    throw;
  }
}

template <typename T>
void vector<T>::ensure_capacity(size_t new_capacity) {
  T* new_data = nullptr;
  if (new_capacity != 0)
    new_data = static_cast<T*>(operator new(sizeof(T) * new_capacity));
  try {
    copy_elements(data_, new_data, size_);
  } catch (...) {
    operator delete(new_data);
    throw;
  }
  remove_elements(data_, size_);
  operator delete(data_);
  data_ = new_data;
  capacity_ = new_capacity;
}

template <typename T>
vector<T>::vector() : data_(nullptr), size_(0), capacity_(0) {}

template <typename T>
vector<T>::vector(vector<T> const& other) : vector() {
  if (other.size_ == 0)
    return;
  T* new_data = static_cast<T*>(operator new(sizeof(T) * other.size_));
  try {
    copy_elements(other.data_, new_data, other.size_);
  } catch (...) {
    operator delete(new_data);
    throw;
  }
  data_ = new_data;
  capacity_ = size_ = other.size_;
}

template <typename T>
vector<T>& vector<T>::operator=(vector<T> const& other) {
  if (this != &other) {
    vector<T> new_vector(other);
    swap(new_vector);
  }
  return *this;
}

template <typename T>
vector<T>::~vector() {
  clear();
  operator delete(data_);
}

template <typename T>
T& vector<T>::operator[](size_t i) {
  return data_[i];
}

template <typename T>
T const& vector<T>::operator[](size_t i) const {
  return data_[i];
}

template <typename T>
T* vector<T>::data() {
  return data_;
}

template <typename T>
T const* vector<T>::data() const {
  return data_;
}

template <typename T>
size_t vector<T>::size() const {
  return size_;
}

template <typename T>
T& vector<T>::front() {
  return *data_;
}

template <typename T>
T const& vector<T>::front() const {
  return *data_;
}

template <typename T>
T& vector<T>::back() {
  return data_[size_ - 1];
}

template <typename T>
T const& vector<T>::back() const {
  return data_[size_ - 1];
}

template <typename T>
void vector<T>::push_back(T const& element) {
  if (size_ != capacity_) {
    new (data_ + size_) T(element);
  } else {
    size_t new_capacity = 2 * size_ + 1;
    T* new_data = static_cast<T*>(operator new(sizeof(T) * (new_capacity)));
    try {
      copy_elements(data_, new_data, size_, &element);
    } catch (...) {
      operator delete(new_data);
      throw;
    }
    remove_elements(data_, size_);
    operator delete(data_);
    data_ = new_data;
    capacity_ = new_capacity;
  }
  ++size_;
}

template <typename T>
void vector<T>::pop_back() {
  data_[--size_].~T();
}

template <typename T>
bool vector<T>::empty() const {
  return size_ == 0;
}

template <typename T>
size_t vector<T>::capacity() const {
  return capacity_;
}

template <typename T>
void vector<T>::reserve(size_t new_capacity) {
  if (capacity_ < new_capacity) {
    ensure_capacity(new_capacity);
  }
}

template <typename T>
void vector<T>::shrink_to_fit() {
  if (capacity_ > size_) {
    ensure_capacity(size_);
  }
}

template <typename T>
void vector<T>::clear() {
  remove_elements(data_, size_);
  size_ = 0;
}

template <typename T>
void vector<T>::swap(vector<T>& other) {
  std::swap(data_, other.data_);
  std::swap(size_, other.size_);
  std::swap(capacity_, other.capacity_);
}

template <typename T>
typename vector<T>::iterator vector<T>::begin() {
  return data_;
}

template <typename T>
typename vector<T>::iterator vector<T>::end() {
  return begin() + size_;
}

template <typename T>
typename vector<T>::const_iterator vector<T>::begin() const {
  return data_;
}

template <typename T>
typename vector<T>::const_iterator vector<T>::end() const {
  return begin() + size_;
}

template <typename T>
typename vector<T>::iterator vector<T>::insert(vector<T>::const_iterator pos,
                                               T const& element) {
  ptrdiff_t pos_ind = pos - begin();
  push_back(element);
  for (ptrdiff_t i = end() - begin() - 1; i > pos_ind; --i) {
    std::swap(data_[i], data_[i - 1]);
  }
  return begin() + pos_ind;
}

template <typename T>
typename vector<T>::iterator vector<T>::erase(vector<T>::const_iterator pos) {
  return erase(pos, pos + 1);
}

template <typename T>
typename vector<T>::iterator vector<T>::erase(vector<T>::const_iterator first,
                                              vector<T>::const_iterator last) {
  ptrdiff_t first_ind = first - begin();
  ptrdiff_t last_ind = last - begin();
  for (ptrdiff_t i = 0; i < end() - last; ++i) {
    std::swap(data_[first_ind + i], data_[last_ind + i]);
  }
  for (ptrdiff_t i = 0; i < last_ind - first_ind; ++i) {
    pop_back();
  }
  return begin() + first_ind;
}
