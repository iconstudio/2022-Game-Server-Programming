#include <iostream>
#include <sdkddkver.h>
#include <unordered_map>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using namespace std;
constexpr int PORT = 3500;

class session;
unordered_map <int, session> g_clients;

int g_client_id = 0;
void accept_callback(boost::system::error_code ec, tcp::socket& socket, tcp::acceptor& my_acceptor);

int main(int argc, char* argv[])
{
	try
	{
		boost::asio::io_context io_context{};
		tcp::acceptor my_acceptor{ io_context, tcp::endpoint(tcp::v4(), PORT) };

		my_acceptor.async_accept([&my_acceptor](boost::system::error_code ec, tcp::socket socket) {
			accept_callback(ec, socket, my_acceptor); });

		io_context.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}

class session
{
	int my_id;

	tcp::socket socket_;

	enum { max_length = 1024 };
	char data_[max_length];

public:
	session() : socket_(nullptr)
	{
		cout << "Session Creation Error.\n";
	}
	// 소켓은 복사가 불가능하다!

	session(tcp::socket socket, int id) : socket_(std::move(socket)), my_id(id)
	{
		do_read();
	}

	void do_read()
	{
		socket_.async_read_some(boost::asio::buffer(data_, max_length)
			, [this](boost::system::error_code ec, std::size_t length) {
			if (ec)
			{
				g_clients.erase(my_id);
			}
			else
			{
				g_clients[my_id].do_write(length);
			}
		});
	}

	void do_write(std::size_t length)
	{
		boost::asio::async_write(socket_, boost::asio::buffer(data_, length)
			, [this](boost::system::error_code ec, std::size_t /*length*/) {
			if (!ec)
			{
				g_clients[my_id].do_read();
			}
			else
			{
				g_clients.erase(my_id);
			}
		});
	}
};

void accept_callback(boost::system::error_code ec, tcp::socket& socket, tcp::acceptor& my_acceptor)
{
	g_clients.try_emplace(g_client_id, move(socket), g_client_id);
	g_client_id++;

	my_acceptor.async_accept([&my_acceptor](boost::system::error_code ec, tcp::socket socket) {
		accept_callback(ec, socket, my_acceptor);
	});
}
