/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *\
 * Copyright (C) 2020-2023 Oleg Butakov                                       *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person obtaining a    *
 * copy of this software and associated documentation files (the "Software"), *
 * to deal in the Software without restriction, including without limitation  *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
 * and/or sell copies of the Software, and to permit persons to whom the      *
 * Software is furnished to do so, subject to the following conditions:       *
 *                                                                            *
 * The above copyright notice and this permission notice shall be included in *
 * all copies or substantial portions of the Software.                        *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    *
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        *
 * DEALINGS IN THE SOFTWARE.                                                  *
\* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#pragma once

#include <array>
#include <concepts>
#include <type_traits>

#include "tit/core/assert.hpp"
#include "tit/core/math.hpp"
#include "tit/core/types.hpp"
#include "tit/core/vec.hpp"

namespace tit {

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** Matrix symmetry type. */
enum class MatSymm {

  none, /**< Non symmetric matrix. */
  symm, /**< Symmetric matrix. */

}; // enum class MatSymm

/** Common matrix symmetry type. */
template<class... RestMatSymms>
  requires (std::same_as<MatSymm, std::remove_cvref_t<RestMatSymms>> && ...)
constexpr auto common_symm(MatSymm symm, RestMatSymms... rest_symms) noexcept
    -> MatSymm {
  return ((symm == rest_symms) && ...) ? symm : MatSymm::none;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************\
 ** Algebraic square matrix.
\******************************************************************************/
template<class Num, size_t Dim, MatSymm Symm = MatSymm::none>
class Mat final {
public:

  /** Symmetry type. */
  static constexpr auto symm = Symm;

  /** Number of rows. */
  static constexpr auto num_rows = Dim;

  /** Number of columns. */
  static constexpr auto num_cols = Dim;

  /** Number of elements. */
  static constexpr auto num_elements = num_rows * num_cols;

  /** Matrix row type. */
  using Row = Vec<Num, Dim>;

private:

  std::array<Row, Dim> _rows;

public:

  /** Construct a matrix with rows. */
  template<class... Rows>
    requires (sizeof...(Rows) == Dim) &&
             (std::constructible_from<Row, Rows> && ...)
  constexpr explicit Mat(Rows... rows) noexcept : _rows{rows...} {}

  /** Construct a scalar matrix. */
  constexpr Mat(Num q = Num{}) noexcept {
    *this = q;
  }
  /** Assign scalar matrix. */
  constexpr auto operator=(Num q) noexcept -> Mat& {
    for (size_t i = 0; i < num_rows; ++i) {
      for (size_t j = 0; j < num_cols; ++j) {
        (*this)[i, j] = (i == j) ? q : Num{};
      }
    }
    return *this;
  }

  /** Matrix row at index. */
  /** @{ */
  constexpr auto operator[](size_t i) noexcept -> Row& {
    TIT_ASSERT(i < num_rows, "Row index is out of range.");
    return _rows[i];
  }
  constexpr auto operator[](size_t i) const noexcept -> Row {
    TIT_ASSERT(i < num_rows, "Row index is out of range.");
    return _rows[i];
  }
  /** @} */

  /** Matrix element at index. */
  /** @{ */
  constexpr auto operator[](size_t i, size_t j) noexcept -> Num& {
    TIT_ASSERT(i < num_rows, "Row index is out of range.");
    TIT_ASSERT(j < num_cols, "Column index is out of range.");
    return _rows[i][j];
  }
  constexpr auto operator[](size_t i, size_t j) const noexcept -> Num {
    TIT_ASSERT(i < num_rows, "Row index is out of range.");
    TIT_ASSERT(j < num_cols, "Column index is out of range.");
    return _rows[i][j];
  }
  /** @} */

}; // class Mat

// TODO: We cannot yet construct matrices like `Mat{{1,0},{0,1}}`.
template<class Row, class... RestRows>
Mat(Row, RestRows...) -> Mat<vec_num_t<Row>, 1 + sizeof...(RestRows)>;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** Matrix input operator. */
template<class Stream, class Num, size_t Dim, MatSymm Symm>
constexpr auto operator>>(Stream& stream, Mat<Num, Dim, Symm>& a) -> Stream& {
  if constexpr (Symm == MatSymm::none) {
    for (size_t i = 0; i < a.num_rows; ++i) stream >> a[i];
  } else if constexpr (Symm == MatSymm::symm) {
    for (size_t i = 0; i < a.num_rows; ++i) {
      stream >> a[i, i];
      for (size_t j = i; j < a.num_cols; ++j) {
        stream >> a[i, j];
        a[i, j] = a[j, i];
      }
    }
  }
  return stream;
}

/** Vector output operator. */
template<class Stream, class Num, size_t Dim, MatSymm Symm>
constexpr auto operator<<(Stream& stream, Mat<Num, Dim, Symm> a) -> Stream& {
  if constexpr (Symm == MatSymm::none) {
    stream << a[0];
    for (size_t i = 1; i < a.num_rows; ++i) stream << " " << a[i];
  } else if constexpr (Symm == MatSymm::symm) {
    stream << a[0, 0];
    for (size_t j = 1; j < a.num_cols; ++j) {
      stream << " " << a[0, j];
    }
    for (size_t i = 1; i < a.num_rows; ++i) {
      for (size_t j = i; j < a.num_cols; ++j) {
        stream << " " << a[i, j];
      }
    }
  }
  return stream;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** Matrix unary plus. */
template<class Num, size_t Dim, MatSymm Symm>
constexpr auto operator+(const Mat<Num, Dim, Symm>& a) noexcept {
  return a;
}

/** Matrix addition. */
template<class NumA, class NumB, size_t Dim, MatSymm SymmA, MatSymm SymmB>
constexpr auto operator+(const Mat<NumA, Dim, SymmA>& a,
                         const Mat<NumB, Dim, SymmA>& b) noexcept {
  Mat<add_result_t<NumA, NumB>, Dim, common_symm(SymmA, SymmB)> r;
  for (size_t i = 0; i < r.num_rows; ++i) r[i] = a[i] + b[i];
  return r;
}

/** Matrix addition assignment. */
template<class NumA, class NumB, size_t Dim, MatSymm Symm>
constexpr auto& operator+=(Mat<NumA, Dim, Symm>& a,
                           const Mat<NumB, Dim, Symm>& b) noexcept {
  for (size_t i = 0; i < a.num_rows; ++i) a[i] += b[i];
  return a;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** Matrix negation. */
template<class Num, size_t Dim, MatSymm Symm>
constexpr auto operator-(const Mat<Num, Dim, Symm>& a) noexcept {
  Mat<negate_result_t<Num>, Dim, Symm> r;
  for (size_t i = 0; i < r.num_rows; ++i) r[i] = -a[i];
  return r;
}

/** Matrix subtraction. */
template<class NumA, class NumB, size_t Dim, MatSymm SymmA, MatSymm SymmB>
constexpr auto operator-(const Mat<NumA, Dim, SymmA>& a,
                         const Mat<NumB, Dim, SymmA>& b) noexcept {
  Mat<sub_result_t<NumA, NumB>, Dim, common_symm(SymmA, SymmB)> r;
  for (size_t i = 0; i < r.num_rows; ++i) r[i] = a[i] - b[i];
  return r;
}

/** Matrix subtraction assignment. */
template<class NumA, class NumB, size_t Dim, MatSymm Symm>
constexpr auto& operator-=(Mat<NumA, Dim, Symm>& a,
                           const Mat<NumB, Dim, Symm>& b) noexcept {
  for (size_t i = 0; i < a.num_rows; ++i) a[i] -= b[i];
  return a;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** Matrix-scalar multiplication. */
/** @{ */
template<class NumA, class NumB, size_t Dim, MatSymm Symm>
constexpr auto operator*(NumA a, const Mat<NumB, Dim, Symm>& b) noexcept {
  Mat<mul_result_t<NumA, NumB>, Dim, Symm> r;
  for (size_t i = 0; i < r.num_rows; ++i) r[i] = a * b[i];
  return r;
}
template<class NumA, class NumB, size_t Dim, MatSymm Symm>
constexpr auto operator*(const Mat<NumA, Dim, Symm>& a, NumB b) noexcept {
  Mat<mul_result_t<NumA, NumB>, Dim, Symm> r;
  for (size_t i = 0; i < r.num_rows; ++i) r[i] = a[i] * b;
  return r;
}
/** @} */

/** Matrix-scalar multiplication assignment. */
template<class NumA, class NumB, size_t Dim, MatSymm Symm>
constexpr auto& operator*=(Mat<NumA, Dim, Symm>& a, NumB b) noexcept {
  for (size_t i = 0; i < a.num_rows; ++i) a[i] *= b;
  return a;
}

/** Matrix-scalar division. */
template<class NumA, class NumB, size_t Dim, MatSymm Symm>
constexpr auto operator/(const Mat<NumA, Dim, Symm>& a, NumB b) noexcept {
  Mat<div_result_t<NumA, NumB>, Dim, Symm> r;
  for (size_t i = 0; i < r.num_rows; ++i) r[i] = a[i] / b;
  return r;
}

/** Matrix-scalar division assignment. */
template<class NumA, class NumB, size_t Dim, MatSymm Symm>
constexpr auto& operator/=(Mat<NumA, Dim, Symm>& a, NumB b) noexcept {
  for (size_t i = 0; i < a.num_rows; ++i) a[i] /= b;
  return a;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** Matrix-vector multiplication. */
template<class NumA, class NumB, size_t Dim, MatSymm Symm>
constexpr auto operator*(const Mat<NumA, Dim, Symm>& a,
                         Vec<NumB, Dim> b) noexcept {
  Vec<mul_result_t<NumA, NumB>, Dim> r;
  for (size_t i = 0; i < r.num_rows; ++i) r[i] = dot(a[i], b);
  return r;
}

/** Matrix-vector multiplication-assignment. */
template<class NumA, class NumB, size_t Dim, MatSymm Symm>
constexpr auto operator*=(Vec<NumA, Dim>& a,
                          const Mat<NumB, Dim, Symm>& b) noexcept {
  // Can not be implemented inplace.
  return a = b * a;
}

/** Vector outer product. */
template<class NumA, class NumB, size_t Dim>
constexpr auto outer(Vec<NumA, Dim> a, Vec<NumB, Dim> b) noexcept {
  Mat<mul_result_t<NumA, NumB>, Dim> r;
  for (size_t i = 0; i < r.num_rows; ++i) r[i] = a[i] * b;
  return r;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/******************************************************************************\
 ** Matrix inversion.
\******************************************************************************/
template<class Num, size_t Dim, MatSymm Symm = MatSymm::none>
class MatInv final {
private:

  Mat<Num, Dim> _l, _u;
  Num _det;

public:

  /** Construct matrix inversion. */
  constexpr explicit MatInv(const Mat<Num, Dim, Symm>& a) noexcept
      : _l(Num{1.0}), _u(Num{0.0}) {
    // Compute factors.
    for (size_t i = 0; i < a.num_rows; ++i) {
      for (size_t j = 0; j < i; ++j) {
        _l[i, j] = a[i, j];
        for (size_t k = 0; k < j; ++k) _l[i, j] -= _l[i, k] * _u[k, j];
        _l[i, j] /= _u[j, j];
      }
      for (size_t j = i; j < a.num_cols; ++j) {
        _u[i, j] = a[i, j];
        for (size_t k = 0; k < i; ++k) _u[i, j] -= _l[i, k] * _u[k, j];
      }
    }
    // Compute determinant.
    _det = _l[0, 0] * _u[0, 0];
    for (size_t i = 1; i < a.num_rows; ++i) _det *= _l[i, i] * _u[i, i];
  }

  /** Determinant of the original matrix. */
  constexpr auto det() const noexcept {
    return _det;
  }
  /** Is this matrix non-singular? */
  constexpr operator bool() const noexcept {
    return !is_zero(_det);
  }

  /** Multiply by an inverse matrix. */
  template<class Obj>
  constexpr auto operator()(Obj x) const noexcept -> Obj {
    TIT_ASSERT(*this, "Matrix must be non-singular.");
    // "Divide" by L.
    for (size_t i = 0; i < x.num_rows; ++i) {
      for (size_t j = 0; j < i; ++j) x[i] -= _l[i, j] * x[j];
      x[i] /= _l[i, i];
    }
    // "Divide" by U.
    for (ssize_t i = x.num_rows - 1; i >= 0; --i) {
      for (size_t j = i + 1; j < x.num_rows; ++j) x[i] -= _u[i, j] * x[j];
      x[i] /= _u[i, i];
    }
    return x;
  }
  /** Evaluate inverse matrix. */
  constexpr auto operator()() const noexcept -> Mat<Num, Dim, Symm> {
    TIT_ASSERT(*this, "Matrix must be non-singular.");
    return (*this)(Mat<Num, Dim, Symm>(1.0));
  }

}; // class MatInv

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

} // namespace tit
