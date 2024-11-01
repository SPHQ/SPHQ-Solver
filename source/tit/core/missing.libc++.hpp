/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *\
 * Part of the Tit Solver project, under the MIT License.
 * See /LICENSE.md for license information. SPDX-License-Identifier: MIT
\* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#pragma once

#include <cstddef> // IWYU pragma: keep
#ifdef _LIBCPP_VERSION

#include <array>
#include <charconv>
#include <concepts>
#include <functional>
#include <iostream>
#include <ranges>
#include <sstream>
#include <utility>

#include "tit/core/uint_utils.hpp"

_LIBCPP_BEGIN_NAMESPACE_STD

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Missing range adaptors.
//

namespace ranges::views {

template<class Range>
concept good_range =
    std::ranges::viewable_range<Range> && std::ranges::sized_range<Range> &&
    std::ranges::random_access_range<Range>;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Simple implementation of `std::views::enumerate`.
struct EnumerateAdaptor {
  template<good_range Range>
  constexpr auto operator()(Range&& range) const noexcept {
    using Ref = std::ranges::range_reference_t<Range>;
    auto view = std::views::all(std::forward<Range>(range));
    return std::views::iota(0UZ, std::ranges::size(range)) |
           std::views::transform([view = std::move(view)](size_t index) {
             return std::pair<size_t, Ref>{index, view[index]};
           });
  }
};

inline constexpr EnumerateAdaptor enumerate{};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Simple implementation of `std::views::chunk`.
struct ChunkAdaptor {
  template<good_range Range>
  constexpr auto operator()(Range&& range, size_t chunk_size) const noexcept {
    auto view = std::views::all(std::forward<Range>(range));
    const auto num_chunks = tit::divide_up(std::ranges::size(view), chunk_size);
    return std::views::iota(0UZ, num_chunks) |
           std::views::transform(
               [view = std::move(view), chunk_size](size_t chunk_index) {
                 const auto sz = std::ranges::size(view);
                 const auto first = std::min(sz, chunk_index * chunk_size);
                 const auto last = std::min(sz, (chunk_index + 1) * chunk_size);
                 const auto iter = std::ranges::begin(view);
                 return std::ranges::subrange(iter + first, iter + last);
               });
  }
};

inline constexpr ChunkAdaptor chunk{};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Simple implementation of `std::views::adjacent_transform`.
template<size_t N>
struct AdjacentTransformAdaptor {
  static_assert(N == 2, "Only two adjacent elements are supported!");
  template<good_range Range, class Func>
  constexpr auto operator()(Range&& range, Func func) const noexcept {
    auto view = std::views::all(std::forward<Range>(range));
    return std::views::iota(0UZ, std::ranges::size(range) - 1) |
           std::views::transform(
               [view = std::move(view), func = std::move(func)](size_t index) {
                 return func(std::ranges::begin(view)[index],
                             std::ranges::begin(view)[index + 1]);
               });
  }
  template<class Func>
  constexpr auto operator()(Func func) const {
    return __range_adaptor_closure_t(std::__bind_back(*this, std::move(func)));
  }
};

template<size_t N>
inline constexpr AdjacentTransformAdaptor<N> adjacent_transform{};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Simple implementation of `std::views::cartesian_product`.
// This simple implementation requires all ranges to have the same value type.
struct CartesianProductAdaptor {
  template<good_range... Ranges>
  constexpr auto operator()(Ranges&&... ranges) const noexcept {
    static constexpr auto Dim = sizeof...(Ranges);
    const auto flat_size = (std::ranges::size(ranges) * ...);
    std::array views{std::views::all(std::forward<Ranges>(ranges))...};
    using Elem = decltype(auto(views[0][0]));
    return std::views::iota(0UZ, flat_size) |
           std::views::transform([views = std::move(views)](size_t flat_index) {
             std::array<Elem, Dim> items{};
             for (ssize_t axis = Dim - 1; axis >= 0; --axis) {
               const auto size = std::ranges::size(views[axis]);
               const auto index = flat_index % size;
               items[axis] = views[axis][index];
               flat_index /= size;
             }
             return items;
           });
  }
};

inline constexpr CartesianProductAdaptor cartesian_product{};

} // namespace ranges::views

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Miscellaneous.
//

// Argumentless `println` is not implemented in libc++.
inline void println() {
  std::cout << '\n';
}

#if _LIBCPP_VERSION < 190100
// `std::bind_back` is a private implementation detail before libc++ 19.1.
template<class Func, class... BackArgs>
constexpr auto bind_back(Func&& func, BackArgs&&... back_arguments) {
  return __bind_back(std::forward<Func>(func),
                     std::forward<BackArgs>(back_arguments)...);
}
#endif

// `std::from_chars` does not support floating-point types.
template<std::floating_point Float>
auto from_chars(const char* first, // NOLINT(*-exception-escape)
                const char* last,
                Float& result,
                std::chars_format /*fmt*/ = std::chars_format::general) noexcept
    -> std::from_chars_result {
  std::istringstream stream{std::string{first, last}};
  stream >> result;
  if (stream.fail()) return {first, std::errc::invalid_argument};
  return {last, std::errc{}}; // NOLINT
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

_LIBCPP_END_NAMESPACE_STD

#endif // ifdef _LIBCPP_VERSION
