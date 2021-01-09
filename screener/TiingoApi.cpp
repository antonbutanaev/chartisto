#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>

#include "RootCertificate.hpp"
#include "TiingoApi.h"

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;
using namespace std;

namespace tiingo {

struct TiingoApi::Impl {
	Impl(const char *host, const char *port, const std::string &authToken) :
		authToken(authToken),
		host(host),
		port(port)
	{
        load_root_certificates(ctx);
        ctx.set_verify_mode(boost::asio::ssl::verify_none);
	}

	~Impl() {}

	Json::Value getData(const string &ticker, Date from, Date to) {
		int version = 10;

		stringstream target;
		target
			<< "/tiingo/daily/" << ticker
			<< "/prices?startDate=" << from
			<< "&endDate=" << to
			<< "&token=" << authToken;

        beast::ssl_stream<beast::tcp_stream> tcpStream(ioc, ctx);
        if(!SSL_set_tlsext_host_name(tcpStream.native_handle(), host))
        {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
            throw beast::system_error{ec};
        }

        auto const results = resolver.resolve(host, port);
        beast::get_lowest_layer(tcpStream).connect(results);
        tcpStream.handshake(ssl::stream_base::client);

        http::request<http::string_body> req{http::verb::get, target.str().c_str(), version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        http::write(tcpStream, req);
        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(tcpStream, buffer, res);

		stringstream stream;
		for (auto seq : res.body().data()) {
			auto buf = net::buffer_cast<const char*>(seq);
			stream.write(buf, net::buffer_size(seq));
		}

		Json::Value root;
		stream >> root;

        beast::error_code ec;
        tcpStream.shutdown(ec);
        if(ec == net::error::eof)
            ec = {};
        if(ec)
            cerr << "error shutdown stream: " << ec << endl;

		return root;
	}

    const std::string authToken;
	const char *host;
	const char *port;
    net::io_context ioc;
    ssl::context ctx{ssl::context::tlsv12_client};
    tcp::resolver resolver{ioc};
};

TiingoApi::TiingoApi(const std::string &authToken) : i_(new Impl("api.tiingo.com", "443", authToken)) {
}

TiingoApi::~TiingoApi() {
}

Json::Value TiingoApi::getData(const string &ticker, Date from, Date to) {
	return i_->getData(ticker, from, to);
}

}
