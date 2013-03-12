#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([libhttp-server], [0.1], [tonnerre@ancient-solutions.com])
AC_CONFIG_SRCDIR([request.cc])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE
LT_INIT

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL
AC_LANG(C++)

LIBRARY_VERSION=`echo $VERSION | $SED -e's/\./:/g'`
AC_SUBST(LIBRARY_VERSION)

OLDCXXFLAGS="$CXXFLAGS"
CXXFLAGS="$CXXFLAGS -std=c++11"
AC_MSG_CHECKING([whether $CXX accepts -std=c++11])
AC_LINK_IFELSE([AC_LANG_CALL([], [main])], [AC_MSG_RESULT(yes)],
	[AC_MSG_RESULT(no)
	 CXXFLAGS="$OLDCXXFLAGS -std=c++0x"
	 AC_MSG_CHECKING([whether $CXX accepts -std=c++0x])
	 AC_LINK_IFELSE([AC_LANG_CALL([], [main])], [AC_MSG_RESULT(yes)],
	 [AC_MSG_RESULT(no); CXXFLAGS="$OLDCXXFLAGS"])])
unset OLDCXXFLAGS
AC_SUBST(CXXFLAGS)

# Checks for libraries.
GTEST_LIBS=""
AC_CHECK_LIB([z], [crc32],
	     [AC_LIBS="$AC_LIBS -lz"])
AC_CHECK_LIB([thread++], [main],
	     [AC_LIBS="$AC_LIBS -lthread++"],
	     [AC_ERROR(libthread++ is required)])
AC_CHECK_LIB([serialdata], [main],
	     [AC_LIBS="$AC_LIBS -lserialdata"
	      AC_DEFINE([BINARY_LOG], [1], [Enable binary logging])])
AC_CHECK_LIB([glog], [main],
	     [AC_LIBS="$AC_LIBS -lglog"])
AC_CHECK_LIB([toolbox], [main],
	     [AC_LIBS="$AC_LIBS -ltoolbox"],
	     [AC_ERROR(libtoolbox is required)])
AC_CHECK_LIB([toolbox_crypto], [main],
	     [AC_LIBS="$AC_LIBS -ltoolbox_crypto"],
	     [AC_ERROR(libtoolbox_crypto is required)])
AC_CHECK_LIB([siot], [main],
	     [AC_LIBS="$AC_LIBS -lsiot"],
	     [AC_ERROR(libsiot is required)])
ORIGLIBS="${LIBS}"
LIBS="${LIBS} ${AC_LIBS}"
AC_CHECK_LIB([protobuf], [main],
	     [AC_LIBS="$AC_LIBS -lprotobuf"],
	     [AC_ERROR(libprotobuf is required)])
LIBS="${ORIGLIBS}"
AC_CHECK_LIB([gtest], [main],
	     [GTEST_LIBS="$GTEST_LIBS -lgtest"])
AC_CHECK_LIB([gtest_main], [main],
	     [GTEST_LIBS="$GTEST_LIBS -lgtest_main"])
AC_SUBST(GTEST_LIBS)
AC_SUBST(AC_LIBS)

# Checks for header files.
old_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $INCLUDES"
AC_CHECK_HEADERS([siot/server.h siot/connection.h toolbox/expvar.h])
CPPFLAGS="$old_CPPFLAGS"

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
