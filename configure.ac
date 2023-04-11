#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Tambur], [0.1], [mhrudow@gmail.com,francisy@microsoft.com])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_HEADERS([config.h])

#CXXFLAGS='-O3'
CXX17_FLAGS="-std=c++17 -pthread"
PICKY_CXXFLAGS="-pedantic -Wall -Wextra  -Winit-self -Woverloaded-virtual -Wuninitialized -Winit-self"
AC_SUBST([CXX17_FLAGS])
AC_SUBST([PICKY_CXXFLAGS])

# Checks for programs.
AC_PROG_CXX
AC_PROG_RANLIB

# Checks for libraries.
PKG_CHECK_MODULES([gtest],[gtest])
PKG_CHECK_MODULES([VPX],[vpx])
PKG_CHECK_MODULES([SDL],[sdl2])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_LIB([gf_complete], [gf_init_easy])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    src/util/Makefile
    src/examples/Makefile
    src/fec_examples/Makefile
    src/ringmaster_app/Makefile
    src/tests/Makefile
])

AC_OUTPUT
