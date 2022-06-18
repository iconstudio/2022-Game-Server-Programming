#include "pch.hpp"
#include "Framework.hpp"

constexpr unsigned short PORT = 8000;
constexpr int THREADS_COUNT = 6;

Framework::Framework()
	: myService(THREADS_COUNT)
	, myAddress(tcp::v4(), PORT)
	, myAcceptor(myService, myAddress)
	, thWorkers(), thTimer(), thDB()
	, myClients(), myUsers()
	, mySprites()
{}

Framework::~Framework()
{}

void Framework::Awake()
{
	thWorkers.reserve(THREADS_COUNT);
}

void Framework::Start()
{
	try
	{
		myAcceptor.async_accept([&](NetErrorCode ec, Socket socket) {
			AcceptCallback(*this, socket, ec);
		});

		for (int i = 0; i < THREADS_COUNT; ++i)
		{
			thWorkers.emplace_back([&]() {
				myService.run();
			});
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

void Framework::Update(float delta_time)
{}

void Framework::Release()
{}

shared_atomic<ClientSlot> Framework::CreateSession(Socket& socket, size_t place)
{
	return atomic(std::make_shared<ClientSlot>());
}

void AcceptCallback(Framework& framework, Socket& socket, const NetErrorCode& ec)
{}

void SendCallback(Framework & framework, PID id, Socket & socket, const NetErrorCode & ec)
{}
