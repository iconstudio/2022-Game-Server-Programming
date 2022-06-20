#include "pch.hpp"
#include "Session.h"
#include "Asynchron.hpp"
#include "Framework.hpp"
#include "SightSector.hpp"

Session::Session(UINT index, PID id, IOCPFramework& framework)
	: Index(index), ID(id), myNickname(), Socket(NULL)
	, myFramework(framework)
	, Status(SESSION_STATES::NONE)
	, recvOverlap(OVERLAP_OPS::RECV), recvBuffer(), recvCBuffer(), recvBytes(0)
	, myAvatar(nullptr)
	, mySightSector(nullptr), myViewList()
	, myLuaMachine(nullptr)
{
	ClearOverlap(&recvOverlap);

	recvBuffer.buf = recvCBuffer;
	recvBuffer.len = BUFFSZ;
	ClearRecvBuffer();
}

Session::~Session()
{
	closesocket(Socket);
}

void Session::TryMove(MOVE_TYPES dir)
{
	const auto port = myFramework.GetCompletionPort();
	auto my_id = AcquireID();

	if (IsPlayer(my_id))
	{
		auto asyncer = new Asynchron(OVERLAP_OPS::ENTITY_MOVE);

		PostQueuedCompletionStatus(port, static_cast<DWORD>(dir), ULONG_PTR(my_id), asyncer);
	}
	else
	{
		auto asyncer = new Asynchron(OVERLAP_OPS::ENTITY_MOVE);

		PostQueuedCompletionStatus(port, static_cast<DWORD>(dir), ULONG_PTR(my_id), asyncer);
	}

	ReleaseID(my_id);
}

void Session::TryNormalAttack(MOVE_TYPES dir)
{
	const auto port = myFramework.GetCompletionPort();
	auto my_id = AcquireID();

	if (IsPlayer(my_id))
	{
		// PLAYER_ATTACK
		auto asyncer = new Asynchron(OVERLAP_OPS::PLAYER_ATTACK);

		PostQueuedCompletionStatus(port, static_cast<DWORD>(dir), ULONG_PTR(my_id), asyncer);
	}
	else
	{
		// ENTITY_ATTACK
		auto asyncer = new Asynchron(OVERLAP_OPS::ENTITY_ATTACK);

		PostQueuedCompletionStatus(port, static_cast<DWORD>(dir), ULONG_PTR(my_id), asyncer);
	}

	ReleaseID(my_id);
}

void Session::ProceedReceived(Asynchron* overlap, DWORD byte)
{
	std::cout << "ProceedReceived (" << ID << ")" << "\n";
	auto& wbuffer = recvBuffer;
	auto& cbuffer = wbuffer.buf;

	recvBytes += byte;

	const auto sz_min = sizeof(Packet);
	while (sz_min <= recvBytes)
	{
		auto packet = reinterpret_cast<Packet*>(cbuffer); // 클라이언트 → 서버
		auto sz_want = packet->Size;
		auto type = packet->Type;
		auto pid = packet->playerID;

		if (recvBytes < sz_want)
		{
			auto lack = sz_want - recvBytes;
			std::cout << "클라이언트 " << ID << "에게 받아온 정보가 "
				<< lack << " 만큼 모자라서 다시 수신합니다.\n";
			break;
		}

		switch (type)
		{
			case PACKET_TYPES::CS_SIGNIN:
			{
				auto result = reinterpret_cast<CSPacketSignIn*>(cbuffer);

				if (IsAccepted()) // 잘못된 메시지 받음. 연결 종료.
				{
					Disconnect();
				}
				else // 클라이언트 수용.
				{
					myNickname = result->Nickname;
					std::cout << ID << "'s Nickname: " << myNickname << ".\n";

					myAvatar = std::make_shared<PlayerCharacter>(ID, 100.0f, 100.0f);

					myFramework.ConnectFrom(Index);
				}
			}
			break;

			case PACKET_TYPES::CS_SIGNOUT:
			{
				if (pid == ID && IsAccepted())
				{
					Disconnect();
					return;
				}
				else // 잘못된 메시지 받음.
				{
				}
			}
			break;

			case PACKET_TYPES::CS_MOVE:
			{
				auto result = reinterpret_cast<CSPacketMove*>(cbuffer);
				auto avatar = AcquireAvatar();

				if (pid == ID && avatar)
				{
					TryMove(result->myDirection);
				}
				else // 잘못된 메시지 받음.
				{
				}

				ReleaseAvatar(avatar);
			}
			break;

			case PACKET_TYPES::CS_ATTACK_NONTARGET:
			{
				auto result = reinterpret_cast<CSPacketAttack*>(cbuffer);
				auto avatar = AcquireAvatar();

				if (avatar)
				{
					TryNormalAttack(result->attackDirection);
				}

				ReleaseAvatar(avatar);
			}
			break;

			default:
			{
				ClearRecvBuffer();
				ClearOverlap(overlap); // recvOverlap
				ErrorDisplay("ProceedReceived: 잘못된 패킷 받음");

				Disconnect();
				return;
			}
			break;
		}

		recvBytes -= sz_want;
		if (0 < recvBytes)
		{
			MoveStream(cbuffer, sz_want, BUFFSZ);
		}
	}

	// 아무거나 다 받는다.
	int result = RecvStream(recvBytes);
	if (SOCKET_ERROR == result)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorDisplay("ProceedReceived → RecvStream()");
			Disconnect();
			return;
		}
	}
}

void Session::ProceedSent(Asynchron* overlap, DWORD byte)
{
	if (0 == byte)
	{
		if (WSA_IO_PENDING != WSAGetLastError())
		{
			ErrorDisplay("ProceedSent()");
			Disconnect();
		}
	}

	std::cout << "ProceedSent (" << ID << ")" << "\n";
	auto& sz_send = overlap->sendSize;
	auto& tr_send = overlap->sendSzWant;

	delete overlap;
	//ClearOverlap(overlap);
}


void Session::SetStatus(SESSION_STATES state)
{
	Status.store(state, std::memory_order_relaxed);
}

void Session::SetSocket(SOCKET sock)
{
	Socket.store(sock, std::memory_order_relaxed);
}

void Session::SetID(const PID id)
{
	ID.store(id, std::memory_order_relaxed);
}

void Session::SetAvatar(shared_ptr<GameObject> handle)
{
	myAvatar.store(handle, std::memory_order_relaxed);
}

void Session::AddSight(const PID id)
{
	myViewList.insert(id);
}

void Session::RemoveSight(const PID id)
{
	myViewList.unsafe_erase(id);
}

void Session::AssignSight(const concurrent_set<PID>& view)
{
	myViewList.clear();
	myViewList = (view);
}

void Session::AssignSight(const std::unordered_set<PID>& view)
{
	myViewList.clear();
	myViewList.insert(view.begin(), view.end());
}

void Session::AssignSight(const std::vector<PID>& view)
{
	myViewList.clear();
	std::for_each(view.begin(), view.end(), [&](const PID id) {
		myViewList.insert(id);
	});
}

void Session::AssignSight(std::vector<PID>&& view)
{
	const auto&& list = std::forward<std::vector<PID>>(view);
	AssignSight(list);
}

const concurrent_set<PID>& Session::GetSight() const
{
	return myViewList;
}

concurrent_set<PID>& Session::GetSight()
{
	return myViewList;
}

std::unordered_set<PID> Session::GetLocalSight() const
{
	return std::unordered_set<PID>(myViewList.cbegin(), myViewList.cend());
}

SESSION_STATES Session::AcquireStatus() const volatile
{
	return Status.load(std::memory_order_acquire);
}

PID Session::AcquireID() const volatile
{
	return ID.load(std::memory_order_acquire);
}

shared_ptr<GameObject> Session::AcquireAvatar()
{
	return myAvatar.load(std::memory_order_acquire);
}

SESSION_STATES Session::GetStatus() const volatile
{
	return Status.load(std::memory_order_relaxed);
}

PID Session::GetID() const volatile
{
	return ID.load(std::memory_order_relaxed);
}

shared_ptr<GameObject> Session::GetAvatar()
{
	return myAvatar.load(std::memory_order_relaxed);
}

void Session::ReleaseStatus(SESSION_STATES state)
{
	Status.store(state, std::memory_order_release);
}

void Session::ReleaseID(PID id)
{
	ID.store(id, std::memory_order_release);
}

void Session::ReleaseAvatar(shared_ptr<GameObject> handle)
{
	myAvatar.store(handle, std::memory_order_release);
}

void Session::Cleanup()
{
	SetID(-1);
	SetStatus(SESSION_STATES::NONE);
	SetSocket(NULL);
	mySightSector = nullptr;
	myViewList.clear();

	closesocket(Socket.load(std::memory_order_seq_cst));

	myAvatar.load(std::memory_order_seq_cst).reset();
}

void Session::Disconnect()
{
	myFramework.Disconnect(ID);
}

bool Session::IsConnected() const volatile
{
	return (SESSION_STATES::CONNECTED == Status || SESSION_STATES::ACCEPTED == Status);
}

bool Session::IsDisconnected() const volatile
{
	return (SESSION_STATES::NONE == Status);
}

bool Session::IsAccepted() const volatile
{
	return (SESSION_STATES::CONNECTED == Status);
}
int Session::RecvStream(DWORD size, DWORD begin_bytes)
{
	recvBuffer.buf = recvCBuffer + begin_bytes;
	recvBuffer.len = size - begin_bytes;

	return Recv(0);
}

int Session::RecvStream(DWORD begin_bytes)
{
	return RecvStream(BUFFSZ, begin_bytes);
}

void Session::ClearRecvBuffer()
{
	recvBytes = 0;
	ZeroMemory(recvCBuffer, sizeof(recvCBuffer));
}

int Session::Recv(DWORD flags)
{
	if (!recvBuffer.buf) return 0;

	return WSARecv(Socket, &recvBuffer, 1, 0, &flags, &recvOverlap, NULL);
}

int Session::Send(LPWSABUF datas, UINT count, LPWSAOVERLAPPED overlap)
{
	if (!datas || !overlap) return 0;

	return WSASend(Socket, datas, count, NULL, 0, overlap, NULL);
}

void Session::MoveStream(CHAR*& buffer, DWORD position, DWORD max_size)
{
	MoveMemory(buffer, (buffer + position), max_size - position);
	ZeroMemory(buffer + max_size - position, position);
}

void Session::SetSightArea(const shared_ptr<SightSector>& sector)
{
	mySightSector = sector;
}

void Session::SetSightArea(shared_ptr<SightSector>&& sector)
{
	mySightSector = std::forward<shared_ptr<SightSector>>(sector);
}

const shared_ptr<SightSector>& Session::GetSightArea() const
{
	return mySightSector;
}

shared_ptr<SightSector>& Session::GetSightArea()
{
	return mySightSector;
}
