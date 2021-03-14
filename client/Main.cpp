/* This is the client program of client-server, end-to-end encryption chat project
*  we use a specified binary protocol over TCP
*    Author: Arie Gruber
*/

#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <fstream>
#include <vector>

#include "Request.h"
#include "Constants.h"
#include "UsersList.h"
#include "Response.h"
#include "Handler.h"

using boost::asio::ip::tcp;

#define SERVER_INFO_FILE "server.info"

// reads server info from info file. returns string with the info as it saved in file.
std::string readServerInfo() {
	std::ifstream serverFile(SERVER_INFO_FILE);
	std::string serverString;
	std::getline(serverFile, serverString);
	serverFile.close();

	return serverString;
}

// returns the ip from the server info file. throws ServerFileError if there is some error
std::string getIp() {
	std::string serverInfo = readServerInfo();
	return serverInfo.substr(0, serverInfo.find(":"));
}

// returns the port from the server info file. throws ServerFileError if there is some error
std::string getPort() {
	std::string serverInfo = readServerInfo();
	return serverInfo.substr(serverInfo.find(":") + 1, serverInfo.size());
}

int main()
{
	std::string serverIp;
	std::string serverPort;
	// try to load info from server file. if something went wrong, print error message and exit program with error
	try {
		serverIp = getIp();
		serverPort = getPort();
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		exit(1);
	}
	Handler handler;
	
	// main loop
	while (handler.mainIOLoop()) {
		unsigned char responseBuffer[BUFFER_SIZE];
		boost::asio::io_context io_context;
		tcp::socket s(io_context);
		tcp::resolver resolver(io_context);
		try {
			// connect to server
			boost::asio::connect(s, resolver.resolve(serverIp, serverPort));
			// send request
			boost::asio::write(s, boost::asio::buffer(handler.getRequest()));
			// read response header
			boost::asio::read(s, boost::asio::buffer(responseBuffer, RESPONSE_HEADER_SIZE));
			// build response with header
			Response response(responseBuffer);
			// while there is more data from server, read it to the buffer and build the response payload
			for (int i = 0; i < response.getPayloadSIze(); i += BUFFER_SIZE) {
				int remain = response.getPayloadSIze() - i;
				int size = (BUFFER_SIZE > remain) ? remain : BUFFER_SIZE;
				boost::asio::read(s, boost::asio::buffer(responseBuffer, size));
				response.pushPayload(responseBuffer, size);
			}

			handler.handleResponse(&response);
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl;
		}
	}

	return 0;
}