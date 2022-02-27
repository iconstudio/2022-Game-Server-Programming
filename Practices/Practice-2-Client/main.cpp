#include "pch.hpp"
#include "main.hpp"

#include "Network.hpp"

NetworkServer server;

int main() {
	server.Start();
	server.SetActiveAccept(true);

	return 0;
}
