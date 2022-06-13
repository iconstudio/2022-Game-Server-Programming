#include <iostream>
#include <SDKDDKVER.h>
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;

int main()
{
	try
	{
		io_context io_context{};
		ip::tcp::endpoint server_addr(ip::address::from_string("127.0.0.1"), 3500);

		ip::tcp::socket socket(io_context);
		connect(socket, &server_addr);

		for (;;)
		{
			std::string buf;
			boost::system::error_code error;

			std::cout << "Enter Message: ";
			std::getline(std::cin, buf);

			if (0 == buf.size()) break;

			socket.write_some(boost::asio::buffer(buf), error);

			if (error == boost::asio::error::eof)
				break;
			else if (error)
				throw boost::system::system_error(error);

			char reply[1024 + 1];
			size_t len = socket.read_some(boost::asio::buffer(reply, 1024), error);
			if (error == boost::asio::error::eof)
				break;
			else if (error)
				throw boost::system::system_error(error);

			reply[len] = 0;

			std::cout << len << " bytes received: " << reply << endl;
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}
