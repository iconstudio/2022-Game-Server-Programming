#pragma once

#ifndef __PCH__
#define __PCH__

#include "stdafx.hpp"
#include "..\TermProjectServer\TermProjectServer\protocol.h"

constexpr auto SCREEN_WIDTH = 9;
constexpr auto SCREEN_HEIGHT = 9;

constexpr auto TILE_WIDTH = 65;
constexpr auto WINDOW_WIDTH = TILE_WIDTH * SCREEN_WIDTH + 10;   // size of window
constexpr auto WINDOW_HEIGHT = TILE_WIDTH * SCREEN_WIDTH + 10;

class Framework;

class GameSprite;

#endif
