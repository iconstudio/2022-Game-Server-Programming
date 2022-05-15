#pragma once

class Asynchron : public WSAOVERLAPPED
{};

void ClearOverlap(WSAOVERLAPPED* overlap);
void ClearOverlap(Asynchron* overlap);
