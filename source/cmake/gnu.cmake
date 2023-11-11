# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
# Part of the Tit Solver project, under the MIT License
# See /LICENSE.md for license information.
# SPDX-License-Identifier: MIT
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

include_guard()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

# Set minimal compiler version.
set(GNU_MIN_VERSION "13.1")

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

# Set warnings and diagnostics options.
set(
  GNU_WARNINGS
  # Enable most of the commonly used warning options.
  -Wall
  # Enable extra warning options not included in `-Wall`.
  -Wextra
  # Treat warnings as errors.
  -Werror
  # Issue all the warnings demanded by strict ISO C and ISO C++ standards.
  -Wpedantic
  # Warn about pointer casts that increase or decrease alignment.
  -Wcast-align
  # Warn if a pointer is cast so that it removes the `const` qualifier.
  -Wcast-qual
  # Warn about type conversions that may lose data.
# -Wconversion # I am not sure we have to enable this.
  # Warn when character array subscripts have type `int`.
  -Wchar-subscripts
  # Warn about potentially ambiguous or dangling `else` clauses in code.
  -Wdangling-else
  # Warn when an optimization pass is disabled.
  -Wdisabled-optimization
  # Warn about duplicated or redundant conditions in code.
  -Wduplicated-cond
  # Warn about floating-point equality comparisons.
  -Wfloat-equal
  # Check calls to `printf`-like functions for format string-related issues.
  -Wformat=2
  # Warn about self-initialization.
  -Winit-self
  # Warn about logical operations in code that is likely to be a mistake.
  -Wlogical-op
  # Warn if a global function is defined without a previous declaration.
  -Wmissing-declarations
  # Warn about using C-style casts.
  -Wold-style-cast
  # Warn about structures that are packed to an unusual degree.
  -Wpacked
  # Warn about possibly incorrect pointer arithmetic.
  -Wpointer-arith
  # Warn about redundant declarations.
  -Wredundant-decls
  # Warn about incorrect use of the `restrict` keyword.
  -Wrestrict
  # Warn about implicit conversions changing the sign of an integer.
  -Wsign-promo
  # Warn if a local variable or type declaration shadows another local variable.
  -Wshadow
  # Warn about left-shifting negative values.
  -Wshift-negative-value
  # Warn about potential arithmetic overflows during shifts.
  -Wshift-overflow
  # Warn if a `switch` statement does not have a `default` case.
  -Wswitch-enum
  # Warn about the use of uninitialized variables.
  -Wuninitialized
  # Warn when string constants are passed to non-`const char*` parameters.
  -Wwrite-strings
  # Warn about violations of pointer safety annotations (disabled).
  -Wno-psabi
  # TODO: enable this warning later.
  -Wno-unused-result)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

# Set optimization options (all configurations).
set(GNU_OPTIMIZE_OPTIONS
  # Optimize code for the host system's architecture.
  -march=native)

# Set optimization options (`Debug` configuration).
set(
  GNU_OPTIMIZE_OPTIONS_DEBUG
  # Inherit common options.
  ${GNU_OPTIMIZE_OPTIONS}
  # Generate debugging information.
  -g
  # Disable optimization, resulting in the compilation of unoptimized code.
  # Useful for debugging and inspecting code in its original form.
  -O0)

# Set optimization options (`Coverage` configuration).
set(
  GNU_OPTIMIZE_OPTIONS_COVERAGE
  # Inherit all the `Debug` options.
  ${GNU_OPTIMIZE_OPTIONS_DEBUG}
  # Enable code coverage instrumentation during compilation.
  --coverage
  # Disable inlining of functions, preventing the compiler from optimizing
  # function calls by replacing them with the function's code.
  -fno-default-inline
  -fno-inline
  -fno-inline-small-functions
  # Disable copy constructor elision, which can help in debugging and
  # inspecting constructor calls.
  -fno-elide-constructors)

# Set optimization options (`Release` configuration).
set(
  GNU_OPTIMIZE_OPTIONS_RELEASE
  # Inherit common options.
  ${GNU_OPTIMIZE_OPTIONS}
  # Enable aggressive optimization levels to maximize performance.
  -Ofast
  # Optimize code for the host system's architecture.
  -march=native
  # Enables aggressive floating-point expression contraction.
  -ffp-contract=fast
  # Set a high limit for inlining functions.
  -finline-limit=10000000)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

# Set link options (all configurations).
set(GNU_LINK_OPTIONS)
if(APPLE)
  # Enable "classic" linker for macOS (if possible).
  include(CheckLinkerFlag)
  check_linker_flag(CXX -ld_classic HAVE_LD_CLASSIC)
  if(HAVE_LD_CLASSIC)
    list(APPEND GNU_LINK_OPTIONS -ld_classic)
  endif()
endif()

# Set the link options (`Coverage` configuration).
set(
  GNU_LINK_OPTIONS_COVERAGE
  # Inherit common options.
  ${GNU_LINK_OPTIONS}
  # Enables code coverage instrumentation during linking.
  --coverage)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
