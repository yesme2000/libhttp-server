/*
 * Unit Test for the Cookie Implementation.
 */

#include "server.h"
#include <gtest/gtest.h>

namespace http
{
namespace server
{
namespace testing
{
class CookieTest : public ::testing::Test
{
};

TEST_F(CookieTest, NameValue)
{
	Cookie c;
	c.name = "test1";
	c.value = "some value with; characters' in it!";
	c.version = 0;
	EXPECT_EQ("test1=\"some value with%3B characters%27 in it%21\"",
			c.ToString().toStdString());
}

TEST_F(CookieTest, Path)
{
	Cookie c;
	c.name = "a";
	c.value = "b";
	c.path = "/some/path/";
	c.version = 0;
	EXPECT_EQ("a=\"b\"; Path=/some/path/",
			c.ToString().toStdString());
}

TEST_F(CookieTest, Domain)
{
	Cookie c;
	c.name = "a";
	c.value = "b";
	c.domain = ".ngas.ch";
	c.version = 0;
	EXPECT_EQ("a=\"b\"; Domain=.ngas.ch",
			c.ToString().toStdString());
}

TEST_F(CookieTest, Expires)
{
	Cookie c;
	c.name = "a";
	c.value = "b";
	c.expires = QDateTime::fromTime_t(1360542537);
	c.version = 0;
	EXPECT_EQ("a=\"b\"; Expires=Mon, 11-Feb-2013 00:28:57 GMT",
			c.ToString().toStdString());
}

TEST_F(CookieTest, MaxAge)
{
	Cookie c;
	c.name = "a";
	c.value = "b";
	c.max_age = 12345;
	c.version = 0;
	EXPECT_EQ("a=\"b\"; Max-Age=12345", c.ToString().toStdString());
}

TEST_F(CookieTest, Port)
{
	Cookie c;
	c.name = "a";
	c.value = "b";
	c.port = 12345;
	c.version = 0;
	EXPECT_EQ("a=\"b\"; Port=12345", c.ToString().toStdString());
}

TEST_F(CookieTest, Secure)
{
	Cookie c;
	c.name = "a";
	c.value = "b";
	c.secure = true;
	c.version = 0;
	EXPECT_EQ("a=\"b\"; Secure", c.ToString().toStdString());
}

TEST_F(CookieTest, Discard)
{
	Cookie c;
	c.name = "a";
	c.value = "b";
	c.discard = true;
	c.version = 0;
	EXPECT_EQ("a=\"b\"; Discard", c.ToString().toStdString());
}

} /* namespace testing */
} /* namespace server */
} /* namespace http */
