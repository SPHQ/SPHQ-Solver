/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *\
 * Part of the Tit Solver project, under the MIT License.
 * See /LICENSE.md for license information. SPDX-License-Identifier: MIT
\* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// IWYU pragma: private, include "tit/core/mat.hpp"
#pragma once

#include <expected>
#include <utility>

#include "tit/core/basic_types.hpp"
#include "tit/core/mat/mat.hpp"
#include "tit/core/mat/part.hpp"
#include "tit/core/mat/traits.hpp"
#include "tit/core/math.hpp"

namespace tit {

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/// Factorization error type.
enum class FactError : uint8_t {
  no_error,              ///< No error.
  near_singular,         ///< The matrix is nearly singular.
  not_positive_definite, ///< The matrix is not positive definite.
};

/// Factorization result.
template<class Fact>
using FactResult = std::expected<Fact, FactError>;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/// LU matrix factorization: `A = L * U`, where `L` is a lower-triangular matrix
/// with unit diagonal and `U` is an upper-triangular matrix.
template<class Num, size_t Dim>
class FactLU final {
public:

  /// Initialize a factorization.
  constexpr explicit FactLU(Mat<Num, Dim> LU) noexcept : LU_{std::move(LU)} {}

  /// `L` matrix factor.
  constexpr auto L() const -> Mat<Num, Dim> {
    return copy_part<MatPart::lower_unit>(LU_);
  }

  /// `U` matrix factor.
  constexpr auto U() const -> Mat<Num, Dim> {
    return copy_part<MatPart::upper_diag>(LU_);
  }

  /// Determinant of the matrix.
  constexpr auto det() const -> Num {
    return prod_diag(LU_);
  }

  /// Solve the matrix equation.
  template<mat_multiplier<Mat<Num, Dim>> Mult>
  constexpr auto solve(Mult x) const -> Mult {
    using enum MatPart;
    part_solve_inplace<lower_unit, upper_diag>(LU_, x);
    return x;
  }

  /// Compute the inverse matrix.
  constexpr auto inverse() const -> Mat<Num, Dim> {
    constexpr Mat<Num, Dim> I(Num{1.0});
    return solve(I);
  }

private:

  Mat<Num, Dim> LU_{};

}; // class FactLU

/// Compute LU matrix factorization: `A = L * U`, where `L` is a
/// lower-triangular matrix with unit diagonal and `U` is an upper-triangular
/// matrix.
template<class Num, size_t Dim>
constexpr auto lu(const Mat<Num, Dim>& A) -> FactResult<FactLU<Num, Dim>> {
  Mat<Num, Dim> LU;
  auto& L = LU;
  auto& U = LU;
  for (size_t i = 0; i < Dim; ++i) {
    for (size_t j = 0; j < i; ++j) {
      L[i, j] = A[i, j];
      for (size_t k = 0; k < j; ++k) {
        L[i, j] -= L[i, k] * U[k, j];
      }
      L[i, j] /= U[j, j];
    }
    for (size_t j = i; j < Dim; ++j) {
      U[i, j] = A[i, j];
      for (size_t k = 0; k < i; ++k) {
        U[i, j] -= L[i, k] * U[k, j];
      }
    }
    if (is_tiny(U[i, i])) {
      return std::unexpected{FactError::near_singular};
    }
  }
  return FactLU{std::move(LU)};
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/// Cholesky matrix factorization: `A = L * L^T`, where `L` is a
/// lower-triangular matrix.
template<class Num, size_t Dim>
class FactChol final {
public:

  /// Initialize a factorization.
  constexpr explicit FactChol(Mat<Num, Dim> L) noexcept : L_{std::move(L)} {}

  /// `L` matrix factor.
  constexpr auto L() const -> const Mat<Num, Dim>& {
    return L_;
  }

  /// Determinant of the matrix.
  constexpr auto det() const -> Num {
    return pow2(prod_diag(L_));
  }

  /// Solve the matrix equation.
  template<mat_multiplier<Mat<Num, Dim>> Mult>
  constexpr auto solve(Mult x) const -> Mult {
    using enum MatPart;
    part_solve_inplace<lower_diag, upper_diag | transposed>(L_, x);
    return x;
  }

  /// Compute the inverse matrix.
  constexpr auto inverse() const -> Mat<Num, Dim> {
    constexpr Mat<Num, Dim> I(Num{1.0});
    return solve(I);
  }

private:

  Mat<Num, Dim> L_{};

}; // class FactChol

/// Compute the Cholesky matrix factorization: `A = L * L^T`,
/// where `L` is a lower-triangular matrix.
///
/// Suitable for symmetric positive definite matrices.
///
/// Only the lower-triangular part of the input matrix is accessed.
template<class Num, size_t Dim>
constexpr auto chol(const Mat<Num, Dim>& A) -> FactResult<FactChol<Num, Dim>> {
  Mat<Num, Dim> L;
  for (size_t i = 0; i < Dim; ++i) {
    for (size_t j = 0; j < i; ++j) {
      L[i, j] = A[i, j];
      for (size_t k = 0; k < j; ++k) {
        L[i, j] -= L[i, k] * L[j, k];
      }
      L[i, j] /= L[j, j];
    }
    auto D = A[i, i];
    for (size_t k = 0; k < i; ++k) {
      D -= pow2(L[i, k]);
    }
    if (D < Num{0.0}) {
      return std::unexpected{FactError::not_positive_definite};
    }
    L[i, i] = sqrt(D);
    if (is_tiny(L[i, i])) {
      return std::unexpected{FactError::near_singular};
    }
  }
  return FactChol{std::move(L)};
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/// Modified Cholesky matrix factorization: `A = L * D * L^T`,
/// where `D` is a diagonal matrix and `L` is a lower-triangular matrix with
/// unit diagonal.
template<class Num, size_t Dim>
class FactLDL final {
public:

  /// Initialize a factorization.
  constexpr explicit FactLDL(Mat<Num, Dim> LD) noexcept : LD_{std::move(LD)} {}

  /// `L` matrix factor.
  constexpr auto L() const -> Mat<Num, Dim> {
    return copy_part<MatPart::lower_unit>(LD_);
  }

  /// `D` matrix factor.
  constexpr auto D() const -> Mat<Num, Dim> {
    return copy_part<MatPart::diag>(LD_);
  }

  /// Determinant of the matrix.
  constexpr auto det() const -> Num {
    return prod_diag(LD_);
  }

  /// Solve the matrix equation.
  template<mat_multiplier<Mat<Num, Dim>> Mult>
  constexpr auto solve(Mult x) const -> Mult {
    using enum MatPart;
    part_solve_inplace<lower_unit, diag, upper_unit | transposed>(LD_, x);
    return x;
  }

  /// Compute the inverse matrix.
  constexpr auto inverse() const -> Mat<Num, Dim> {
    constexpr Mat<Num, Dim> I(Num{1.0});
    return solve(I);
  }

private:

  Mat<Num, Dim> LD_{};

}; // class FactLDL

/// Compute the Modified Cholesky matrix factorization: `A = L * D * L^T`,
/// where `D` is a diagonal matrix and `L` is a lower-triangular matrix with
/// unit diagonal.
///
/// Suitable for symmetric matrices.
///
/// Only the lower-triangular part of the input matrix is accessed.
template<class Num, size_t Dim>
constexpr auto ldl(const Mat<Num, Dim>& A) -> FactResult<FactLDL<Num, Dim>> {
  Mat<Num, Dim> LD;
  auto& L = LD;
  auto& D = LD;
  for (size_t i = 0; i < Dim; ++i) {
    for (size_t j = 0; j < i; ++j) {
      L[i, j] = A[i, j];
      for (size_t k = 0; k < j; ++k) {
        L[i, j] -= L[i, k] * D[k, k] * L[j, k];
      }
      L[i, j] /= D[j, j];
    }
    D[i, i] = A[i, i];
    for (size_t k = 0; k < i; ++k) {
      D[i, i] -= L[i, k] * D[k, k] * L[i, k];
    }
    if (is_tiny(D[i, i])) {
      return std::unexpected{FactError::near_singular};
    }
  }
  return FactLDL{std::move(LD)};
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

} // namespace tit
