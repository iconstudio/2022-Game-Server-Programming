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

	LONG width, height;						// 창 크기
	UINT frames;							// 초 당 프레임 수

private:
	HINSTANCE instance;						// 프로세스 인스턴스
	HWND hwindow;							// 창 인스턴스
	WindowProcedure procedure;				// 창 처리기
	WNDCLASSEX properties;					// 창 등록정보
	LPCWSTR title_caption, class_id;		// 창 식별자
	const FLOAT frame_time;					// 프레임 당 시간
};
