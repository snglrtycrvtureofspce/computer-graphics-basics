//------------------------------------------------------
//	Файл:		BMPVIEW.CPP
//	Описание:	Демонстрирует работу с растрами
//------------------------------------------------------


#define STRICT
#define WIN32_LEAN_AND_MEAN 

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "BMPVIEW.h"

// Объекты контекста в памяти и шрифт
static HDC h_mem_dc;
static HBITMAP h_mem_bitmap;
static HFONT h_info_font;

// Два цвета для надписей
COLORREF crFontColor1;
COLORREF crFontColor2;

// Имя выводимого растра
static LPCTSTR sz_file_name = TEXT("photo.bmp");

// Ширина и высота экрана
static UINT n_screen_x;
static UINT	n_screen_y;

// Реальные ширина и высота картинки
static UINT	n_bmp_width;
static UINT	n_bmp_height;

// Ширина и высота картинки при выводе
static UINT	n_v_bmp_width;
static UINT	n_v_bmp_height;

// Отступ сверху для надписи
static UINT	n_top_down = 100;

int WINAPI WinMain(HINSTANCE h_inst, HINSTANCE h_prev_inst, LPSTR lpsz_cmd_line, int n_cmd_show)
{
	WNDCLASSEX wnd_class;
	HWND h_wnd;
	MSG msg;

	n_screen_x = GetSystemMetrics(SM_CXSCREEN);
	n_screen_y = GetSystemMetrics(SM_CYSCREEN);

	// Регистрация оконного класса
	wnd_class.cbSize = sizeof(wnd_class);
	wnd_class.style = CS_HREDRAW | CS_VREDRAW;
	wnd_class.lpfnWndProc = wnd_proc;
	wnd_class.cbClsExtra = 0;
	wnd_class.cbWndExtra = 0;
	wnd_class.hInstance = h_inst;
	wnd_class.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	wnd_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wnd_class.hbrBackground = GetStockBrush(BLACK_BRUSH);
	wnd_class.lpszMenuName = nullptr;
	wnd_class.lpszClassName = sz_class_name;
	wnd_class.hIconSm = LoadIcon(nullptr, IDI_WINLOGO);

	RegisterClassEx(&wnd_class);

	// Создание окна на основе класса
	h_wnd = CreateWindowEx(
		WS_EX_LEFT,//Дополнит. стиль окна
		sz_class_name,	//Класс окна
		sz_app_name,	//Текст заголовка
		WS_POPUP,	//Стиль окна
		0, 0,		//Координаты X и Y
		n_screen_x, n_screen_y,//Ширина и высота
		nullptr,		//Дескриптор родит. окна
		nullptr,		//Дескриптор меню
		h_inst,		//Описатель экземпляра
	nullptr);		//Дополнит. данные

	ShowWindow(h_wnd, n_cmd_show);
	UpdateWindow(h_wnd);

	// Главный цикл программы
	while (TRUE)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
			bmp_on_idle(h_wnd);
	}
	return (msg.wParam);
}


/////////////////////////////////////////////////
//	Оконная процедура
////////////////////////////////////////////////

LRESULT CALLBACK wnd_proc(const HWND h_wnd, const UINT msg, const WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		HANDLE_MSG(h_wnd, WM_CREATE, bmp_on_create);
		HANDLE_MSG(h_wnd, WM_DESTROY, bmp_on_destroy);
		HANDLE_MSG(h_wnd, WM_TIMER, bmp_on_timer);
		HANDLE_MSG(h_wnd, WM_KEYDOWN, bmp_on_key);
	default:
		return DefWindowProc(h_wnd, msg, wParam, lParam);
	}
}

/* Обработчики сообщений */

BOOL bmp_on_create(const HWND hwnd, LPCREATESTRUCT lp_create_struct)
{
	// Получаем контекст Рабочего стола
	const HDC h_temp_dc = GetDC(HWND_DESKTOP);

	// Создаём контекст в памяти и растр, совместимые с экраном
	h_mem_dc = CreateCompatibleDC(h_temp_dc);
	h_mem_bitmap = CreateCompatibleBitmap(h_temp_dc, n_screen_x, n_screen_y);

	// Заливаем содержимое контекста черным цветом
	SelectBitmap(h_mem_dc, h_mem_bitmap);
	SelectBrush(h_mem_dc, GetStockBrush(BLACK_BRUSH));
	PatBlt(h_mem_dc, 0, 0, n_screen_x, n_screen_y, PATCOPY);

	// Копируем картинку в контекст памяти

	if (!load_bmp(h_mem_dc, sz_file_name))
	{
		MessageBox(hwnd, L"Не найден файл", L"Ошибка", MB_OK | MB_ICONSTOP);
		return (FALSE);
	}

	// Освобождаем временный контекст
	ReleaseDC(HWND_DESKTOP, h_temp_dc);

	// Создаём шрифт и переменные для цветов шрифта
	h_info_font = CreateFont(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, VARIABLE_PITCH, TEXT("Courier New"));

	crFontColor1 = RGB(255, 0, 0);
	crFontColor2 = RGB(0, 255, 0);

	// Выбираем цвет шрифта
	SetTextColor(h_mem_dc, crFontColor1);

	// Рисуем текст в контексте памяти
	show_text();

	// Устанавливаем таймер
	if (!SetTimer(hwnd, timer_id, timer_rate, nullptr))
		return (FALSE);
	return (TRUE);
}

//-----------------------------------------------------------------

void bmp_on_destroy(const HWND hwnd)
{
	// Убираем после себя
	KillTimer(hwnd, timer_id);
	DeleteDC(h_mem_dc);
	DeleteBitmap(h_mem_bitmap);
	DeleteFont(h_info_font);
	PostQuitMessage(0);
}

//-----------------------------------------------------------------

void bmp_on_timer(HWND hwnd, UINT id)
{
	// Рисуем текст в контексте памяти
	show_text();
}

//-----------------------------------------------------------------

void bmp_on_idle(const HWND hwnd)
{
	// Получаем контекст окна, копируем в него содержимое
	// контекста памяти, освобождаем контекст окна
	const HDC h_window_dc = GetDC(hwnd);
	BitBlt(h_window_dc, 0, 0, n_screen_x, n_screen_y, h_mem_dc, 0, 0, SRCCOPY);
	ReleaseDC(hwnd, h_window_dc);
}

//-----------------------------------------------------------------

void bmp_on_key(const HWND hwnd, const UINT vk, BOOL f_down, int c_repeat, UINT flags)
{
	// При нажатии пробела - прекратить программу
	if (vk == VK_SPACE)
		DestroyWindow(hwnd);
}

//-------------------------------------------------------------------

/* Копирует содержимое файла BMP в контекст устройства */

BOOL load_bmp(const HDC hdc, const LPCTSTR sz_file_name)
{
	DWORD dw_bytes_read;

	BITMAPFILEHEADER bmp_header;


	// Пытаемся открыть файл
	const HANDLE h_file = CreateFile(sz_file_name, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);

	if (h_file == INVALID_HANDLE_VALUE) {
		return (FALSE);
	}

	// Определяем размер данных, которые необходимо загрузить
	const DWORD dw_file_length = GetFileSize(h_file, nullptr);
	const DWORD dw_bmp_size = dw_file_length - sizeof(BITMAPFILEHEADER);

	// Выделяем память и считываем данные
	BYTE* p_bmp = (BYTE*)malloc(dw_bmp_size);
	ReadFile(h_file, &bmp_header, sizeof(bmp_header), &dw_bytes_read, nullptr);
	ReadFile(h_file, (LPVOID)p_bmp, dw_bmp_size, &dw_bytes_read, nullptr);
	CloseHandle(h_file);

	// Инициализируем указатели на информацию о картинке
	// и на графические данные
	BITMAPINFO* p_bmp_info = (BITMAPINFO*)p_bmp;
	const BYTE* p_pixels = p_bmp + bmp_header.bfOffBits - sizeof(BITMAPFILEHEADER);

	// Получаем ширину и высоту картинки
	n_bmp_height = p_bmp_info->bmiHeader.biHeight;
	n_bmp_width = p_bmp_info->bmiHeader.biWidth;

	// Вычисляем ширину и высоту картинки для вывода на экран
	n_v_bmp_height = n_screen_y - n_top_down * 2;
	n_v_bmp_width = (UINT)((double)n_bmp_width * (double)n_v_bmp_height / (double)n_bmp_height);
	if (n_v_bmp_width > n_screen_x)
		n_v_bmp_width = n_screen_x;


	//Устанавливаем режим масштабирования
	SetStretchBltMode(hdc, HALFTONE);

	//Копируем картинку в контекст памяти
	StretchDIBits(hdc, (n_screen_x - n_v_bmp_width) / 2, n_top_down, n_v_bmp_width, n_v_bmp_height, 0, 0, n_bmp_width, n_bmp_height, p_pixels, p_bmp_info, 0, SRCCOPY);

	//Освобождаем память
	free(p_bmp);
	return (TRUE);
}

/* Меняет текущий цвет шрифта и выводит текст в контекст памяти*/
void show_text()
{
	RECT	r_text_rect;
	wchar_t sz_info_text[256];  // Change to wchar_t

	// Определяем текущий цвет шрифта
	if (GetTextColor(h_mem_dc) == crFontColor1)
		SetTextColor(h_mem_dc, crFontColor2);
	else
		SetTextColor(h_mem_dc, crFontColor1);

	// Производим настройки...
	SetBkMode(h_mem_dc, TRANSPARENT);
	const HFONT h_old_font = SelectFont(h_mem_dc, h_info_font);

	// ...и выводим текст.
	SetRect(&r_text_rect, 0, 0, n_screen_x, n_top_down);
	DrawTextW(h_mem_dc, sz_file_name, wcslen(sz_file_name), &r_text_rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER); // Use DrawTextW

	wsprintfW(sz_info_text, L"Зеневич Александр\nReal width-%d, Real height-%d\n Virtual width-%d, Virtual height-%d", n_bmp_width, n_bmp_height, n_v_bmp_width, n_v_bmp_height); // Use wsprintfW

	SetRect(&r_text_rect, 0, n_screen_y - n_top_down, n_screen_x, n_screen_y);
	DrawTextW(h_mem_dc, sz_info_text, wcslen(sz_info_text), &r_text_rect, DT_CENTER);

	SelectFont(h_mem_dc, h_old_font);
}
