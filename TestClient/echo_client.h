#pragma once
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "asio.hpp"

enum { max_length = 1024 };

class adbp
{
public:
	typedef asio::ip::basic_endpoint<adbp> endpoint;

	/// Construct to represent the IPv4 TCP protocol.
	static adbp v4()
	{
		return adbp(ASIO_OS_DEF(AF_INET));
	}

	/// Construct to represent the IPv6 TCP protocol.
	static adbp v6()
	{
		return adbp(ASIO_OS_DEF(AF_INET6));
	}

	/// Obtain an identifier for the type of the protocol.
	int type() const
	{
		return ASIO_OS_DEF(SOCK_STREAM);
	}

	/// Obtain an identifier for the protocol.
	int protocol() const
	{
		return ASIO_OS_DEF(AF_UNSPEC);
	}

	/// Obtain an identifier for the protocol family.
	int family() const
	{
		return family_;
	}

	/// The TCP socket type.
	typedef asio::basic_stream_socket<adbp> socket;

	/// The TCP acceptor type.
	typedef asio::basic_socket_acceptor<adbp> acceptor;

	/// The TCP resolver type.
	typedef asio::ip::basic_resolver<adbp> resolver;


	/// Compare two protocols for equality.
	friend bool operator==(const adbp& p1, const adbp& p2)
	{
		return p1.family_ == p2.family_;
	}

	/// Compare two protocols for inequality.
	friend bool operator!=(const adbp& p1, const adbp& p2)
	{
		return p1.family_ != p2.family_;
	}

private:
	// Construct with a specific family.
	explicit adbp(int protocol_family)
		: family_(protocol_family)
	{
	}

	int family_;
};

int RunEchoClient(unsigned short port)
{
	try
	{
		asio::io_service io_service;
#if 0
		adbp::socket s(io_service);
		adbp::endpoint ep(
			asio::ip::address(asio::ip::address_v4::loopback()), port);
#else
		asio::ip::tcp::socket s(io_service);
		asio::ip::tcp::endpoint ep(
			asio::ip::address(asio::ip::address_v4::loopback()), port);
#endif
		

		while (true)
		{
			std::cout << "Enter message: ";
			char request[max_length];
			char tmp[5] = { 0 };
			std::cin.getline(request, max_length);
			//char* request = "host:version";

			if (!strncmp(request, "quit", 4))
				break;

			s.connect(ep);

			size_t request_length = std::strlen(request);
			_snprintf(tmp, sizeof tmp, "%04x", request_length);
			asio::write(s, asio::buffer(tmp, 4));
			asio::write(s, asio::buffer(request, request_length));

			char status[5] = { 0 };
			asio::read(s, asio::buffer(status, 4));

			char buffer_len[5] = { 0 };
			asio::read(s, asio::buffer(buffer_len, 4));
			size_t reply_length = strtoul(buffer_len, 0, 16);
			char reply[max_length];
			reply_length = asio::read(s,
				asio::buffer(reply, reply_length));
			std::cout << "Reply is: ";
			std::cout.write(reply, reply_length);
			std::cout << "\n";

			s.close();
		}

	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;

}