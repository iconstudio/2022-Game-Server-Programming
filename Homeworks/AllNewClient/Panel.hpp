#pragma once

using WindowProcedure = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);

class Panel
{
public:
	Panel(size_t width, size_t height);
	~Panel();

	bool Initialize(HINSTANCE handle, WNDPROC procedure, LPCWSTR title, LPCWSTR id, INT cmd_show);
	HWND GetHandle() const;
	size_t GetWidth() const;
	size_t GetHeight() const;

	const size_t width, height;

private:
	HINSTANCE instance;						// ���μ��� �ν��Ͻ�
	HWND hwindow;							// â �ν��Ͻ�
	WindowProcedure procedure;				// â ó����
	WNDCLASSEX properties;					// â �������
	LPCWSTR title_caption, class_id;		// â �ĺ���
};

