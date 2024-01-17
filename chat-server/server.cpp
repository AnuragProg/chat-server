#include "server.hpp"
#include <iostream>
#include <string>
#include <memory>
#include <future>
#include <thread>
#include <vector>

ChatServer::ChatServer(int port): 
	acceptor(asio::ip::tcp::acceptor(io, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))){}


void ChatServer::run() {
	auto ts = std::vector<std::thread>();
	// Event loop
	while (true) {
		std::cout << "waiting for connection" << std::endl;
		// Creating new socket conn and accpeting it
		auto socket = asio::ip::tcp::socket(io);
		acceptor.accept(socket);

		try{
			ts.push_back(std::thread(
				[this](
					asio::ip::tcp::socket&& socket
					) {
						accept(std::move(socket));
				},
				std::move(socket)
			));
		} catch (const std::exception& e) {
			std::cout << e.what() << std::endl;
		}
		
		//std::async(
		//	[this](
		//		asio::ip::tcp::socket&& socket
		//	) {
		//		accept(std::move(socket));
		//	},
		//	std::move(socket)
		//);
		std::cout << "user accepted" << std::endl;
	}
}

// Will convert tcp to websocket and push it into the chat server
void ChatServer::accept(
	asio::ip::tcp::socket&& socket
){
	try {
		// elevating connection to websocket
		auto ws = std::make_shared<beast::websocket::stream<asio::ip::tcp::socket>>(std::move(socket));
		ws->accept();
		std::cout << "connection became websocket" << std::endl;

		// inserting the socket in clients list
		std::string client_id = "user" + std::to_string(next_id);
		next_id++; // updating the id

		client_mut.lock();
		clients[client_id] = ws;
		client_mut.unlock();
		std::cout << "client " << client_id << " added to clients!" << std::endl;

		// starting new thread for listening messages
		std::async(
			std::launch::async,
			[this](
				std::string client_id,
				std::shared_ptr<beast::websocket::stream<asio::ip::tcp::socket>> client
			) {
				try {
					while (true) {
						beast::multi_buffer buffer;
						client->read(buffer);
						std::string msg = beast::buffers_to_string(buffer.data());
						fan_out(client_id, msg);
						std::cout << "fanned out the message" << std::endl;
					}
				} catch (const std::exception& e) {
					client_id += " left";
					fan_out(client_id);
				}
			},
			//std::move(client_id), 
			client_id,
			ws
		);
	}catch (const std::exception& e){
		std::cout << e.what() << std::endl;
	}
}

void ChatServer::fan_out(std::string& client_id, std::string& msg) {
	std::string updated_message = client_id + ": " + msg;
	for (auto& entry : clients) {
		if (entry.first == client_id) continue;
		std::cout << "message sent to = " << entry.first << std::endl;
		entry.second->text(true);
		entry.second->write(asio::buffer(updated_message));
	}
}

void ChatServer::fan_out(std::string& msg) {
	for (auto& entry : clients) {
		std::cout << "message sent to = " << entry.first << std::endl;
		entry.second->text(true);
		entry.second->write(asio::buffer(msg));
	}
}



