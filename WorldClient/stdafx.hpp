#pragma once
#include "targetver.h"
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "Ws2_32.lib")
#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WS2tcpip.h>
#include <MSWSock.h>

#include <string>
#include <iostream>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>
#include <numeric>
#include <concurrent_vector.h>
#include <vector>
#include <array>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <filesystem>
#include <random>

using std::string;
using std::array;
using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::atomic;
using std::atomic_bool;
using std::atomic_int;
using std::atomic_uint;
using std::make_shared;
using std::make_unique;
using std::uniform_int_distribution;
using std::uniform_real_distribution;
using std::default_random_engine;
using std::min;
using std::max;

using int_pair = std::pair<int, int>;
using float_pair = std::pair<float, float>;
using Clock = std::chrono::system_clock::time_point;
using Duration = std::chrono::system_clock::duration;
using Filepath = std::filesystem::path;

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS 일부 CString 생성자는 명시적으로 선언됩니다.
#include <atlbase.h>
#include <atlimage.h>
#include <atlstr.h>

template<typename Type>
using concurrent_vector = Concurrency::concurrent_vector<Type>;
template<typename Type>
using shared_concurrent_vector = concurrent_vector<shared_ptr<Type>>;
template<typename Type>
using weak_concurrent_vector = concurrent_vector<weak_ptr<Type>>;
template<typename Type>
using unique_concurrent_vector = concurrent_vector<unique_ptr<Type>>;

template<typename Type>
using shared_atomic = atomic<shared_ptr<Type>>;
template<typename Type>
using weak_atomic = atomic<weak_ptr<Type>>;

template<typename Type>
using shared_atomic_concurrent_vector = concurrent_vector<shared_atomic<Type>>;
template<typename Type>
using weak_atomic_concurrent_vector = concurrent_vector<weak_atomic<Type>>;

constexpr double PI = 3.141592653589793;

template<typename Ty>
constexpr Ty dcos(Ty value)
{
	return std::cos(value * PI / 180);
}

template<typename Ty>
constexpr Ty dsin(Ty value)
{
	return std::sin(value * PI / 180);
}

template<typename Ty>
constexpr Ty radtodeg(Ty value)
{
	return value / PI * Ty(180.0);
}

template<typename Ty>
constexpr Ty degtorad(Ty value)
{
	return value * PI / Ty(180.0);
}

template<typename Ty>
constexpr Ty lengthdir_x(Ty length, Ty direction)
{
	return std::cos(degtorad<Ty>(direction)) * length;
}

template<typename Ty>
constexpr double lengthdir_y(Ty length, Ty direction)
{
	return -std::sin(degtorad<Ty>(direction)) * length;
}

template<typename Ty>
inline double point_distance(Ty x1, Ty y1, Ty x2, Ty y2)
{
	return std::sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

template<typename Ty>
inline double point_direction(Ty x1, Ty y1, Ty x2, Ty y2)
{
	return radtodeg<Ty>(atan2(y1 - y2, x2 - x1));
}

namespace Render
{
	void transform_set(HDC, XFORM&);
	void transform_set_identity(HDC);
	void transform_set_rotation(HDC);

	void draw_clear(HDC, int width, int height, COLORREF color = 0);
	BOOL draw_rectangle(HDC, int, int, int, int);
	void draw_end(HDC, HGDIOBJ, HGDIOBJ);

	static XFORM transform_identity{ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
}

void ErrorAbort(const wchar_t* title, const int errorcode);
void ErrorDisplay(const wchar_t* title, const int errorcode);
void ErrorAbort(const wchar_t* title);
void ErrorDisplay(const wchar_t* title);
