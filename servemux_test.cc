/*
 * Unit Test for the Serve Multiplexer Object.
 */

#include "server.h"
#include "server_internal.h"
#include <gtest/gtest.h>

#include <iostream>

namespace http
{
namespace server
{
namespace testing
{
class MockHandler : public Handler
{
public:
	virtual void ServeHTTP(ResponseWriter* w, const Request* req)
	{
	}
};

class ServeMuxTest : public ::testing::Test
{
};

TEST_F(ServeMuxTest, MatchEverything)
{
	MockHandler a;
	ServeMux mux;

	mux.Handle("", &a);

	EXPECT_EQ(&a, mux.GetHandler("/"));
	EXPECT_EQ(&a, mux.GetHandler("/foo/bar/baz"));
}

TEST_F(ServeMuxTest, SimpleMatches)
{
	MockHandler a;
	MockHandler b;
	ServeMux mux;

	mux.Handle("/foo/", &a);
	mux.Handle("", &b);

	EXPECT_EQ(&b, mux.GetHandler("/"));
	EXPECT_EQ(&b, mux.GetHandler("/foo"));
	EXPECT_EQ(&a, mux.GetHandler("/foo/"));
	EXPECT_EQ(&a, mux.GetHandler("/foo/bar/baz"));
	EXPECT_EQ(&b, mux.GetHandler("/bar/foo/baz"));
}
}  // namespace testing
}  // namespace server
}  // namespace http
