#pragma once
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WS2tcpip.h>
#include <MSWSock.h>

extern "C" {
#include "include/luaconf.h"
#include "include/lauxlib.h"
#include "include/lualib.h"
#include "include/lua.h"
}

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
#include <concurrent_unordered_map.h>
#include <concurrent_unordered_set.h>
#include <vector>
#include <array>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

using std::string;
using std::array;
using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::optional;
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

template<typename Ty1, typename Ty2>
	requires std::is_arithmetic_v<Ty1>&& std::is_arithmetic_v<Ty2>
constexpr std::pair<Ty1, Ty2> operator+(std::pair<Ty1, Ty2>&& lhs
	, std::pair<Ty1, Ty2>&& rhs)
{
	return std::make_pair<Ty1, Ty2>(
		std::forward<Ty1>(lhs.first) + std::forward<Ty1>(rhs.first)
		, std::forward<Ty2>(lhs.second) + std::forward<Ty2>(rhs.second));
}

template<typename Ty1, typename Ty2>
	requires std::is_arithmetic_v<Ty1>&& std::is_arithmetic_v<Ty2>
constexpr std::pair<Ty1, Ty2> operator+(const std::pair<Ty1, Ty2>& lhs
	, const std::pair<Ty1, Ty2>& rhs)
{
	return std::make_pair<Ty1, Ty2>(lhs.first + rhs.first, lhs.second + rhs.second);
}

template<typename Ty>
	requires std::is_arithmetic_v<Ty>
constexpr std::pair<Ty, Ty> operator+(std::pair<Ty, Ty>&& lhs
	, std::pair<Ty, Ty>&& rhs)
{
	return std::make_pair<Ty, Ty>(
		std::forward<Ty>(lhs.first) + std::forward<Ty>(rhs.first)
		, std::forward<Ty>(lhs.second) + std::forward<Ty>(rhs.second));
}

template<typename Type>
using concurrent_vector = Concurrency::concurrent_vector<Type>;
template<typename Type>
using shared_concurrent_vector = concurrent_vector<shared_ptr<Type>>;
template<typename Type>
using weak_concurrent_vector = concurrent_vector<weak_ptr<Type>>;
template<typename Type>
using unique_concurrent_vector = concurrent_vector<unique_ptr<Type>>;

template<typename KeyType, typename ValueType>
using concurrent_map = Concurrency::concurrent_unordered_map<KeyType, ValueType>;
template<typename KeyType>
using concurrent_set = Concurrency::concurrent_unordered_set<KeyType>;
template<typename KeyType, typename ValueType>
using shared_concurrent_map = concurrent_map<KeyType, shared_ptr<ValueType>>;
template<typename KeyType>
using shared_concurrent_set = Concurrency::concurrent_unordered_set<shared_ptr<KeyType>>;

template<typename Type>
using shared_atomic = atomic<shared_ptr<Type>>;
template<typename Type>
using weak_atomic = atomic<weak_ptr<Type>>;

template<typename Type>
using shared_atomic_concurrent_vector = concurrent_vector<shared_atomic<Type>>;
template<typename Type>
using weak_atomic_concurrent_vector = concurrent_vector<weak_atomic<Type>>;
template<typename KeyType, typename ValueType>
using shared_atomic_concurrent_map = concurrent_map<KeyType, shared_atomic<ValueType>>;
template<typename KeyType>
using shared_atomic_concurrent_set = Concurrency::concurrent_unordered_set<shared_atomic<KeyType>>;

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

void ErrorDisplay(const char* title);

using PID = long long;
