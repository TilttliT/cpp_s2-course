#pragma once

template <typename T, size_t SMALL_SIZE>
struct socow_vector {
  using iterator = T*;
  using const_iterator = T const*;

  socow_vector() : small_(true), size_(0) {}

  socow_vector(socow_vector const& other)
      : small_(other.small_), size_(other.size_) {
    if (small_) {
      copy_elements(other.small_data, small_data, size_);
    } else {
      big_data = new_link(other.big_data);
    }
  }

  socow_vector& operator=(socow_vector const& other) {
    if (&other != this) {
      socow_vector temp(other);
      swap(temp);
    }
    return *this;
  }

  ~socow_vector() {
    if (small_ || single()) {
      remove_elements(uncut_begin(), size_);
    }
    if (!small_) {
      delete_big_data();
    }
  }

  T& operator[](size_t i) {
    return *(data() + i);
  }

  T const& operator[](size_t i) const {
    return *(data() + i);
  }

  T* data() {
    cut_link();
    return uncut_begin();
  }

  T const* data() const {
    return (small_ ? small_data : big_data->data_);
  }

  size_t size() const {
    return size_;
  }

  T& front() {
    return *(data());
  }

  T const& front() const {
    return *(data());
  }

  T& back() {
    return *(data() + size_ - 1);
  }

  T const& back() const {
    return *(data() + size_ - 1);
  }

  void push_back(T const& element) {
    if (size_ == capacity()) {
      if (small_) {
        small_to_big(2 * SMALL_SIZE, &element);
      } else {
        ensure_capacity(2 * capacity(), &element);
      }
    } else {
      cut_link();
      new (uncut_end()) T(element);
    }
    ++size_;
  }

  void pop_back() {
    back().~T();
    --size_;
  }

  bool empty() const {
    return size_ == 0;
  }

  size_t capacity() const {
    return (small_ ? SMALL_SIZE : big_data->capacity_);
  }

  void reserve(size_t new_capacity) {
    if ((!small_ && !single()) || new_capacity >= capacity()) {
      if (small_) {
        small_to_big(new_capacity);
      } else {
        ensure_capacity(std::max(new_capacity, capacity()));
      }
    }
  }

  void shrink_to_fit() {
    if (!small_) {
      if (size_ <= SMALL_SIZE) {
        big_to_small();
      } else if (size_ != capacity()) {
        ensure_capacity(size_);
      }
    }
  }

  void clear() {
    if (small_ || single()) {
      remove_elements(uncut_begin(), size_);
    } else {
      delete_big_data();
      big_data = new_big_data(capacity());
    }
    size_ = 0;
  }

  void swap(socow_vector& other) {
    if (small_ && other.small_) {
      swap_small_small(*this, other);
    } else if (!small_ && other.small_) {
      swap_big_small(*this, other);
    } else if (small_ && !other.small_) {
      swap_big_small(other, *this);
    } else {
      std::swap(big_data, other.big_data);
    }
    std::swap(small_, other.small_);
    std::swap(size_, other.size_);
  }

  iterator begin() {
    return data();
  }

  iterator end() {
    return data() + size_;
  }

  const_iterator begin() const {
    return data();
  }

  const_iterator end() const {
    return data() + size_;
  }

  iterator insert(const_iterator pos, T const& element) {
    ptrdiff_t pos_ind = pos - uncut_begin();
    push_back(element);
    for (ptrdiff_t i = uncut_end() - uncut_begin() - 1; i > pos_ind; --i) {
      std::swap((*this)[i], (*this)[i - 1]);
    }
    return uncut_begin() + pos_ind;
  }

  iterator erase(const_iterator pos) {
    return erase(pos, pos + 1);
  }

  iterator erase(const_iterator first, const_iterator last) {
    ptrdiff_t ind = first - uncut_begin();
    ptrdiff_t dist = last - first;
    cut_link();
    for (iterator it = uncut_begin() + ind; it < uncut_end() - dist; ++it) {
      std::swap(*(it), *(it + dist));
    }
    for (ptrdiff_t i = 0; i < dist; ++i) {
      pop_back();
    }
    return uncut_begin() + ind;
  }

private:
  struct fam {
    size_t capacity_;
    size_t links_;
    T data_[0];
  };

  bool small_;
  size_t size_;
  union {
    fam* big_data;
    T small_data[SMALL_SIZE];
  };

private:
  bool single() {
    return big_data->links_ == 1;
  }

  fam* new_big_data(size_t new_capacity) {
    size_t bytes = sizeof(fam) + new_capacity * sizeof(T);
    void* temp = operator new(bytes);
    fam* ptr = new (temp) fam;
    ptr->capacity_ = new_capacity;
    ptr->links_ = 1;
    return ptr;
  }

  void delete_big_data() {
    if (single()) {
      operator delete(big_data);
    } else {
      --big_data->links_;
    }
  }

  fam* new_link(fam* other) {
    other->links_++;
    return other;
  }

  void copy_elements(T const* from, T* to, const size_t size,
                     T const* element = nullptr) {
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

  void remove_elements(T* data, const size_t size) {
    for (size_t i = 0; i < size; ++i) {
      data[i].~T();
    }
  }

  void ensure_capacity(size_t new_capacity, T const* element = nullptr) {
    fam* ptr = new_big_data(new_capacity);
    try {
      copy_elements(uncut_begin(), ptr->data_, size_, element);
    } catch (...) {
      operator delete(ptr);
      throw;
    }
    if (single()) {
      remove_elements(big_data->data_, size_);
    }
    delete_big_data();
    big_data = ptr;
  }

  void small_to_big(size_t new_capacity, T const* element = nullptr) {
    fam* ptr = new_big_data(new_capacity);
    try {
      copy_elements(small_data, ptr->data_, size_, element);
    } catch (...) {
      operator delete(ptr);
      throw;
    }
    remove_elements(small_data, size_);
    big_data = ptr;
    small_ = false;
  }

  void big_to_small() {
    fam* ptr = big_data;
    try {
      copy_elements(ptr->data_, small_data, size_);
    } catch (...) {
      big_data = ptr;
      throw;
    }
    if (ptr->links_ == 1) {
      remove_elements(ptr->data_, size_);
      operator delete(ptr);
    } else {
      --ptr->links_;
    }
    small_ = true;
  }

  void cut_link() {
    if (!small_ && !single()) {
      ensure_capacity(capacity());
    }
  }

  void swap_big_small(socow_vector& big_vec, socow_vector& small_vec) {
    fam* ptr = big_vec.big_data;
    try {
      copy_elements(small_vec.small_data, big_vec.small_data, small_vec.size_);
    } catch (...) {
      big_vec.big_data = ptr;
      throw;
    }
    remove_elements(small_vec.small_data, small_vec.size_);
    small_vec.big_data = ptr;
  }

  void swap_small_small(socow_vector& first, socow_vector& second) {
    if (first.size_ < second.size_) {
      swap_small_small(second, first);
    } else {
      size_t general_size = std::min(first.size_, second.size_);
      for (size_t i = 0; i < general_size; ++i) {
        std::swap(first.small_data[i], second.small_data[i]);
      }
      swap_tail(first.small_data + general_size,
                second.small_data + general_size, first.size_ - general_size);
    }
  }

  void swap_tail(T* from, T* to, size_t size) {
    copy_elements(from, to, size);
    remove_elements(from, size);
  }

  iterator uncut_begin() {
    return (small_ ? small_data : big_data->data_);
  }

  iterator uncut_end() {
    return uncut_begin() + size_;
  }
};
