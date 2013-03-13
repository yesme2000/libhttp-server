/*
 * Unit Test for the Request Object.
 */

#include "server.h"
#include <gtest/gtest.h>

namespace http
{
namespace server
{
namespace testing
{
class RequestTest : public ::testing::Test
{
};

TEST_F(RequestTest, AddCookie)
{
	Request r;
	list<Cookie*> cookies = r.GetCookies();
	Cookie* a = new Cookie;
	Cookie* a2 = new Cookie;
	Cookie* b = new Cookie;

	a->name = "a";
	a2->name = "a";
	b->name = "b";

	EXPECT_EQ(0, cookies.size());

	r.AddCookie(a);
	cookies = r.GetCookies();
	EXPECT_EQ(1, cookies.size());
	EXPECT_EQ(a, cookies.front());

	r.AddCookie(a2);
	cookies = r.GetCookies();
	EXPECT_EQ(1, cookies.size());
	EXPECT_EQ(a2, cookies.front());

	r.AddCookie(b);
	cookies = r.GetCookies();
	EXPECT_EQ(2, cookies.size());
	EXPECT_EQ(a2, cookies.front());
	EXPECT_EQ(b, cookies.back());
}

TEST_F(RequestTest, Headers)
{
	Request r;
	Headers* hdr = new Headers;
	Headers* hdr2 = new Headers;

	EXPECT_EQ(0, r.GetHeaders());

	r.SetHeaders(hdr);
	EXPECT_EQ(hdr, r.GetHeaders());

	r.SetHeaders(hdr2);
	EXPECT_EQ(hdr2, r.GetHeaders());
}

TEST_F(RequestTest, Referer)
{
	Request r;
	Headers* hdr = new Headers;

	EXPECT_EQ("", r.Referer());

	hdr->Set("Referer", "http://lolcathost/wtf.html");
	r.SetHeaders(hdr);

	EXPECT_EQ("http://lolcathost/wtf.html", r.Referer());
}

TEST_F(RequestTest, UserAgent)
{
	Request r;
	Headers* hdr = new Headers;

	EXPECT_EQ("", r.Referer());

	hdr->Set("User-Agent",
			"Lynx/2.8.4rel.1 libwww-FM/2.14 SSL-MM/1.4.1");
	r.SetHeaders(hdr);

	EXPECT_EQ("Lynx/2.8.4rel.1 libwww-FM/2.14 SSL-MM/1.4.1",
			r.UserAgent());
}

TEST_F(RequestTest, Host)
{
	Request r;
	Headers* hdr = new Headers;

	EXPECT_EQ("", r.Host());

	hdr->Set("Host", "lolcathost.example.com");
	r.SetHeaders(hdr);

	EXPECT_EQ("lolcathost.example.com", r.Host());
}

TEST_F(RequestTest, RequestProperties)
{
	Request r;

	EXPECT_EQ("", r.Path());
	EXPECT_EQ("", r.Protocol());
	EXPECT_EQ("", r.Action());

	r.SetPath("/foo/bar");
	r.SetProtocol("HTTP/1.1");
	r.SetAction("GET");

	EXPECT_EQ("/foo/bar", r.Path());
	EXPECT_EQ("HTTP/1.1", r.Protocol());
	EXPECT_EQ("GET", r.Action());
}

TEST_F(RequestTest, BasicAuth)
{
	Request r;
	Headers* hdr = new Headers;
	pair<string, string> auth = r.GetBasicAuth();

	EXPECT_EQ("", auth.first);
	EXPECT_EQ("", auth.second);

	r.SetHeaders(hdr);
	auth = r.GetBasicAuth();
	EXPECT_EQ("", auth.first);
	EXPECT_EQ("", auth.second);

	r.SetBasicAuth("user", "password");
	hdr = r.GetHeaders();
	EXPECT_EQ("Basic dXNlcjpwYXNzd29yZA==",
			hdr->GetFirst("Authorization"));

	auth = r.GetBasicAuth();
	EXPECT_EQ("user", auth.first);
	EXPECT_EQ("password", auth.second);
}

TEST_F(RequestTest, AsURL)
{
	Request r;
	Headers* hdr = new Headers;

	hdr->Set("Host", "lolcathost.example.com");

	r.SetSchema("http");
	r.SetHeaders(hdr);
	r.SetPath("/foo/bar");
	EXPECT_EQ("http://lolcathost.example.com/foo/bar", r.AsURL());
}
}  // namespace testing
}  // namespace server
}  // namespace http
