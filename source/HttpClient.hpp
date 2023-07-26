//#include <boost/asio/ssl/error.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/version.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/stream_traits.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/core/ignore_unused.hpp>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>



namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;



namespace testClient {
    struct State {
        net::io_context &ioc;
        ssl::context &ctx;
        int httpVersion;
        std::vector<std::size_t> ids;
    };

    void fail(beast::error_code ec, char const *what) {
        std::cerr << what << ": " << ec.message() << "\n";
    }

    class session : public std::enable_shared_from_this<session>{
    public:
        explicit session(net::any_io_executor ex, State &state) :
                state_(state),
                resolver_(ex),
                stream_(ex, state.ctx){}

        void run(char const *host, char const *service, char const *path) {
            if (!SSL_set_tlsext_host_name(stream_.native_handle(), host)) {
                beast::error_code ec{static_cast<int>(ERR_get_error()),
                                     net::error::get_ssl_category()};
                std::cerr << ec.message() << "\n";
                return;
            }

            req_.version(state_.httpVersion);
            req_.method(http::verb::get);
            req_.target(path);
            req_.set(http::field::host, host);
            req_.set(http::field::user_agent, BOOST_BEAST_DEPRECATION_STRING);
            std::cout << "Requesting"
                      << "\n";

            // Look up the domain name
            resolver_.async_resolve(host, service,
                                    beast::bind_front_handler(
                                            &session::on_resolve, shared_from_this()));
        }

        void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
            if (ec)
                return fail(ec, "resolve");
            beast::get_lowest_layer(stream_).expires_after(
                    std::chrono::seconds(30));
            // Make the connection on the IP address we get from a lookup
            beast::get_lowest_layer(stream_).async_connect(results,
                                                           beast::bind_front_handler(
                                                                   &session::on_connect, shared_from_this()));
        }

        void on_connect(
                beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
            if (ec)
                return fail(ec, "connect");

            stream_.async_handshake(ssl::stream_base::client,
                                    beast::bind_front_handler(
                                            &session::on_handshake, shared_from_this()));
        }

        void on_handshake(beast::error_code ec) {
            if (ec)
                return fail(ec, "handshake");
            do_write();
        }

        void do_write() {
            beast::get_lowest_layer(stream_).expires_after(
                    std::chrono::seconds(30));

            http::async_write(stream_, req_,
                              beast::bind_front_handler(&session::on_write, shared_from_this()));
        }

        void on_write(beast::error_code ec, std::size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);
            if (ec) {
                return fail(ec, "write");
            }

            http::async_read(stream_, buffer_, res_,
                             beast::bind_front_handler(&session::on_read, shared_from_this()));
        }

        void on_read(beast::error_code ec, std::size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);
            if (ec)
                return fail(ec, "read");

            try {
                std::cout << "Received data" << res_.body() << "\n";
                state_.ids.push_back(1);
            }
            catch (...) {
                std::cerr << "Retrive failed"
                          << "\n";
            }
            beast::get_lowest_layer(stream_).expires_after(
                    std::chrono::seconds(30));

            stream_.async_shutdown(beast::bind_front_handler(
                    &session::on_shutdown, shared_from_this()));
        }

        void on_shutdown(beast::error_code ec) {
            if (ec == net::error::eof) {
                ec = {};
            }
            if (ec)
                return fail(ec, "shutdown");
        }


    private:
        State &state_;
        tcp::resolver resolver_;
        beast::ssl_stream<beast::tcp_stream> stream_;
        beast::flat_buffer buffer_;
        http::request<http::empty_body> req_;
        http::response<http::string_body> res_;
    };

//    int run(const char *host, const char *service, int version){
//        net::io_context ioc;
//        ssl::context ctx{ssl::context::tlsv12_client};
//        load_root_ceriticates(ctx);
//
//        ctx.set_verify_mode(ssl::verify_peer);
//        ctx.set_default_verify_paths();
//
//        State state{ioc, ctx, version};
//
//        for (const char *path: {"/public/time"})
//            std::make_shared<session>(net::make_strand(ioc), state)->run(host, service, path);
//
//        ioc.run();
//
//        return EXIT_SUCCESS;
//    }
}
