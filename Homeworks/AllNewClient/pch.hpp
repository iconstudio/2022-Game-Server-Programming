#pragma once
#include "targetver.h"

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
#include <optional>
#include <memory>
#include <numeric>
#include <concurrent_vector.h>
#include <vector>
#include <array>
#include <queue>
#include <unordered_map>
#include <algorithm>

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

using int_pair = std::pair<int, int>;
using float_pair = std::pair<float, float>;
using Clock = std::chrono::system_clock::time_point;
using Duration = std::chrono::system_clock::duration;

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

template<typename T>
constexpr T dcos(T value)
{
	return std::cos(value * PI / 180);
}

template<typename T>
constexpr T dsin(T value)
{
	return std::sin(value * PI / 180);
}

void ErrorAbort(const wchar_t* title, const int errorcode);
void ErrorDisplay(const wchar_t* title, const int errorcode);
void ErrorAbort(const wchar_t* title);
void ErrorDisplay(const wchar_t* title);
