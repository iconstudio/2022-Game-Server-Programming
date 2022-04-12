#pragma once
#include <thread>
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <ratio>

void Worker(unsigned int id);
void SingleAddWorker();
void MassiveAddWorker();
void SafeAddWorker();
