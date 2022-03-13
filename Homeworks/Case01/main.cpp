#include "stdafx.h"
#include "main.h"
#include "WindowsForm.h"
#include "Framework.h"

Framework framework{};

int main()
{
	framework.Start();
}

const int Framework::BOARD_W = framework.CELL_W * framework.CELLS_CNT_H;
const int Framework::BOARD_H = framework.CELL_H * framework.CELLS_CNT_V;
const int Framework::BOARD_X = (WND_SZ_W - BOARD_W) * 0.5;
const int Framework::BOARD_Y = (WND_SZ_H - BOARD_H - 20) * 0.5;
