/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *\
 * Part of the Tit Solver project, under the MIT License.
 * See /LICENSE.md for license information. SPDX-License-Identifier: MIT
\* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// IWYU pragma: private, include "tit/core/simd.hpp"
#pragma once

#include <bit>
#include <concepts>
#include <type_traits>

#include "tit/core/basic_types.hpp"
#include "tit/core/type_traits.hpp"

namespace tit::simd {

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/// Is SIMD supported for the specified numeric type?
template<class Num>
concept supported_type = std::integral<Num> || std::floating_point<Num>;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/// Minimal byte width of the SIMD register that is available on the current
/// hardware.
///
/// That is 16 bytes for all supported instruction sets.
inline constexpr size_t min_reg_byte_width_v{
#if defined __SSE__ || defined __ARM_NEON
    16
#else
#error Unknown SIMD instruction set!
#endif
};

/// Maximal byte width of the SIMD register that is available on the current
/// hardware.
///
/// That is 16 bytes for NEON and SSE instruction set, 32 and 64 bytes for
/// AVX and AVX-512 respectively.
inline constexpr size_t max_reg_byte_width_v{
#if defined __AVX512F__
    64
#elif defined __AVX__
    32
#elif defined __SSE__ || defined __ARM_NEON
    16
#else
#error Unknown SIMD instruction set!
#endif
};

// Just in case.
static_assert(max_reg_byte_width_v >= min_reg_byte_width_v &&
              std::has_single_bit(max_reg_byte_width_v / min_reg_byte_width_v));

/// Minimal SIMD register size for the specified type.
template<supported_type Num>
inline constexpr size_t min_reg_size_v = min_reg_byte_width_v / sizeof(Num);

/// Maximal SIMD register size for the specified type.
template<supported_type Num>
inline constexpr size_t max_reg_size_v = max_reg_byte_width_v / sizeof(Num);

/// Is SIMD supported for the specified numeric type and size?
template<class Num, size_t Size>
concept supported =
    supported_type<Num> &&
    in_range_v<Size, min_reg_size_v<Num>, max_reg_size_v<Num>> &&
    (Size % min_reg_size_v<Num> == 0);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace impl {

// In Highway, type is identified as an integer, if it is one of the standard
// fixed-width types. This leads to the problem that if, for example,
// `int64_t` is defined as `long long` on some platforms, the specialization
// for `long` would be missing. Even if `long` and `long long` have the same
// size, they are not the same type:
//
// static_assert(sizeof(long) == sizeof(long long) &&
//               !std::same_as<long, long long>);
//
// So, we need to map the incoming integral types to the corresponding
// standard fixed-width types, leaving other types as they are.

template<supported_type Num>
struct fixed_width_type;

template<supported_type Num>
using fixed_width_type_t = typename fixed_width_type<Num>::type;

template<std::unsigned_integral UInt>
  requires (sizeof(UInt) == 1)
struct fixed_width_type<UInt> : std::type_identity<uint8_t> {};

template<std::unsigned_integral UInt>
  requires (sizeof(UInt) == 2)
struct fixed_width_type<UInt> : std::type_identity<uint16_t> {};

template<std::unsigned_integral UInt>
  requires (sizeof(UInt) == 4)
struct fixed_width_type<UInt> : std::type_identity<uint32_t> {};

template<std::unsigned_integral UInt>
  requires (sizeof(UInt) == 8)
struct fixed_width_type<UInt> : std::type_identity<uint64_t> {};

template<std::signed_integral SInt>
struct fixed_width_type<SInt> :
    std::make_signed<fixed_width_type_t<std::make_unsigned_t<SInt>>> {};

template<std::floating_point Float>
struct fixed_width_type<Float> : std::type_identity<Float> {};

} // namespace impl

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

} // namespace tit::simd
