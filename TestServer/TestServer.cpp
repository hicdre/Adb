// TestServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>



using asio::ip::tcp;

class session
	: public std::enable_shared_from_this < session >
{
public:
	session(tcp::socket socket)
		: socket_(std::move(socket))
	{
	}

	void start()
	{
		tcp::endpoint pt = socket_.remote_endpoint();
		remote_address_ = pt.address().to_string()
			+ ":" + std::to_string(pt.port());
		

		std::cout
			<< "Connected ["
			<< remote_address_
			<< "]"
			<< std::endl;

		do_read();
	}

private:
	void do_read()
	{
		auto self(shared_from_this());
		socket_.async_read_some(asio::buffer(data_, max_length),
			[this, self](std::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				
				std::cout
					<< "Recieved ["
					<< remote_address_
					<< "] :";
				std::cout.write(data_, length);
				std::cout << std::endl;
				do_write(length);
			}
			else if (ec != asio::error::operation_aborted)
			{
				std::cout
					<< "Disconnected ["
					<< remote_address_
					<< "]"
					<< std::endl;
			}
		});
	}

	void do_write(std::size_t length)
	{
		auto self(shared_from_this());
		asio::async_write(socket_, asio::buffer(data_, length),
			[this, self](std::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				do_read();
			}
		});
	}

	tcp::socket socket_;
	std::string remote_address_;
	enum { max_length = 1024 };
	char data_[max_length];
};

class server
{
public:
	server(asio::io_service& io_service, short port)
		: acceptor_(io_service, tcp::endpoint(asio::ip::address_v4::loopback()
								, port)),
		socket_(io_service)
	{
		do_accept();
	}

private:
	void do_accept()
	{
		acceptor_.async_accept(socket_,
			[this](std::error_code ec)
		{
			if (!ec)
			{
				std::make_shared<session>(std::move(socket_))->start();
			}

			do_accept();
		});
	}

	tcp::acceptor acceptor_;
	tcp::socket socket_;
};

int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		asio::io_service io_service;

		server s(io_service, 2345);

		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}

