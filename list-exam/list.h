#pragma once
#include <cassert>
#include <iterator>

template <typename T>
struct list {
private:
  struct node_base {
    node_base *next, *prev;

    node_base() : next(this), prev(this) {}
  };

  struct node : node_base {
    T data;

    node(T const& val) : data(val) {};
  };

  template <typename V>
  struct Iterator {
    friend struct list<T>;

    using difference_type = std::ptrdiff_t;
    using value_type = V;
    using pointer = V*;
    using reference = V&;
    using iterator_category = std::bidirectional_iterator_tag;

    Iterator(std::nullptr_t) = delete;

    template <typename U, typename std::enable_if<
                              std::is_same<V, const U>::value>::type* = nullptr>
    Iterator(Iterator<U> const& other) : ptr(other.ptr) {}

    Iterator& operator++() {
      ptr = ptr->next;
      return (*this);
    }

    Iterator operator++(int) {
      Iterator res(*this);
      ++(*this);
      return res;
    }

    Iterator& operator--() {
      ptr = ptr->prev;
      return (*this);
    }

    Iterator operator--(int) {
      Iterator res(*this);
      --(*this);
      return res;
    }

    friend bool operator==(Iterator const& a, Iterator const& b) {
      return a.ptr == b.ptr;
    }

    friend bool operator!=(Iterator const& a, Iterator const& b) {
      return a.ptr != b.ptr;
    }

    reference operator*() const {
      return static_cast<node*>(ptr)->data;
    }

    pointer operator->() const {
      return &static_cast<node*>(ptr)->data;
    }

  private:
    Iterator(node_base* o_ptr) : ptr(o_ptr) {}

    node_base* ptr;
  };

    void cut_node(node_base* a, node_base* b) {
      a->next = b;
      b->prev = a;
    }

  node_base root;

public:
  // bidirectional iterator
  using iterator = Iterator<T>;
  // bidirectional iterator
  using const_iterator = Iterator<const T>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  // O(1)
  list() noexcept = default;

  // O(n), strong
  list(list const& other) : list() {
    for (const auto& x: other) {
      push_back(x);
    }
  }

  // O(n), strong
  list& operator=(list const& other) {
    if (this != &other) {
      list<T> temp(other);
      swap((*this), temp);
    }
    return *this;
  }

  // O(n)
  ~list() {
    clear();
  }

  // O(1)
  bool empty() const noexcept {
    return root.next == &root;
  }

  // O(1)
  T& front() noexcept {
    return *(begin());
  }

  // O(1)
  T const& front() const noexcept {
    return *(begin());
  }

  // O(1), strong
  void push_front(T const& val) {
    insert(begin(), val);
  }

  // O(1)
  void pop_front() noexcept {
    erase(begin());
  }

  // O(1)
  T& back() noexcept {
    return *(--end());
  }

  // O(1)
  T const& back() const noexcept {
    return *(--end());
  }

  // O(1), strong
  void push_back(T const& val) {
    insert(end(), val);
  }

  // O(1)
  void pop_back() noexcept {
    erase(--end());
  }

  // O(1)
  iterator begin() noexcept {
    return iterator(root.next);
  }

  // O(1)
  const_iterator begin() const noexcept {
    return const_iterator(root.next);
  }

  // O(1)
  iterator end() noexcept {
    return iterator(&root);
  }

  // O(1)
  const_iterator end() const noexcept {
    return const_iterator(const_cast<node_base*>(&root));
  }

  // O(1)
  reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }

  // O(1)
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  // O(1)
  reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }

  // O(1)
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  // O(n)
  void clear() noexcept {
    while (!empty()) {
      pop_back();
    }
  }

  // O(1), strong
  iterator insert(const_iterator pos, T const& val) {
    auto n = new node(val);
    n->next = pos.ptr;
    n->prev = pos.ptr->prev;
    pos.ptr->prev->next = n;
    pos.ptr->prev = n;
    return iterator(n);
  }

  // O(1)
  iterator erase(const_iterator pos) noexcept {
    return erase(pos, std::next(pos, 1));
  }

  // O(n)
  iterator erase(const_iterator first, const_iterator last) noexcept {
    node_base* prev = first.ptr->prev;
    node_base* next = last.ptr;
    for (auto it = first.ptr; it != last.ptr;) {
      auto temp = it->next;
      delete static_cast<node*>(it);
      it = temp;
    }
    prev->next = next;
    next->prev = prev;
    return iterator(next);
  }

  // O(1)
    void splice(const_iterator pos, list& other, const_iterator first,
                const_iterator last) noexcept {
      if (first != last) {
        auto left = first.ptr;
        auto right = last.ptr->prev;
        cut_node(left->prev, last.ptr);
        cut_node(pos.ptr->prev, left);
        cut_node(right, pos.ptr);
      }
    }

  friend void swap(list& a, list& b) noexcept {
    if (!a.empty() && !b.empty()) {
      a.root.prev->next = &b.root;
      a.root.next->prev = &b.root;
      b.root.prev->next = &a.root;
      b.root.next->prev = &a.root;
      std::swap(a.root, b.root);
    } else if (!a.empty() && b.empty()) {
      a.root.prev->next = &b.root;
      a.root.next->prev = &b.root;
      b.root.prev = &a.root;
      b.root.next = &a.root;
      std::swap(a.root, b.root);
    } else if (a.empty() && !b.empty()) {
      swap(b, a);
    }
  }
};
