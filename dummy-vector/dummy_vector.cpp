#include "dummy_vector.h"
#include "element.h"

using T = element<size_t>;

void copy_elements(T const* from, T* to, const size_t size) {
  for (size_t i = 0; i < size; ++i) {
    new (to + i) T(from[i]);
  }
}

void remove_elements(T* data, const size_t size) {
  for (size_t i = 0; i < size; ++i) {
    data[i].~T();
  }
}

dummy_vector::dummy_vector() : data_(nullptr), size_(0), capacity_(0) {}

dummy_vector::dummy_vector(dummy_vector const& other) :
      data_(nullptr), size_(0), capacity_(0)
{
  reserve(other.size_);
  copy_elements(other.data_, data_, other.size_);
  size_ = other.size_;
}

dummy_vector& dummy_vector::operator=(dummy_vector const& other) {
  dummy_vector new_vector(other);
  swap(new_vector);
  return *this;
}

dummy_vector::~dummy_vector() {
  clear();
  operator delete(data_);
}

T& dummy_vector::operator[](size_t i) {
  return data_[i];
}

T const& dummy_vector::operator[](size_t i) const {
  return data_[i];
}

T* dummy_vector::data() {
  return data_;
}

T const* dummy_vector::data() const {
  return data_;
}

size_t dummy_vector::size() const {
  return size_;
}

T& dummy_vector::front() {
  return *data_;
}

T const& dummy_vector::front() const {
  return *data_;
}

T& dummy_vector::back() {
  return data_[size_ - 1];
}

T const& dummy_vector::back() const {
  return data_[size_ - 1];
}

void dummy_vector::push_back(T const& element) {
  if (size_ != capacity_) {
    new (data_ + size_) T(element);
  } else {
    size_t new_capacity = 2 * size_ + 1;
    T* new_data = static_cast<T*>(operator new(sizeof(T) * (new_capacity)));
    copy_elements(data_, new_data, size_);
    new (new_data + size_) T(element);
    remove_elements(data_, size_);
    operator delete(data_);
    data_ = new_data;
    capacity_ = new_capacity;
  }
  ++size_;
}

void dummy_vector::pop_back() {
  data_[--size_].~T();
}

bool dummy_vector::empty() const {
  return size_ == 0;
}

size_t dummy_vector::capacity() const {
  return capacity_;
}

void dummy_vector::reserve(size_t new_capacity) {
  if (capacity_ < new_capacity) {
    T* new_data = static_cast<T*>(operator new(sizeof(T) * new_capacity));
    copy_elements(data_, new_data, size_);
    remove_elements(data_, size_);
    operator delete(data_);
    data_ = new_data;
    capacity_ = new_capacity;
  }
}

void dummy_vector::shrink_to_fit() {
  if (capacity_ > size_) {
    T* new_data = nullptr;
    if (size_ != 0) {
      new_data = static_cast<T*>(operator new(sizeof(T) * size_));
    }
    copy_elements(data_, new_data, size_);
    remove_elements(data_, size_);
    operator delete(data_);
    data_ = new_data;
    capacity_ = size_;
  }
}

void dummy_vector::clear() {
  remove_elements(data_, size_);
  size_ = 0;
}

void dummy_vector::swap(dummy_vector& other) {
  std::swap(data_, other.data_);
  std::swap(size_, other.size_);
  std::swap(capacity_, other.capacity_);
}

dummy_vector::iterator dummy_vector::begin() {
  return data_;
}

dummy_vector::iterator dummy_vector::end() {
  return begin() + size_;
}

dummy_vector::const_iterator dummy_vector::begin() const {
  return data_;
}

dummy_vector::const_iterator dummy_vector::end() const {
  return begin() + size_;
}

dummy_vector::iterator dummy_vector::insert(dummy_vector::const_iterator pos,
                                            T const& element)
{
  ptrdiff_t pos_ind = pos - begin();
  push_back(element);
  for (ptrdiff_t i = end() - begin() - 1; i > pos_ind; --i) {
    std::swap(data_[i], data_[i - 1]);
  }
  return begin() + pos_ind;
}

dummy_vector::iterator dummy_vector::erase(dummy_vector::const_iterator pos) {
  return erase(pos, pos + 1);
}

dummy_vector::iterator dummy_vector::erase(dummy_vector::const_iterator first,
                                           dummy_vector::const_iterator last)
{
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