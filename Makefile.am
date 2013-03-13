TESTS=				header_test cookie_test request_test	\
				servemux_test
check_PROGRAMS=			${TESTS}
bin_PROGRAMS=			testwebserver
lib_LTLIBRARIES=		libhttp-server.la
httpserverincludedir=		${includedir}/http
httpserverinclude_HEADERS=	server.h
noinst_HEADERS=			server_internal.h

testwebserver_SOURCES=		testwebserver.cc
testwebserver_LDADD=		${AC_LIBS} ${lib_LTLIBRARIES}

libhttp_server_la_SOURCES=	cookie.cc error_handler.cc header.cc	\
				http.cc request.cc responsewriter.cc	\
				servemux.cc server.cc
libhttp_server_la_LDFLAGS=	-version-info ${LIBRARY_VERSION}
libhttp_server_la_LIBADD=	${AC_LIBS}

CLEANFILES=

${TESTS}:	LDADD="${AC_LIBS} ${GTEST_LIBS} ${lib_LTLIBRARIES}"
