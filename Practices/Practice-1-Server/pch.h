#ifndef PCH_H
#define PCH_H
#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <Windows.h>

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <span>
#include <chrono>
#include <thread>
#include <mutex>
#include <future>
#include <exception>
#include <memory>
#include <fstream>

using std::cout;
using std::cin;
using std::cerr;
using std::vector;
using std::map;
using std::thread;
using std::mutex;
using std::future;
using std::promise;
using std::shared_ptr;
using std::unique_ptr;
using std::exception;
using std::make_shared;
using std::make_unique;
using std::begin;
using std::end;
using std::cbegin;
using std::cend;
using std::make_exception_ptr;
using std::chrono::steady_clock;
using std::chrono::system_clock;

#endif //PCH_H
