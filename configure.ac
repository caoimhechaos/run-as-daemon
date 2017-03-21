#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([run-as-daemon], [1.1], [caoimhechaos@protonmail.com])
AC_CONFIG_SRCDIR([main.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_CHECK_PROG([logger])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h memory.h stdint.h stdlib.h string.h strings.h unistd.h pwd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_UID_T

# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([dup2])

AC_DEFINE([_GNU_SOURCE], [], [Use GNU extensions of libc, e.g. asprintf])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
