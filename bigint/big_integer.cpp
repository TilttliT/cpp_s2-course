#include "big_integer.h"
#include <algorithm>
#include <cstddef>
#include <stdexcept>

const big_integer ONE = 1;
constexpr uint64_t BASE = (1ull << 32);
constexpr size_t MOD_D = 9;
constexpr uint32_t MOD = 1000000000;

void big_integer::int_constructor(uint64_t a) {
  do {
    uint32_t cur = a % BASE;
    data_.push_back(cur);
    a >>= 32;
  } while (a != 0);
  shrink_to_fit();
}

big_integer::big_integer(int a) : big_integer(static_cast<long long int>(a)) {}

big_integer::big_integer(unsigned int a)
    : big_integer(static_cast<unsigned long long int>(a)) {}

big_integer::big_integer(long a) : big_integer(static_cast<long long int>(a)) {}

big_integer::big_integer(unsigned long a)
    : big_integer(static_cast<unsigned long long int>(a)) {}

big_integer::big_integer(long long a) : sign_(false) {
  if (a < 0) {
    int_constructor(static_cast<uint64_t>(-(a + 1)) + 1);
    negate();
  } else {
    int_constructor(static_cast<uint64_t>(a));
  }
}

big_integer::big_integer(unsigned long long a) : sign_(false) {
  int_constructor(static_cast<uint64_t>(a));
}

big_integer::big_integer(std::string const& str) : big_integer() {
  if (str.empty() || str == "-") {
    throw std::invalid_argument("Find empty string");
  }
  for (size_t i = ((str[0] == '-') ? 1 : 0); i < str.size(); ++i) {
    uint32_t temp = 0;
    uint32_t mul = 1;
    size_t start_ind = std::min(i + MOD_D, str.size()) - 1;
    for (size_t j = start_ind; ; --j) {
      if (str[j] < '0' || str[j] > '9') {
        throw std::invalid_argument(std::string("Expected digit, find: ") + str[j]);
      }
      temp += (str[j] - '0') * mul;
      mul *= 10;
      if (j == i) {
        break;
      }
    }
    i = start_ind;
    this->mul_small(mul);
    this->add_small(temp);
  }
  if (str[0] == '-' && !is_zero()) {
    negate();
  }
}

big_integer& big_integer::negate() {
  sign_ = !sign_;
  for (size_t i = 0; i < data_.size(); ++i) {
    data_[i] = ~data_[i];
  }
  add_small(1);
  shrink_to_fit();
  return *this;
}

big_integer& big_integer::add_small(uint32_t rhs) {
  uint64_t temp = rhs;
  for (size_t i = 0; i < data_.size() && temp > 0; ++i) {
    temp += data_[i];
    data_[i] = temp % BASE;
    temp >>= 32;
  }
  if (temp > 0) {
    data_.push_back(temp);
  }
  return *this;
}

big_integer& big_integer::mul_small(uint32_t rhs) {
  uint32_t carry = 0;
  for (size_t i = 0; i < data_.size(); ++i) {
    uint64_t temp = data_[i];
    temp = temp * rhs + carry;
    data_[i] = temp % BASE;
    carry = (temp >> 32);
  }
  if (carry != 0) {
    data_.push_back(carry);
  }
  return *this;
}

uint32_t big_integer::div_small(uint32_t rhs) {
  if (data_.empty()) {
    return 0;
  }
  uint32_t carry = 0;
  for (size_t i = data_.size() - 1;; --i) {
    uint64_t temp = carry;
    temp <<= 32;
    temp += data_[i];
    data_[i] = temp / rhs;
    carry = temp % rhs;
    if (i == 0) {
      break;
    }
  }
  shrink_to_fit();
  return carry;
}

big_integer& big_integer::operator+=(big_integer const& rhs) {
  size_t m = std::max(data_.size(), rhs.data_.size()) + 1;
  expand(m);
  uint64_t carry = 0u;
  for (size_t i = 0; i < m; ++i) {
    carry += get_digit(i);
    carry += rhs.get_digit(i);
    data_[i] = carry % BASE;
    carry >>= 32;
  }
  sign_ = (data_.back() >> 31);
  shrink_to_fit();
  return *this;
}

big_integer& big_integer::operator-=(big_integer const& rhs) {
  size_t m = std::max(data_.size(), rhs.data_.size()) + 1;
  expand(m);
  int64_t carry = 0u;
  for (size_t i = 0; i < m; ++i) {
    carry += get_digit(i);
    carry -= rhs.get_digit(i);
    data_[i] = (carry + BASE) % BASE;
    carry >>= 32;
  }
  sign_ = (data_.back() >> 31);
  shrink_to_fit();
  return *this;
}

big_integer& big_integer::operator*=(big_integer const& rhs) {
  big_integer res;
  bool sign = sign_ ^ rhs.sign_;
  if (sign_) {
    negate();
  }
  res.expand(data_.size() + rhs.data_.size());
  for (size_t i = 0; i < data_.size(); ++i) {
    uint32_t carry = 0;
    for (size_t j = 0; (j < rhs.data_.size()) || (carry != 0); ++j) {
      uint64_t temp = data_[i];
      temp *= (rhs.sign_ ? ~rhs.get_digit(j) : rhs.get_digit(j));
      temp += carry;
      temp += res.get_digit(i + j);
      res.data_[i + j] = temp;
      carry = temp >> 32;
    }
  }
  if (rhs.sign_) {
    res += *this;
  }
  if (sign) {
    res.negate();
  }
  res.shrink_to_fit();
  std::swap(*this, res);
  return *this;
}

uint32_t big_integer::trial(const big_integer& d, size_t k, size_t m) const {
  size_t km = k + m;
  uint64_t r2 = get_digit(km);
  r2 = (r2 << 32) + get_digit(km - 1);
  r2 /= d.get_digit(m - 1);
  if (r2 < UINT32_MAX) {
    return static_cast<uint32_t>(r2);
  } else {
    return UINT32_MAX;
  }
}

bool big_integer::smaller(const big_integer& dq, size_t k, size_t m) const {
  bool res = false;
  for (size_t i = m;; --i) {
    if (get_digit(i + k) != dq.get_digit(i)) {
      res = get_digit(i + k) < dq.get_digit(i);
      break;
    }
    if (i == 0) {
      break;
    }
  }
  return res;
}

void big_integer::difference(const big_integer& dq, size_t k, size_t m) {
  uint32_t borrow = 0;
  for (size_t i = 0; i <= m; ++i) {
    uint64_t diff =
        static_cast<uint64_t>(get_digit(i + k)) - dq.get_digit(i) - borrow;
    borrow = (diff >> 32) > 0;
    data_[i + k] = diff;
  }
}

big_integer& big_integer::div_long(const big_integer& rhs, bool div) {
  bool sign = sign_ ^ rhs.sign_;
  big_integer b = rhs;
  if (b.sign_) {
    b.negate();
  }
  if (sign_) {
    negate();
  }

  size_t n = data_.size();
  size_t m = b.data_.size();
  big_integer res = 0;
  if (m == 1) {
    res = *this;
    *this = res.div_small(b.data_[0]);
  } else if (m <= n) {
    uint32_t f = BASE / (b.data_[m - 1] + 1);
    mul_small(f);
    b.mul_small(f);
    n = data_.size();
    data_.push_back(0);
    m = b.data_.size();
    big_integer dq;
    res.expand(n - m + 1);
    for (size_t k = n - m;; --k) {
      uint32_t qt = trial(b, k, m);
      dq.data_.assign(b.data_.begin(), b.data_.end());
      dq.mul_small(qt);
      if (smaller(dq, k, m)) {
        --qt;
        dq -= b;
      }
      res.data_[k] = qt;
      difference(dq, k, m);
      if (k == 0) {
        break;
      }
    }
    div_small(f);
  }

  if (div) {
    if (sign && !res.is_zero()) {
      res.negate();
    }
    *this = res;
  } else {
    if ((sign ^ rhs.sign_) && !is_zero()) {
      negate();
    }
  }
  shrink_to_fit();
  return *this;
}

big_integer& big_integer::operator/=(big_integer const& rhs) {
  return div_long(rhs, true);
}

big_integer& big_integer::operator%=(big_integer const& rhs) {
  return div_long(rhs, false);
}

template <typename F>
big_integer& big_integer::bitwise(big_integer const& rhs, F func) {
  size_t m = std::max(data_.size(), rhs.data_.size());
  expand(m);
  for (size_t i = 0; i < m; ++i) {
    data_[i] = func(data_[i], rhs.get_digit(i));
  }
  sign_ = func(sign_, rhs.sign_);
  shrink_to_fit();
  return *this;
}

big_integer& big_integer::operator&=(big_integer const& rhs) {
  return bitwise(rhs, [](uint32_t a, uint32_t b) { return a & b; });
}

big_integer& big_integer::operator|=(big_integer const& rhs) {
  return bitwise(rhs, [](uint32_t a, uint32_t b) { return a | b; });
}

big_integer& big_integer::operator^=(big_integer const& rhs) {
  return bitwise(rhs, [](uint32_t a, uint32_t b) { return a ^ b; });
}

big_integer& big_integer::operator<<=(int rhs) {
  *this *= (1u << (rhs % 32));
  data_.insert(data_.begin(), rhs / 32, 0);
  return *this;
}

big_integer& big_integer::operator>>=(int rhs) {
  *this /= (1u << (rhs % 32));
  if (sign_) {
    --(*this);
  }
  size_t m = std::min(data_.size(), static_cast<size_t>(rhs / 32));
  data_.erase(data_.begin(), data_.begin() + m);
  if (data_.empty()) {
    (*this) = big_integer();
  }
  return *this;
}

big_integer big_integer::operator+() const {
  return *this;
}

big_integer big_integer::operator-() const {
  if (is_zero()) {
    return *this;
  }
  big_integer res = *this;
  return res.negate();
}

big_integer big_integer::operator~() const {
  big_integer res;
  res.expand(data_.size());
  res.sign_ = !sign_;
  for (size_t i = 0; i < data_.size(); ++i) {
    res.data_[i] = ~data_[i];
  }
  res.shrink_to_fit();
  return res;
}

big_integer& big_integer::operator++() {
  add_small(1);
  return *this;
}

big_integer big_integer::operator++(int) {
  big_integer res(*this);
  ++(*this);
  return res;
}

big_integer& big_integer::operator--() {
  *this -= ONE;
  return *this;
}

big_integer big_integer::operator--(int) {
  big_integer res(*this);
  --(*this);
  return res;
}

big_integer operator+(big_integer a, big_integer const& b) {
  return a += b;
}

big_integer operator-(big_integer a, big_integer const& b) {
  return a -= b;
}

big_integer operator*(big_integer a, big_integer const& b) {
  return a *= b;
}

big_integer operator/(big_integer a, big_integer const& b) {
  return a /= b;
}

big_integer operator%(big_integer a, big_integer const& b) {
  return a %= b;
}

big_integer operator&(big_integer a, big_integer const& b) {
  return a &= b;
}

big_integer operator|(big_integer a, big_integer const& b) {
  return a |= b;
}

big_integer operator^(big_integer a, big_integer const& b) {
  return a ^= b;
}

big_integer operator<<(big_integer a, int b) {
  return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
  return a >>= b;
}

bool operator==(big_integer const& a, big_integer const& b) {
  return a.sign_ == b.sign_ && (a.data_ == b.data_);
}

bool operator!=(big_integer const& a, big_integer const& b) {
  return !(a == b);
}

bool operator<(big_integer const& a, big_integer const& b) {
  if (a.sign_ != b.sign_) {
    return a.sign_;
  } else if (a.data_.size() != b.data_.size()) {
    return a.data_.size() < b.data_.size();
  } else {
    return a.smaller(b, 0, a.data_.size());
  }
}

bool operator>(big_integer const& a, big_integer const& b) {
  return !(a == b || a < b);
}

bool operator<=(big_integer const& a, big_integer const& b) {
  return !(a > b);
}

bool operator>=(big_integer const& a, big_integer const& b) {
  return !(a < b);
}

std::string to_string(big_integer const& a) {
  if (a == 0) {
    return "0";
  }

  big_integer b = a;
  if (b.sign_) {
    b.negate();
  }
  std::string res;
  bool fl = false;
  do {
    uint32_t d = b.div_small(MOD);
    fl = (b == 0);
    for (size_t j = 0; j < MOD_D && (d != 0 || !fl); ++j) {
      res.push_back(static_cast<char>(d % 10 + '0'));
      d /= 10;
    }
  } while (!fl);
  if (a.sign_) {
    res.push_back('-');
  }
  std::reverse(res.begin(), res.end());
  return res;
}

void big_integer::shrink_to_fit() {
  uint32_t temp = (sign_ ? UINT32_MAX : 0u);
  while (!data_.empty() && data_.back() == temp) {
    data_.pop_back();
  }
}

std::ostream& operator<<(std::ostream& s, big_integer const& a) {
  return s << to_string(a);
}

uint32_t big_integer::get_digit(size_t ind) const {
  return ((ind < data_.size()) ? data_[ind] : (sign_ ? UINT32_MAX : 0u));
}

void big_integer::expand(size_t size) {
  for (size_t i = data_.size(); i < size; ++i) {
    data_.push_back(sign_ ? UINT32_MAX : 0u);
  }
}
bool big_integer::is_zero() const {
  return !sign_ && data_.empty();
}

