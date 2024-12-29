# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Part of the Tit Solver project, under the MIT License.
# See /LICENSE.md for license information. SPDX-License-Identifier: MIT
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

include_guard()
include(clang)
include(compiler)
include(pnpm)
include(sphinx)
include(utils)

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Common target name prefix.
set(TARGET_NAME_PREFIX "tit")

#
# Make a target name.
#
function(make_target_name NAME RESULT_VAR)
  if(NAME MATCHES "^${TARGET_NAME_PREFIX}")
    set(${RESULT_VAR} "${NAME}" PARENT_SCOPE)
  else()
    set(${RESULT_VAR} "${TARGET_NAME_PREFIX}_${NAME}" PARENT_SCOPE)
  endif()
endfunction()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#
# Set the compile and link options for the given target.
#
function(configure_tit_target TARGET ACCESS)
  # Setup include directories.
  target_include_directories(
    ${TARGET} ${ACCESS}
    "${CMAKE_SOURCE_DIR}/source"
  )

  # Setup compile and link options.
  target_compile_features(${TARGET} ${ACCESS} cxx_std_23)
  foreach(CONFIG ${ALL_CONFIGS})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)

    # Apply compilation options.
    target_compile_options(
      ${TARGET} ${ACCESS}
      $<$<CONFIG:${CONFIG}>:${CXX_COMPILE_OPTIONS_${CONFIG_UPPER}}>
    )

    # Apply link options.
    target_link_options(
      ${TARGET} ${ACCESS}
      $<$<CONFIG:${CONFIG}>:${CXX_LINK_OPTIONS_${CONFIG_UPPER}}>
    )
  endforeach()
endfunction()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#
# Add a library.
#
function(add_tit_library)
  # Parse and check arguments.
  cmake_parse_arguments(
    LIB
    ""
    "NAME;TYPE;DESTINATION"
    "SOURCES;DEPENDS"
    ${ARGN}
  )
  if(NOT LIB_NAME)
    message(FATAL_ERROR "Library name must be specified.")
  endif()

  if(LIB_TYPE)
    # Library type is specified.
    set(ALL_LIB_TYPES INTERFACE OBJECT STATIC SHARED)
    if(NOT (LIB_TYPE IN_LIST ALL_LIB_TYPES))
      list(JOIN ALL_LIB_TYPES ", " ALL_LIB_TYPES)
      message(
        FATAL_ERROR
        "Library type can be one of the following options: ${ALL_LIB_TYPES}."
      )
    endif()
  else()
    # Determine library type based on the presence of compilable sources.
    set(LIB_HAS_SOURCES FALSE)
    if(LIB_SOURCES)
      foreach(SOURCE ${LIB_SOURCES})
        is_cpp_source("${SOURCE}" LIB_HAS_SOURCES)
        if(LIB_HAS_SOURCES)
          break()
        endif()
      endforeach()
    endif()
    if(LIB_HAS_SOURCES)
      set(LIB_TYPE STATIC)
    else()
      set(LIB_TYPE INTERFACE)
    endif()
  endif()

  # Setup visibility.
  if(LIB_TYPE STREQUAL "INTERFACE")
    set(LIB_PUBLIC INTERFACE)
  else()
    set(LIB_PUBLIC PUBLIC)
  endif()

  # Create the library and the alias.
  make_target_name(${LIB_NAME} LIB_TARGET)
  set(LIB_TARGET_ALIAS "${TARGET_NAME_PREFIX}::${LIB_NAME}")
  add_library(${LIB_TARGET} ${LIB_TYPE} ${LIB_SOURCES})
  add_library(${LIB_TARGET_ALIAS} ALIAS ${LIB_TARGET})

  # Configure the target.
  configure_tit_target(${LIB_TARGET} ${LIB_PUBLIC})

  # Link with the dependent libraries.
  target_link_libraries(${LIB_TARGET} ${LIB_PUBLIC} ${LIB_DEPENDS})

  # Install the library.
  if(LIB_DESTINATION)
    set(INSTALLABLE_LIB_TYPES STATIC SHARED)
    if(NOT LIB_TYPE IN_LIST INSTALLABLE_LIB_TYPES)
      message(FATAL_ERROR "Cannot install library of type: ${LIB_TYPE}!")
    endif()
    install(TARGETS ${LIB_TARGET} LIBRARY DESTINATION "${LIB_DESTINATION}")
  endif()

  # Enable static analysis.
  if(LIB_SOURCES)
    enable_clang_tidy(${LIB_TARGET})
  endif()
endfunction()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#
# Add an executable.
#
function(add_tit_executable)
  # Parse and check arguments.
  cmake_parse_arguments(
    EXE
    ""
    "NAME;PREFIX;DESTINATION"
    "SOURCES;DEPENDS"
    ${ARGN}
  )
  if(NOT EXE_NAME)
    message(FATAL_ERROR "Executable name must be specified.")
  endif()

  # Create the executable and the alias.
  make_target_name(${EXE_NAME} EXE_TARGET)
  set(EXE_TARGET_ALIAS "${TARGET_NAME_PREFIX}::${EXE_NAME}")
  add_executable(${EXE_TARGET} ${EXE_SOURCES})
  add_executable(${EXE_TARGET_ALIAS} ALIAS ${EXE_TARGET})

  # Configure the target.
  configure_tit_target(${EXE_TARGET} PRIVATE)

  # Link with the dependent libraries.
  target_link_libraries(${EXE_TARGET} PRIVATE ${EXE_DEPENDS})

  # Install the executable.
  if(EXE_DESTINATION)
    install(TARGETS ${EXE_TARGET} RUNTIME DESTINATION "${EXE_DESTINATION}")
  endif()

  # Enable static analysis.
  if(EXE_SOURCES)
    enable_clang_tidy(${EXE_TARGET})
  endif()
endfunction()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#
# Add a web application target.
#
function(add_tit_webapp)
  # Parse and check arguments.
  cmake_parse_arguments(WEBAPP "" "NAME;DESTINATION" "" ${ARGN})
  if(NOT WEBAPP_NAME)
    message(FATAL_ERROR "Web target name must be specified.")
  endif()

  # Create the target.
  make_target_name(${WEBAPP_NAME} WEBAPP_TARGET)
  add_pnpm_target(${WEBAPP_TARGET})

  # Install the target.
  if(WEBAPP_DESTINATION)
    install_pnpm_target(
      TARGET ${WEBAPP_TARGET}
      DESTINATION "${WEBAPP_DESTINATION}"
    )
  endif()

  # Enable static analysis.
  lint_pnpm_target(${WEBAPP_TARGET})
endfunction()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#
# Add a documentation target.
#
function(add_tit_documentation)
  # Parse and check arguments.
  cmake_parse_arguments(DOC "" "NAME;DESTINATION" "" ${ARGN})
  if(NOT DOC_NAME)
    message(FATAL_ERROR "Documentation name must be specified.")
  endif()

  # Create the target.
  make_target_name(${DOC_NAME} DOC_TARGET)
  add_sphinx_target(${DOC_TARGET})

  # Install the target.
  if(DOC_DESTINATION)
    install_sphinx_target(TARGET ${DOC_TARGET} DESTINATION "${DOC_DESTINATION}")
  endif()
endfunction()

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
