#include <iostream>
#include <thread>

int s_data = 0;
bool ready = false;

void Receive();
void Send();

/*
	������ ��忡�� ���װ� �߻��Ѵ�.
	���� 1: VS�� �����Ϸ� ����
	���� 2: �����Ϸ����� while�� �˻��� �� ���� ���� ������ �� �Ѵ�.
			��� �̰� ���װ� �ƴϴ�! C��� ��ü�� ��Ƽ������ �� �ƴϱ� �����̴�.
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
	// �ݺ��� ������ ��������� ������ ��忡�� ���ư�������.
	while (!ready)
	{
		// ... vs ����
		std::this_thread::yield();
	}

	std::cout << "Receive: " << s_data << ".\n";
}

void Send()
{
	s_data = 999;
	ready = true;
}
