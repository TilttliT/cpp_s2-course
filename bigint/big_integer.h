#pragma once

#include <climits>
#include <functional>
#include <iosfwd>
#include <ostream>
#include <string>
#include <vector>

struct big_integer {
  big_integer() = default;
  big_integer(big_integer const& other) = default;
  big_integer(int a);
  big_integer(unsigned a);
  big_integer(long a);
  big_integer(unsigned long a);
  big_integer(long long a);
  big_integer(unsigned long long a);
  explicit big_integer(std::string const& str);
  ~big_integer() = default;

  big_integer& operator=(big_integer const& other) = default;

  big_integer& operator+=(big_integer const& rhs);
  big_integer& operator-=(big_integer const& rhs);
  big_integer& operator*=(big_integer const& rhs);
  big_integer& operator/=(big_integer const& rhs);
  big_integer& operator%=(big_integer const& rhs);

  big_integer& operator&=(big_integer const& rhs);
  big_integer& operator|=(big_integer const& rhs);
  big_integer& operator^=(big_integer const& rhs);

  big_integer& operator<<=(int rhs);
  big_integer& operator>>=(int rhs);

  big_integer operator+() const;
  big_integer operator-() const;
  big_integer operator~() const;

  big_integer& operator++();
  big_integer operator++(int);

  big_integer& operator--();
  big_integer operator--(int);

  friend bool operator==(big_integer const& a, big_integer const& b);
  friend bool operator!=(big_integer const& a, big_integer const& b);
  friend bool operator<(big_integer const& a, big_integer const& b);
  friend bool operator>(big_integer const& a, big_integer const& b);
  friend bool operator<=(big_integer const& a, big_integer const& b);
  friend bool operator>=(big_integer const& a, big_integer const& b);

  friend std::string to_string(big_integer const& a);

  big_integer& add_small(uint32_t rhs);
  big_integer& mul_small(uint32_t rhs);
  big_integer& negate();
  uint32_t div_small(uint32_t rhs);
  bool is_zero() const;

private:
  // - = true, + = false
  bool sign_ {};
  std::vector<uint32_t> data_;

private:
  void shrink_to_fit();
  uint32_t trial(const big_integer& d, size_t k, size_t m) const;
  bool smaller(const big_integer& dq, size_t k, size_t m) const;
  void difference(const big_integer& dq, size_t k, size_t m);
  void int_constructor(uint64_t a);
  uint32_t get_digit(size_t ind) const;
  void expand(size_t size);
  template<typename F>
  big_integer& bitwise(big_integer const& rhs, F func);
  big_integer& div_long(const big_integer& rhs, bool div);
};

big_integer operator+(big_integer a, big_integer const& b);
big_integer operator-(big_integer a, big_integer const& b);
big_integer operator*(big_integer a, big_integer const& b);
big_integer operator/(big_integer a, big_integer const& b);
big_integer operator%(big_integer a, big_integer const& b);

big_integer operator&(big_integer a, big_integer const& b);
big_integer operator|(big_integer a, big_integer const& b);
big_integer operator^(big_integer a, big_integer const& b);

big_integer operator<<(big_integer a, int b);
big_integer operator>>(big_integer a, int b);

bool operator==(big_integer const& a, big_integer const& b);
bool operator!=(big_integer const& a, big_integer const& b);
bool operator<(big_integer const& a, big_integer const& b);
bool operator>(big_integer const& a, big_integer const& b);
bool operator<=(big_integer const& a, big_integer const& b);
bool operator>=(big_integer const& a, big_integer const& b);

std::string to_string(big_integer const& a);
std::ostream& operator<<(std::ostream& s, big_integer const& a);
