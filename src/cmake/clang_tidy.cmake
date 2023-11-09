# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
# Part of the Tit Solver project, under the MIT License
# See /LICENSE.md for license information.
# SPDX-License-Identifier: MIT
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

include_guard()
include(utils)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

# Find clang-tidy (at least 16.0).
find_program_with_version(
  CLANG_TIDY_EXE
  NAMES clang-tidy clang-tidy-16 clang-tidy-17
  MIN_VERSION 16.0)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

## Analyze source code with clang-tidy.
function(enable_clang_tidy TARGET_OR_ALIAS)
  # Exit early in case sufficient clang-tidy was not found.
  if(NOT CLANG_TIDY_EXE)
    return()
  endif()
  # Get the original target name if it is an alias.
  get_target_property(TARGET ${TARGET_OR_ALIAS} ALIASED_TARGET)
  if(NOT TARGET ${TARGET})
    set(TARGET ${TARGET_OR_ALIAS})
  endif()
  # Setup common arguments for IWYU call.
  set(
    CLANG_TIDY_ARGS
    # No annoying output.
    --quiet
    # Enable colors during piping through chronic.
    --use-color)
  # Setup "compilation" arguments for clang-tidy call.
  ## Get generated compile options from target.
  get_generated_compile_options(${TARGET} CLANG_TIDY_COMPILE_ARGS)
  ## Append some extra options.
  list(
    APPEND
    CLANG_TIDY_COMPILE_ARGS
    # Enable many warnings.
    # TODO: these may be moved to a global list once we have proper configurations.
    -Wall -Wextra -Wextra-semi -Wpedantic
    -Wredeclared-class-member
    -Wredundant-decls
    -Wredundant-move
    -Wredundant-parens
    -Wunused-comparison
    -Wunused-const-variable
    -Wunused-exception-parameter
    -Wunused-function
    -Wunused-label
    -Wunused-lambda-capture
    -Wunused-local-typedef
    -Wunused-parameter
    -Wunused-private-field
    -Wunused-template
    -Wunused-value
    -Wunused-variable
    -Wunused-volatile-lvalue
    # And disable some.
    -Wno-unknown-pragmas
    -Wno-unknown-warning-option
    # Enable C++23 (`c++2b` and not `c++23` for clang-16).
    -std=c++2b
    # Some C++23 features are not avaliable even in clang-17,
    # so tell our codebase that.
    -DTIT_IWYU=1)
  # Loop through the target sources and call clang-tidy.
  set(ALL_STAMPS)
  get_target_property(TARGET_SOURCE_DIR ${TARGET} SOURCE_DIR)
  get_target_property(TARGET_SOURCES ${TARGET} SOURCES)
  foreach(SOURCE ${TARGET_SOURCES})
    # Skip non-C/C++ files.
    is_cxx_source(${SOURCE} SOURCE_IS_CXX)
    if(NOT SOURCE_IS_CXX)
       continue()
    endif()
    # Create stamp.
    set(STAMP ${SOURCE}.tidy_stamp)
    list(APPEND ALL_STAMPS ${STAMP})
    # Execute clang-tidy and update a stamp file on success.
    # (wrapped with chronic to avoid annoying `N warnings generated` messages).
    set(SOURCE_PATH "${TARGET_SOURCE_DIR}/${SOURCE}")
    add_custom_command(
      OUTPUT ${STAMP}
      ## Execute clang-tidy.
      COMMAND
        "${CHRONIC_EXE}"
        "${CLANG_TIDY_EXE}" "${SOURCE_PATH}"
        ${CLANG_TIDY_ARGS} -- ${CLANG_TIDY_COMPILE_ARGS}
      ## Update stamp.
      COMMAND
        "${CMAKE_COMMAND}" -E touch "${STAMP}"
      MAIN_DEPENDENCY "${SOURCE_PATH}"
      ## Also check all the dependant files.
      IMPLICIT_DEPENDS CXX "${SOURCE_PATH}"
      COMMENT "Analyzing ${SOURCE_PATH}"
      ## This is needed for generator expressions to work.
      COMMAND_EXPAND_LISTS VERBATIM)
  endforeach()
  # Create a custom target that should "build" once all checks succeed.
  add_custom_target("${TARGET}_tidy" ALL DEPENDS ${TARGET} ${ALL_STAMPS})
endfunction()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
