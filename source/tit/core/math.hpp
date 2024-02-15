/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *\
 * Part of the Tit Solver project, under the MIT License.
 * See /LICENSE.md for license information. SPDX-License-Identifier: MIT
\* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#pragma once

#include <cmath>
#include <concepts> // IWYU pragma: keep
#include <cstdlib>
#include <functional>
#include <initializer_list>
#include <limits>
#include <ranges>
#include <type_traits>
#include <utility>

#include "tit/core/basic_types.hpp"
#include "tit/core/checks.hpp"
#include "tit/core/utils.hpp"

namespace tit {

using std::abs;
using std::ceil;
using std::floor;
using std::round;
using std::sqrt;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/// Negation result type.
template<class Num>
using negate_result_t = decltype(-std::declval<Num>());

/// Addition result type.
template<class NumA, class NumB>
using add_result_t = decltype(std::declval<NumA>() + std::declval<NumB>());

/// Subtraction result type.
template<class NumA, class NumB>
using sub_result_t = decltype(std::declval<NumA>() - std::declval<NumB>());

/// Multiplication result type.
template<class NumA, class NumB>
using mul_result_t = decltype(std::declval<NumA>() * std::declval<NumB>());

/// Division result type.
template<class NumA, class NumB>
using div_result_t = decltype(std::declval<NumA>() / std::declval<NumB>());

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/// Sign of the value.
template<class Num>
constexpr auto sign(Num a) -> Num {
  return static_cast<Num>(int{Num{0} < a} - int{a < Num{0}});
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/// Raise to the second power with 1 multiplication.
template<class Num>
constexpr auto pow2(Num a) -> Num {
  return a * a;
}

/// Raise to the third power with 2 multiplications.
template<class Num>
constexpr auto pow3(Num a) -> Num {
  return a * a * a;
}

/// Raise to the fourth power with 2 multiplications.
template<class Num>
constexpr auto pow4(Num a) -> Num {
  auto const a_sqr = a * a;
  return a_sqr * a_sqr;
}

/// Raise to the fifth power with 3 multiplications.
template<class Num>
constexpr auto pow5(Num a) -> Num {
  auto const a_sqr = a * a;
  return a_sqr * a_sqr * a;
}

/// Raise to the sixth power with 3 multiplications.
template<class Num>
constexpr auto pow6(Num a) -> Num {
  auto const a_cubed = a * a * a;
  return a_cubed * a_cubed;
}

/// Raise to the seventh power with 4 multiplications.
template<class Num>
constexpr auto pow7(Num a) -> Num {
  auto const a_cubed = a * a * a;
  return a_cubed * a_cubed * a;
}

/// Raise to the eighth power with 3 multiplications.
template<class Num>
constexpr auto pow8(Num a) -> Num {
  auto const a_sqr = a * a;
  auto const a_pow4 = a_sqr * a_sqr;
  return a_pow4 * a_pow4;
}

/// Raise to the ninth power with 4 multiplications.
template<class Num>
constexpr auto pow9(Num a) -> Num {
  auto const a_cubed = a * a * a;
  return a_cubed * a_cubed * a_cubed;
}

/// Raise to the power.
template<std::floating_point Float>
constexpr auto pow(Float a, std::type_identity_t<Float> power) noexcept
    -> Float {
  return std::pow(a, static_cast<Float>(power));
}

/// Evaluate polynomial @f$ \sum c_k x^k @f$ value.
template<class Num, class Coeff>
constexpr auto horner(Num x, std::initializer_list<Coeff> ci) {
  mul_result_t<Num, Coeff> r{0};
  for (auto const c : ci | std::views::reverse) r = r * x + c;
  return r;
}

/// Number reciprocal.
template<class Num>
constexpr auto inverse(Num a) noexcept -> Num {
  return Num{1} / a;
}

/// Square root reciprocal.
template<std::floating_point Float>
constexpr auto rsqrt(Float a) noexcept -> Float {
  return Float{1.0} / std::sqrt(a);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/// Arithmetic average function.
template<class... Nums>
  requires (sizeof...(Nums) > 0)
constexpr auto avg(Nums... vals) {
  return (vals + ...) / sizeof...(Nums);
}

/// Arithmetic average function.
/// @note Presence of infinities of different signs will generate NaN.
template<class... Nums>
  requires (sizeof...(Nums) > 0)
constexpr auto havg(Nums... vals) noexcept {
  TIT_ASSERT(((vals >= 0) && ...),
             "Harmonic average requires all non-negative input.");
  return sizeof...(Nums) / (inverse(vals) + ...);
}

/// Geometric average functions.
/// @param values Input values. Must be positive.
template<class... Nums>
  requires (sizeof...(Nums) > 0)
constexpr auto gavg(Nums... values) noexcept {
  TIT_ASSERT(((values >= 0) && ...),
             "Geometric average requires all non-negative input.");
  return pow((values * ...), 1.0 / sizeof...(Nums));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/// Small number, treated as zero.
template<class Num>
inline constexpr auto small_number_v = Num{};

template<std::floating_point Real>
inline constexpr auto small_number_v<Real>{
#ifdef __clang__ // clang's `cbrt` is not constexpr yet.
    std::numeric_limits<Real>::epsilon()
#else
    std::cbrt(std::numeric_limits<Real>::epsilon())
#endif
};

/// Check if number is approximately zero.
template<class Num>
constexpr auto is_small(Num a) noexcept -> bool {
  return abs(a) <= small_number_v<Num>;
}

/// Check if two numbers are approximately equal.
template<class Num>
constexpr auto approx_equal_to(Num a, Num b) noexcept -> bool {
  return is_small(a - b);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/// Newton-Raphson solver status.
enum class NewtonRaphsonStatus {
  /// Success.
  success,
  /// Failure: number of iterations exceeded.
  fail_max_iter,
  /// Failure: computation reached a zero derivative.
  fail_zero_deriv,
}; // enum class NewtonRaphsonStatus

/// Find function rool using Newton-Raphson method.
/// @param x Current estimate of the root.
/// @param f Function whose root we are looking for. Should return a pair of
///          it's value and derivative. Should implicitly depend on @p x.
/// @param eps Tolerance.
/// @param max_iter Maximum amount of iterations.
/// @returns Status of the operation.
template<std::floating_point Num, class Func>
  requires std::invocable<Func&&>
constexpr auto newton_raphson(Num& x, Func&& f, Num eps = small_number_v<Num>,
                              size_t max_iter = 10) -> NewtonRaphsonStatus {
  TIT_ASSUME_UNIVERSAL(Func, f);
  using enum NewtonRaphsonStatus;
  for (size_t iter = 0; iter < max_iter; ++iter) {
    auto const [y, df_dx] = std::invoke(f /*, x*/);
    if (abs(y) <= eps) return success;
    if (is_small(df_dx)) return fail_zero_deriv;
    x -= y / df_dx;
  }
  return fail_max_iter;
}

/// Bisection solver status.
enum class BisectionStatus {
  /// Success.
  success,
  /// Failure: number of iterations exceeded.
  fail_max_iter,
  /// Failure: function has same signs on the ends of the search range.
  failure_sign,
}; // enum class BisectionStatus

/// Find function rool using Bisection method.
/// @param min_x Root's lower bound.
/// @param max_x Root's upper bound.
/// @param f Function whose root we are looking for.
/// @param eps Tolerance.
/// @param max_iter Maximum amount of iterations.
/// @returns Status of the operation.
template<class Num, class Func>
  requires std::invocable<Func&&, Num> &&
           std::same_as<std::invoke_result_t<Func&&, Num>, Num>
constexpr auto bisection(Num& min_x, Num& max_x, Func&& f,
                         Num eps = small_number_v<Num>, size_t max_iter = 10)
    -> BisectionStatus {
  TIT_ASSUME_UNIVERSAL(Func, f);
  TIT_ASSERT(min_x <= max_x, "Inverted search range!");
  using enum BisectionStatus;
  // Check for the region bounds first.
  auto min_f = std::invoke(f, min_x);
  if (abs(min_f) <= eps) {
    max_x = min_x;
    return success;
  }
  auto max_f = std::invoke(f, max_x);
  if (abs(max_f) <= eps) {
    min_x = max_x;
    return success;
  }
  for (size_t iter = 0; iter < max_iter; ++iter) {
    if (sign(max_f) == sign(min_f)) return failure_sign;
    // Approximate f(x) with line equation:
    // f(x) = min_f + (max_f - min_f)/(max_x - min_x) * (x - min_x),
    // so approximate root of f(x) == 0 is:
    auto const x = min_x - min_f * (max_x - min_x) / (max_f - min_f);
    auto const y = std::invoke(f, x);
    if (abs(y) <= eps) {
      min_x = max_x = x;
      return success;
    }
    auto const sign_y = sign(y);
    if (sign_y != sign(min_f)) max_x = x, max_f = y;
    else if (sign_y != sign(max_f)) min_x = x, min_f = y;
  }
  return fail_max_iter;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

} // namespace tit
