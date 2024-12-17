#pragma once
//------------------------------------------------------
//	Файл:		BMPVIEW.H
//	Описание:	Демонстрирует работу с растрами
//------------------------------------------------------


enum
{
	timer_id = 1,
	timer_rate = 500
};

auto sz_class_name = L"BMP_Window";
auto sz_app_name = L"BMP Application";

// Обработчики сообщений Windows
BOOL bmp_on_create(HWND hwnd, LPCREATESTRUCT lp_create_struct);
void bmp_on_destroy(HWND hwnd);
void bmp_on_timer(HWND hwnd, UINT id);
void bmp_on_key(HWND hwnd, UINT vk, BOOL f_down, int c_repeat, UINT flags);
void bmp_on_idle(HWND hwnd);

// Оконная процедура
LRESULT CALLBACK wnd_proc(HWND, UINT, WPARAM, LPARAM);

// Вспомогательные функции
BOOL load_bmp(HDC hdc, LPCTSTR sz_file_name);
void show_text(void);
