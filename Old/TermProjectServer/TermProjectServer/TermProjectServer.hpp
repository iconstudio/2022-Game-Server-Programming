#pragma once
#include "pch.hpp"
#include "Session.hpp"
#include "Asynchron.hpp"
#include "Timeline.hpp"
#include "GameObject.hpp"

void Awake();
void BuildSessions();
void BuildPlayers();
void BuildNPCs();
void Start();
void Listen(LPWSAOVERLAPPED overlapped);
void Update();
void ConcurrentWorker();
void ProceedAccept(Asynchron* asynchron);
void ProceedRecv(const PID pid, Asynchron* asynchron, const DWORD recv_bytes);
void ProceedSend(const PID pid, Asynchron* asynchron, const DWORD sent_bytes);
void Release();

void BehaveNPC(PID npc_id);
int GetGridDistance(PID user_id1, PID user_id2);

PID MakeNewbieID();
void Disconnect(const PID who);

shared_ptr<Session> AcquireSession(PID index);
void ReleaseSession(PID index, shared_ptr<Session> handle);
shared_ptr<Session> GetSession(PID index);

SOCKET MakeSocket();
