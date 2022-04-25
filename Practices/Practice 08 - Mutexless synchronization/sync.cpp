#include <iostream>
#include <thread>

volatile int s_data = 0;
volatile bool ready = false;

void Receive();
void Send();

/*
	릴리스 모드에선 버그가 발생한다.
	버그 1: VS의 컴파일러 버그
	버그 2: 컴파일러에서 while을 검사할 때 조건 변수 갱신을 안 한다.
			사실 이건 버그가 아니다! C언어 자체가 멀티스레드 언어가 아니기 때문이다.
*/
int main()
{
	std::thread worker1{ Receive };
	std::thread worker2{ Send };

	worker1.join();
	worker2.join();

	return 0;
}

void Receive()
{
	// 반복문 내용이 비어있으면 릴리스 모드에선 날아가버린다.
	while (!ready)
	{
		// ... vs 버그
		std::this_thread::yield();
	}

	int my_data = s_data;
	std::cout << "Receive: " << my_data << ".\n";
}

void Send()
{
	s_data = 999;
	ready = true;
}
