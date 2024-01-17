#pragma once

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <unordered_map>
#include <atomic>
#include <string>
#include <mutex>
#include <memory>

namespace beast = boost::beast;
namespace asio = boost::asio;

class ChatServer {
private:
	std::atomic<unsigned int> next_id;
	asio::io_context io;
	asio::ip::tcp::acceptor acceptor;
	std::unordered_map<
		std::string,
		std::shared_ptr<beast::websocket::stream<asio::ip::tcp::socket>>
	> clients;
	std::mutex client_mut;
	void accept(asio::ip::tcp::socket&&);
	void fan_out(
		std::string& //msg
	);
	void fan_out(
		std::string&, //client id
		std::string& //msg
	);
public:
	ChatServer(int);
	void run();
};
