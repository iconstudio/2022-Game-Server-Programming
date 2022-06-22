#include "pch.hpp"
#include "Session.h"
#include "Asynchron.hpp"
#include "Framework.hpp"
#include "SightSector.hpp"

Session::Session(UINT index, PID id, IOCPFramework& framework)
	: Index(index)
	, myNickname()
	, myInfobox(id)
	, ID(id), Status(SESSION_STATES::NONE), Socket(NULL)
	, myLevel(myInfobox.level)
	, myCategory(myInfobox.myCategory), myType(myInfobox.myType)
	, myHP(myInfobox.hp), myMaxHP(myInfobox.maxhp)
	, myMP(myInfobox.mp), myMaxMP(myInfobox.maxmp)
	, myArmour(myInfobox.amour)
	, myDirection(myInfobox.dir)
	, recvOverlap(OVERLAP_OPS::RECV), recvBuffer(), recvCBuffer(), recvBytes(0)
	, mySightSector(nullptr), myViewList()
	, myFramework(framework)
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

bool Session::TryMoveLT(float distance)
{
	if (8.0f < myPosition[0] - distance)
	{
		myPosition[0] -= distance;
		return true;
	}
	else if (myPosition[0] != 8.0f)
	{
		myPosition[0] = 8.0f;
		return true;
	}
	return false;
}

bool Session::TryMoveRT(float distance)
{
	if (myPosition[0] + distance < WORLD_W - 8.0f)
	{
		myPosition[0] += distance;
		return true;
	}
	else if (myPosition[0] != WORLD_W - 8.0f)
	{
		myPosition[0] = WORLD_W - 8.0f;
		return true;
	}
	return false;
}

bool Session::TryMoveUP(float distance)
{
	if (8.0f < myPosition[1] - distance)
	{
		myPosition[1] -= distance;
		return true;
	}
	else if (myPosition[1] != 8.0f)
	{
		myPosition[1] = 8.0f;
		return true;
	}
	return false;
}

bool Session::TryMoveDW(float distance)
{
	if (myPosition[1] + distance < WORLD_H - 8.0f)
	{
		myPosition[1] += distance;
		return true;
	}
	else if (myPosition[1] != WORLD_H - 8.0f)
	{
		myPosition[1] = WORLD_H - 8.0f;
		return true;
	}
	return false;
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

void Session::ProceedReceived(Asynchron* overlap, DWORD bytes)
{
	std::cout << "ProceedReceived (" << ID << ")" << "\n";
	if (0 == bytes)
	{
		ErrorDisplay("ProceedOperations(bytes=0)");
		return;
	}

	const auto port = myFramework.GetCompletionPort();

	auto& wbuffer = recvBuffer;
	auto& cbuffer = wbuffer.buf;

	const auto sz_min = sizeof(Packet);

	recvBytes += bytes;
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
				if (IsAccepted()) // 잘못된 메시지 받음. 연결 종료.
				{
					Disconnect();
				}
				else // 클라이언트 수용.
				{
					auto result = reinterpret_cast<CSPacketSignIn*>(cbuffer);

					myFramework.ProceedSignIn(this, result);

					//auto asyncer = new Asynchron(OVERLAP_OPS::ACCEPT);
					//asyncer->SetSendBuffer(cbuffer, sz_want);
					//PostQueuedCompletionStatus(port, DWORD(Index), ULONG_PTR(ID), asyncer);
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
				if (pid == ID)
				{
					TryMove(result->myDirection);
				}
				else // 잘못된 메시지 받음.
				{
				}
			}
			break;

			case PACKET_TYPES::CS_ATTACK_NONTARGET:
			{
				auto result = reinterpret_cast<CSPacketAttack*>(cbuffer);

				TryNormalAttack(result->attackDirection);
			}
			break;

			case PACKET_TYPES::CS_ATTACK_TARGET:
			{}
			break;

			case PACKET_TYPES::CS_CHAT:
			{
				//auto result = reinterpret_cast<CSPacketChatMessage*>(cbuffer);

				auto asyncer = new Asynchron(OVERLAP_OPS::ACCEPT);
				asyncer->SetSendBuffer(cbuffer, sz_want);
				PostQueuedCompletionStatus(port, 1, ULONG_PTR(ID), asyncer);
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

void Session::ProceedSent(Asynchron* overlap, DWORD bytes)
{
	if (0 == bytes)
	{
		ErrorDisplay("ProceedSent(bytes=0)");

		delete overlap;
		return;
	}

	std::cout << "ProceedSent (" << ID << ")" << "\n";

	delete overlap;
}

void Session::Cleanup()
{
	closesocket(Socket.load(std::memory_order_seq_cst));

	SetID(-1);
	SetStatus(SESSION_STATES::NONE);
	SetSocket(NULL);
	mySightSector = nullptr;
	myViewList.clear();
}

void Session::Disconnect()
{
	myFramework.Disconnect(ID);
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
	myInfobox.playerID = id;
}

void Session::AddSight(const PID id)
{
	myViewList.insert(id);
}

void Session::RemoveViewOf(const PID id)
{
	//myViewList.unsafe_erase(id);
	myViewList.erase(id);
}

void Session::AssignSight(const concurrent_set<PID>& view)
{
	//myViewList.clear();
	//myViewList = (view);
}

void Session::AssignSight(const std::unordered_set<PID>& view)
{
	myViewList.clear();
	myViewList.insert(view.begin(), view.end());
}

void Session::AssignSight(const std::vector<PID>& view)
{
	myViewList.clear();
	myViewList.insert(view.begin(), view.end());
}

void Session::AssignSight(std::vector<PID>&& view)
{
	const auto&& list = std::forward<std::vector<PID>>(view);
	AssignSight(list);
}

const std::unordered_set<PID>& Session::GetSight() const
{
	return myViewList;
}

std::unordered_set<PID>& Session::GetSight()
{
	return myViewList;
}
/*
const concurrent_set<PID>& Session::GetSight() const
{
	return myViewList;
}

concurrent_set<PID>& Session::GetSight()
{
	return myViewList;
}
*/
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

SESSION_STATES Session::GetStatus() const volatile
{
	return Status.load(std::memory_order_relaxed);
}

PID Session::GetID() const volatile
{
	return ID.load(std::memory_order_relaxed);
}

void Session::ReleaseStatus(SESSION_STATES state)
{
	Status.store(state, std::memory_order_release);
}

void Session::ReleaseID(PID id)
{
	ID.store(id, std::memory_order_release);
	myInfobox.playerID = id;
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
	return (SESSION_STATES::ACCEPTED == Status);
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

void Session::SetPosition(float x, float y)
{
	myPosition[0] = x;
	myPosition[1] = y;
	myInfobox.x = x;
	myInfobox.y = y;
}

void Session::SetPosition(const float(&position)[2])
{
	myInfobox.x = position[0];
	myInfobox.y = position[1];
	*myPosition = *position;
}

void Session::SetPosition(float_pair position)
{
	myPosition[0] = position.first;
	myPosition[1] = position.second;
	myInfobox.x = position.first;
	myInfobox.y = position.second;
}

const float* Session::GetPosition() const
{
	return myPosition;
}

float* Session::GetPosition()
{
	return myPosition;
}

bool Session::CheckCollision(const Session& other) const
{
	return !(other.GetBoundingRight() <= GetBoundingLeft()
		|| other.GetBoundingBottom() <= GetBoundingTop()
		|| GetBoundingRight() < other.GetBoundingLeft()
		|| GetBoundingBottom() < other.GetBoundingTop());
}

bool Session::CheckCollision(const Session* other) const
{
	return !(other->GetBoundingRight() <= GetBoundingLeft()
		|| other->GetBoundingBottom() <= GetBoundingTop()
		|| GetBoundingRight() < other->GetBoundingLeft()
		|| GetBoundingBottom() < other->GetBoundingTop());
}

bool Session::CheckCollision(const RECT& other) const
{
	return false;
}

void Session::SetBoundingBox(const RECT& box)
{
	myBoundingBox = box;
}

void Session::SetBoundingBox(RECT&& box)
{
	myBoundingBox = std::forward<RECT>(box);
}

void Session::SetBoundingBox(long left, long top, long right, long bottom)
{
	myBoundingBox.left = left;
	myBoundingBox.top = top;
	myBoundingBox.right = right;
	myBoundingBox.bottom = bottom;
}

float Session::GetBoundingLeft() const
{
	return myPosition[0] + static_cast<float>(myBoundingBox.left);
}

float Session::GetBoundingTop() const
{
	return myPosition[1] + static_cast<float>(myBoundingBox.top);
}

float Session::GetBoundingRight() const
{
	return myPosition[0] + static_cast<float>(myBoundingBox.right);
}

float Session::GetBoundingBottom() const
{
	return myPosition[1] + static_cast<float>(myBoundingBox.bottom);
}
