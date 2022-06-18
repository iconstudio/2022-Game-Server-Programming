#include "pch.hpp"
#include "Framework.hpp"

Framework serverFramework{};

int main()
{
	std::cout << "Awakening server.\n";
	serverFramework.Awake();

	std::cout << "Starting server.\n";
	serverFramework.Start();

	std::cout << "Server is ended.\n";
}
