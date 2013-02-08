/*
 * Unit Test for the Protocol Header Implementation.
 */

#include "server.h"
#include <gtest/gtest.h>

namespace http
{
namespace server
{
namespace testing
{
class HeaderTest : public ::testing::Test
{
};

class HeadersTest : public ::testing::Test
{
};

TEST_F(HeaderTest, AddDeleteClearValues)
{
	QList<QString> values;
	values.push_back("close");

	Header h("Connectiop", values);

	EXPECT_EQ("close", h.GetFirstValue());
	EXPECT_EQ("Connectiop", h.GetName());
	h.SetName("Connection");
	EXPECT_EQ("Connection", h.GetName());

	h.AddValue("keep-alive");
	EXPECT_EQ("close", h.GetFirstValue());

	QList<QString> testvals = h.GetValues();
	EXPECT_EQ(2, testvals.length());
	EXPECT_EQ("close", testvals[0]);
	EXPECT_EQ("keep-alive", testvals[1]);

	h.DeleteValue("close");
	EXPECT_EQ("keep-alive", h.GetFirstValue());
	testvals = h.GetValues();
	EXPECT_EQ(1, testvals.length());
	EXPECT_EQ("keep-alive", testvals[0]);

	h.ClearValues();
	EXPECT_EQ("", h.GetFirstValue());
	testvals = h.GetValues();
	EXPECT_EQ(0, testvals.length());
}

TEST_F(HeaderTest, Merge)
{
	QList<QString> values;
	values.push_back("close");
	Header h1("Connection", values);
	values.clear();
	values.push_back("keep-alive");
	Header h2("Connection", values);
	values.clear();
	values.push_back("42");
	Header h3("Content-Length", values);

	EXPECT_TRUE(h1.Merge(h2));
	EXPECT_EQ("close", h1.GetFirstValue());
	values = h1.GetValues();
	EXPECT_EQ(2, values.length());
	EXPECT_EQ("close", values[0]);
	EXPECT_EQ("keep-alive", values[1]);

	values = h2.GetValues();
	EXPECT_EQ(1, values.length());
	EXPECT_EQ("keep-alive", values[0]);

	EXPECT_FALSE(h1.Merge(h3));
	EXPECT_EQ("close", h1.GetFirstValue());
	values = h1.GetValues();
	EXPECT_EQ(2, values.length());
	EXPECT_EQ("close", values[0]);
	EXPECT_EQ("keep-alive", values[1]);
}

TEST_F(HeadersTest, AddSetDelGet)
{
	Headers h;

	h.Set("Connection", "close");
	h.Add("Connection", "keep-alive");

	Header* conn = h.Get("Connection");
	ASSERT_NE((Header*) 0, conn);

	QList<QString> values = conn->GetValues();
	EXPECT_EQ(2, values.length());
	EXPECT_EQ("close", values[0]);
	EXPECT_EQ("keep-alive", values[1]);

	h.Set("Connection", "closed");
	conn = h.Get("Connection");
	ASSERT_NE((Header*) 0, conn);

	values = conn->GetValues();
	EXPECT_EQ(1, values.length());
	EXPECT_EQ("closed", values[0]);

	h.Delete("Connection");
	EXPECT_EQ(0, h.Get("Connection"));
}

} /* namespace testing */
} /* namespace server */
} /* namespace http */
