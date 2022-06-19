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
	HINSTANCE instance;						// 프로세스 인스턴스
	HWND hwindow;							// 창 인스턴스
	WindowProcedure procedure;				// 창 처리기
	WNDCLASSEX properties;					// 창 등록정보
	LPCWSTR title_caption, class_id;		// 창 식별자
};

