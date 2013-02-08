TESTS=				header_test
check_PROGRAMS=			${TESTS}
lib_LTLIBRARIES=		libhttp-server.la
httpserverincludedir=		${includedir}/http
httpserverinclude_HEADERS=	server.h

libhttp_server_la_SOURCES=	server.cc connection.cc http.cc header.cc
libhttp_server_la_LDFLAGS=	-version-info ${LIBRARY_VERSION}
libhttp_server_la_LIBADD=	${AC_LIBS}

CLEANFILES=

${TESTS}:	LDADD="${AC_LIBS} ${GTEST_LIBS} ${lib_LTLIBRARIES}"
