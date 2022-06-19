#include "pch.hpp"
#include "Framework.hpp"
#include "GameObject.hpp"
#include "GameSprite.hpp"
#include "Session.hpp"

sf::TcpSocket serverEntry{};

Framework::Framework(const char* title, unsigned width, unsigned height)
	: myID(0), mySession(nullptr), myAvatar(nullptr)
	, everyInstances(), everyPlayers(), everyNPCs(), everySprites()
	, view_x(0.0f), view_y(0.0f)
	, clientTitle(title), clientW(width), clientH(height)
	, clientPanel(nullptr)
	, recvBuffer(), recvSize(0), savedBuffer(), savedSize(0)
{}

Framework::~Framework()
{}

void Framework::Awake()
{
	clientPanel = new sf::RenderWindow(sf::VideoMode(clientW, clientH), clientTitle);

	if (clientPanel->isOpen())
	{
		clientPanel->setFramerateLimit(100);

		std::cout << "Ŭ���̾�Ʈ ����\n";
	}
	else
	{
		throw "������ Ŭ���̾�Ʈ�� ������ �� �����ϴ�!";
	}

	if (!defaultFont.loadFromFile("cour.ttf"))
	{
		std::cout << "��Ʈ�� �ҷ��� �� �����ϴ�!\n";

		exit(-1);
	}
}

void Framework::Start(const char* ip_address, unsigned short port_number)
{
	auto status = serverEntry.connect(ip_address, port_number);

	if (sf::Socket::Done != status)
	{
		throw "������ ������ �� �����ϴ�!";
	}

	serverEntry.setBlocking(false);
	std::cout << "���� ���� ����";

	SendPacket(CS_LOGIN_PACKET{ "TEMP" });

	//CallbackNetwork(*this);

	//auto& self = *this;
	//std::thread(CallbackNetwork, self);
	//std::thread(CallbackRender, self);
	
	//clientWorker = 
	//clientRenderer = 
}

void Framework::Update()
{
	sf::Event window_event{};

	while (IsOpened())
	{
		while (clientPanel->pollEvent(window_event))
		{
			if (window_event.type == sf::Event::Closed)
			{
				Close();
				break;
			}

			if (window_event.type == sf::Event::KeyPressed)
			{
				MOVE_TYPES direction = MOVE_TYPES::NONE;

				switch (window_event.key.code)
				{
					case sf::Keyboard::Left:
					{
						direction = MOVE_TYPES::LEFT;
					}
					break;

					case sf::Keyboard::Right:
					{
						direction = MOVE_TYPES::RIGHT;
					}
					break;

					case sf::Keyboard::Up:
					{
						direction = MOVE_TYPES::UP;
					}
					break;

					case sf::Keyboard::Down:
					{
						direction = MOVE_TYPES::DOWN;
					}
					break;

					case sf::Keyboard::Escape:
					{
						Close();
					}
					break;
				}

				if (MOVE_TYPES::NONE != direction)
				{
					SendPacket(CS_MOVE_PACKET{ direction });
				}
			}
		}

		CallbackNetwork(*this);
		CallbackRender(*this);
	}

	//clientWorker.join();
	//clientRenderer.join();
}

void Framework::PrepareRendering()
{
	clientPanel->clear();
}

void Framework::Render()
{
	PrepareRendering();

	for (auto& pl : everyInstances)
	{
		pl.second->Render(*clientPanel, view_x, view_y);
	}

	clientPanel->display();
}

void Framework::Close()
{
	clientPanel->close();
}

void Framework::Release()
{
	if (clientPanel->isOpen())
	{
		clientPanel->close();
	}
}

shared_ptr<Session> Framework::CreatePlayer(PID id)
{
	return everyPlayers.insert(CreateSession(myID, true)).first->second;
}

shared_ptr<Session> Framework::CreateNPC(PID id)
{
	return everyNPCs.insert(CreateSession(myID, false)).first->second;
}

std::pair<PID, shared_ptr<Session>> Framework::CreateSession(PID id, bool is_player)
{
	return std::make_pair(id, make_shared<Session>(id, is_player));
}

void Framework::RemoveSession(PID id)
{
	auto it = everyPlayers.find(id);
	if (everyPlayers.end() != it)
	{
		everyPlayers.erase(it);
	}
}

sf::Socket::Status Framework::TryReceive(const size_t buffer_limit)
{
	return serverEntry.receive(recvBuffer, buffer_limit, recvSize);
}

void Framework::ProcessStream()
{
	if (!recvBuffer)
	{
		Release();

		throw "���� ����!";
	}

	unsigned char* ptr = recvBuffer;

	static size_t wanted_size_min = 0;

	while (0 != recvSize)
	{
		if (0 == wanted_size_min)
		{
			wanted_size_min = static_cast<size_t>(ptr[0]); // packet.size
		}

		const auto got_size = recvSize + savedSize;
		if (wanted_size_min <= got_size)
		{
			memcpy(savedBuffer + savedSize, ptr, wanted_size_min - savedSize);

			ProcessPacket(savedBuffer);

			ptr += wanted_size_min - savedSize;
			recvSize -= wanted_size_min - savedSize;

			wanted_size_min = 0;
			savedSize = 0;
		}
		else
		{
			// ó�� �� �� ��Ŷ�� ��� ���� ������ ����.
			memcpy(savedBuffer + savedSize, ptr, recvSize);
			savedSize += recvSize;

			recvSize = 0;
			break;
		}
	}
}

void Framework::ProcessPacket(const void* handle)
{
	const auto view = reinterpret_cast<const Packet*>(handle);
	const auto pk_type = view->myType;
	const auto pk_sz = view->mySize;

	switch (pk_type)
	{
		case PACKET_TYPES::SC_LOGIN_OK:
		{
			auto packet = reinterpret_cast<const SC_LOGIN_OK_PACKET*>(handle);

			const auto users_count = packet->usersCount;
			const auto users_limit = packet->usersLimit;
			std::cout << "������ ���� ��: " << users_count << "/" << users_limit << "\n";

			// ���� �����
			auto session = CreatePlayer(myID);

			string a_name = "P";
			a_name += std::to_string(myID);
			session->set_name(a_name.c_str());

			// �ƹ�Ÿ �����
			auto avatar = make_shared<GameObject>(session, GetSprite("sPiece"));
			avatar->Jump(packet->x, packet->y);
			avatar->show();

			// ���ǿ� �ƹ�Ÿ �Ҵ�
			session->myCharacter = avatar;

			// ���� ����
			view_x = packet->x - 4;
			view_y = packet->y - 4;

			// ����
			myID = packet->myID;
			mySession = session;
			myAvatar = avatar;
		}
		break;

		case PACKET_TYPES::SC_ADD_OBJECT:
		{
			auto packet = reinterpret_cast<const SC_ADD_OBJECT_PACKET*>(handle);

			const auto id = packet->myID;

			if (IsPlayer(id))
			{
				if (0 != everyPlayers.count(id))
				{
					std::cout << "Player " << id << " already exists.\n";
					break;
				}

				auto& new_place = everyPlayers[id];
				auto new_session = CreatePlayer(id);

				new_session->set_name(packet->myNickname);

				// ����, ���� �� ��������Ʈ ã��
				const auto sprite = GetSprite("sPiece");

				// �ƹ�Ÿ �����
				auto avatar = make_shared<GameObject>(new_session, sprite);
				avatar->Jump(packet->x, packet->y);
				avatar->show();

				// ���ǿ� �ƹ�Ÿ �Ҵ�
				new_session->myCharacter = avatar;
			}
			else if (IsNPC(id))
			{
				if (0 != everyNPCs.count(id))
				{
					std::cout << "NPC " << id << " already exists.\n";
					break;
				}

				auto& new_place = everyNPCs[id];
				auto new_session = CreateNPC(id);
				new_session->set_name(packet->myNickname);

				// ���� �� ��������Ʈ ã��
				const auto sprite = GetSprite("sPiece");

				// �ƹ�Ÿ �����
				auto avatar = make_shared<GameObject>(new_session, sprite);
				avatar->Jump(packet->x, packet->y);
				avatar->show();

				// ���ǿ� �ƹ�Ÿ �Ҵ�
				new_session->myCharacter = avatar;
			}
		}
		break;

		case PACKET_TYPES::SC_MOVE_OBJECT:
		{
			auto my_packet = reinterpret_cast<const SC_MOVE_OBJECT_PACKET*>(handle);

			const auto other_id = my_packet->myID;

			if (other_id == myID)
			{
				myAvatar->Jump(my_packet->x, my_packet->y);
				view_x = my_packet->x - 4;
				view_y = my_packet->y - 4;
			}
			else if (IsPlayer(other_id))
			{
				auto it = everyPlayers.find(other_id);
				if (everyPlayers.end() == it)
				{
					std::cout << "Player " << other_id << " does not exists.\n";

					break;
				}

				auto& player = it->second;

				auto& character = player->myCharacter;
				character->Jump(my_packet->x, my_packet->y);
			}
			else
			{
				auto it = everyNPCs.find(other_id);
				if (everyNPCs.end() == it)
				{
					std::cout << "NPC " << other_id << " does not exists.\n";

					break;
				}

				auto& npc = it->second;

				auto& character = npc->myCharacter;
				character->Jump(my_packet->x, my_packet->y);
			}
			break;
		}

		case PACKET_TYPES::SC_REMOVE_OBJECT:
		{
			auto my_packet = reinterpret_cast<const SC_REMOVE_OBJECT_PACKET*>(handle);

			const auto other_id = my_packet->targetID;
			if (other_id == myID)
			{
				myAvatar->hide();
			}
			else if (IsPlayer(other_id))
			{
				auto it = everyPlayers.find(other_id);
				if (everyPlayers.end() == it)
				{
					std::cout << "Player " << other_id << " does not exists.\n";

					break;
				}

				auto& player = it->second;

				auto& character = player->myCharacter;
				if (character)
				{
					character->hide();
				}
				else
				{
					throw "�÷��̾��� ĳ���Ͱ� �������� �ʽ��ϴ�!";
				}

				everyPlayers.erase(it);
			}
			else
			{
				auto it = everyNPCs.find(other_id);
				if (everyNPCs.end() == it)
				{
					std::cout << "NPC " << other_id << " does not exists.\n";
					break;
				}

				auto& npc = it->second;

				auto& character = npc->myCharacter;
				if (character)
				{
					character->hide();
				}
				else
				{
					throw "NPC�� ĳ���Ͱ� �������� �ʽ��ϴ�!";
				}

				everyNPCs.erase(it);
			}
		}
		break;

		default:
		{
			printf("�� �� ���� ��Ŷ [%d] (size: [%u])\n", pk_type, pk_sz);
		}
		break;
	}
}

void CallbackNetwork(Framework& framework)
{
	//while (framework.IsOpened())
	{
		const auto result = framework.TryReceive(BUF_SIZE);
		switch (result)
		{
			case sf::Socket::Error:
			{
				throw "���� ����!\n";
			}
			break;

			case sf::Socket::Disconnected:
			{
				std::cout << "���� ���� ����\n";

				framework.Close();
			}
			break;

			case sf::Socket::NotReady: // ��Ŷ ����
			{
				framework.ProcessStream();
			}
			break;

			default:
			{}
			break;
		}
	}
}

void CallbackRender(Framework& framework)
{
	//while (framework.IsOpened())
	{
		framework.Render();

		std::this_thread::sleep_for(Duration(10));
	}
}

bool Framework::IsOpened() const
{
	return clientPanel->isOpen();
}

void Framework::SendPacket(const Packet& packet)
{
	auto& instance = const_cast<Packet&>(packet);
	const auto void_handle = reinterpret_cast<void*>(&instance);
	const auto sz = packet.mySize;

	SendPacket(void_handle, sz);
}

void Framework::SendPacket(Packet&& packet)
{
	auto instance = const_cast<Packet&&>(std::forward<Packet>(packet));
	const auto void_handle = reinterpret_cast<void*>(&instance);
	const auto sz = std::forward<unsigned char>(packet.mySize);

	SendPacket(void_handle, sz);
}

void Framework::SendPacket(const void* ptr, size_t size)
{
	auto handle = const_cast<void*>(ptr);

	size_t sent = 0;
	serverEntry.send(ptr, size, sent);
}

shared_ptr<GameSprite> Framework::CreateSprite(const Filepath& path, float ox, float oy)
{
	return make_shared<GameSprite>(path, ox, oy);
}

shared_ptr<GameSprite> Framework::CreateSprite(const Filepath& path, float ox, float oy, int tx, int ty, int w, int h)
{
	return make_shared<GameSprite>(path, ox, oy, tx, ty, w, h);
}

shared_ptr<GameSprite> Framework::AddSprite(const char* name, shared_ptr<GameSprite> handle)
{
	return everySprites.try_emplace(name, handle).first->second;
}

shared_ptr<GameSprite> Framework::LoadSprite(const char* name, const Filepath& path, float ox, float oy)
{
	return AddSprite(name, CreateSprite(path, ox, oy));
}

shared_ptr<GameSprite> Framework::LoadSprite(const char* name, const Filepath& path, float ox, float oy, int tx, int ty, int w, int h)
{
	return AddSprite(name, CreateSprite(path, ox, oy, tx, ty, w, h));
}

shared_ptr<GameSprite> Framework::GetSprite(const char* name) const
{
	return everySprites.at(name);
}
