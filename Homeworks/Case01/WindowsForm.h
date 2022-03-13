#pragma once


typedef LRESULT(CALLBACK* WindowProcedure)(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

class WindowsForm
{
public:
	WindowsForm(LONG width, LONG height, UINT framerate);
	~WindowsForm();

	BOOL Initialize(HINSTANCE handle, WNDPROC procedure, LPCWSTR title, LPCWSTR id, INT cmd_show);
	LONG GetWidth() const;
	LONG GetHeight() const;
	float GetFrametime() const;

	LONG width, height;						// â ũ��
	UINT frames;							// �� �� ������ ��

private:
	HINSTANCE instance;						// ���μ��� �ν��Ͻ�
	HWND hwindow;							// â �ν��Ͻ�
	WindowProcedure procedure;				// â ó����
	WNDCLASSEX properties;					// â �������
	LPCWSTR title_caption, class_id;		// â �ĺ���
	const FLOAT frame_time;					// ������ �� �ð�
};
