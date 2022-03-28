#pragma once


typedef LRESULT(CALLBACK* WindowProcedure)(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

class WindowsForm
{
public:
	WindowsForm(LONG width, LONG height);
	~WindowsForm();

	BOOL Initialize(HINSTANCE handle, WNDPROC procedure, LPCWSTR title, LPCWSTR id, INT cmd_show);
	LONG GetWidth() const;
	LONG GetHeight() const;

	LONG width, height;						// â ũ��

private:
	HINSTANCE instance;						// ���μ��� �ν��Ͻ�
	HWND hwindow;							// â �ν��Ͻ�
	WindowProcedure procedure;				// â ó����
	WNDCLASSEX properties;					// â �������
	LPCWSTR title_caption, class_id;		// â �ĺ���
};
