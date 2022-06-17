#pragma once
#include <sdkddkver.h>

#include <iostream>
#include <memory>
#include <atomic>
#include <condition_variable>
#include <numeric>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <initializer_list>
#include <array>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <concurrent_vector.h>
#include <boost/asio.hpp>

using std::array;
using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::atomic;
using std::atomic_flag;
using std::atomic_bool;
using std::atomic_int;
using std::atomic_uint;

using int_pair = std::pair<int, int>;
using float_pair = std::pair<float, float>;

using boost::asio::ip::tcp;
using boost::asio::ip::make_address;

using NetService = boost::asio::io_context;
using NetGate = boost::asio::ip::tcp::acceptor;
using NetIPAddress = boost::asio::ip::address;
using NetAddress = boost::asio::ip::tcp::endpoint;
using NetErrorCode = boost::system::error_code;
using Socket = boost::asio::ip::tcp::socket;

using PID = size_t;

template<typename Type>
using shared_concurrent_vector = Concurrency::concurrent_vector<shared_ptr<Type>>;
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


