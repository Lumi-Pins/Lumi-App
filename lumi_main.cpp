/*
 * File: lumi_main.cpp
 * Created: 01/25/19
 * Authors: Zelin Liu
 * Content: This is the main file for Lumi-pins app.
 *
 *
 * */

#include <windows.h>
#include <stdio.h>
#include "BoardData.h"

// Macro
#define NUMBER_OF_ROWS 15
#define NUMBER_OF_COLS 15
#define BOARD_WIDTH 750
#define BOARD_HEIGHT 750
#define BOARD_BACKGROUND_COLOR RGB(233, 233, 233)
#define CIRCLE_RATIO 0.80

// Global variables
BoardData bd(NUMBER_OF_ROWS, NUMBER_OF_COLS);

// Forward Declarations
LRESULT CALLBACK LumiWndProc(HWND, UINT, WPARAM, LPARAM);
void CloseProgram(HWND, bool);
bool Draw_Default(HWND);

/*
 * The main function for Windows application
 */
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow) {

//  MessageBox(NULL, "Error", "Lumi-Pins", MB_OK);

	const char CLASS_NAME[]  = "class0";

	// create a window class
	WNDCLASSEXA  wc0 = { };
	wc0.cbSize = sizeof(WNDCLASSEX);	// default size
	wc0.lpfnWndProc = LumiWndProc;	// callback process
	wc0.hInstance     = hInstance;	// instance handle
	wc0.lpszClassName = CLASS_NAME;	// set class name
	wc0.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// class style, you can OR these macros
	wc0.cbClsExtra = 0;
	wc0.cbWndExtra = 0;
	wc0.lpszMenuName = "lumi_menu_rc";	// set menu resource
	wc0.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);	// background color
	wc0.hCursor = LoadCursorA(NULL, IDC_ARROW);	//	default cursor
	wc0.hIcon = LoadIcon(NULL, IDI_APPLICATION); // class icon ***PLANNED***

	// register window class
	if (!RegisterClassExA(&wc0)){	// returns a class atom on success, 0 on failure
		MessageBox(NULL, "Error: RegisterClassExA returned 0", "Lumi-Pins", MB_OK);
		return 0;
	}

	// calcualte window size by client size
	RECT rect;
	rect.top = 0;
	rect.bottom = BOARD_HEIGHT;
	rect.left = 0;
	rect. right = BOARD_WIDTH + 300;
	if (!AdjustWindowRectEx(
			&rect,
			WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			1,
			0)){
		MessageBox(NULL, "Error: AdjustWindowRectEx returned 0", "Lumi-Pins", MB_OK);
		return 0;
	}

	// create window
	HWND hwnd = CreateWindowExA(	// returns an handle
			0, // Extra Window styles
			CLASS_NAME,	// WNDCLASSEXA name
			"LumiPins", // Top bar name
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX , // Window styles
			CW_USEDEFAULT,CW_USEDEFAULT,rect.right,rect.bottom, // X,Y,W,H
			NULL, // Parent handle
			NULL, // Menu handle
			hInstance, // Instance handle
			NULL // Passing value
			);

	// check window creation, handle would be null on failure
	if (hwnd == NULL){
		MessageBox(NULL, "Error: CreateWindowExA returned NULL", "Lumi-Pins", MB_OK);
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);	// show window
	UpdateWindow(hwnd);

	// sent message loop to the callback function
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}




  return 0;
}

/*
 * The call back function for Windows applcation
 */
LRESULT CALLBACK LumiWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
	case WM_PAINT:
		Draw_Default(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);	// closes background thread
		return 0;
	case WM_CLOSE:	// when X is clicked, or when the window is closed from task bar
		CloseProgram(hwnd, 0);
		return 0;
	case WM_COMMAND:	//
		switch(HIWORD(wParam)){
		case 0:
			switch(LOWORD(wParam)){
			case 101:
				MessageBox(NULL, "New", "Lumi-Pins", MB_OK);	// placeholder
				break;
			case 102:
				MessageBox(NULL, "Save", "Lumi-Pins", MB_OK);	// placeholder
				break;
			case 103:
				MessageBox(NULL, "Load", "Lumi-Pins", MB_OK);	// placeholder
				break;
			case 104:
				CloseProgram(hwnd, 0);
				break;
			}
			break;
		case 1:
			break;
		default:
			break;
		}
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void CloseProgram(HWND hwnd, bool saved){
	if(saved){
		PostQuitMessage(0);
	}else{
		switch(MessageBox(hwnd, "You have unsaved work, would you like to save?", "Error", MB_YESNOCANCEL)){
				case 6:	// Yes
					// to be added: save function
					break;
				case 7:	// No
					DestroyWindow(hwnd);	// manually destroys window, sends WM_DESTROY
					break;
				case 2:	// Cancel
					break;
				}
	}
}

bool Draw_Default(HWND hwnd){
//	HBITMAP hbmp = LoadBitmap(GetModuleHandle(NULL), "testbmp");
//	BITMAP bitmap;
//	GetObject(hbmp, sizeof(bitmap), &bitmap);
//	HDC hdc_img = CreateCompatibleDC(hdc);
//	SelectObject(hdc_img, hbmp);
//	BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdc_img, 0, 0, SRCCOPY);
//	return 0;

	HDC hdc = GetDC(hwnd);
//	PAINTSTRUCT ps;
//	hdc = BeginPaint(hwnd, &ps);.


	SelectObject(hdc, GetStockObject(DC_PEN));
	SelectObject(hdc, GetStockObject(DC_BRUSH));
	SetDCPenColor(hdc, BOARD_BACKGROUND_COLOR);
	SetDCBrushColor(hdc, BOARD_BACKGROUND_COLOR);

	Rectangle(hdc, 0, 0, 750, 750);

//	SelectObject(hdc, GetStockObject(DC_BRUSH));
//	SetDCBrushColor(hdc, RGB(255,0,0));
//	SelectObject(hdc, GetStockObject(DC_PEN));
//	SetDCPenColor(hdc, RGB(0,0,0));
//
//	Pie(hdc, 10, 10, 50, 50, 30, 10, 30, 50);
//
//	SelectObject(hdc, GetStockObject(DC_BRUSH));
//	SetDCBrushColor(hdc, RGB(0,255,0));
//	SelectObject(hdc, GetStockObject(DC_PEN));
//	SetDCPenColor(hdc, RGB(0,0,255));
//
//	Pie(hdc, 10, 10, 50, 50, 30, 50, 30, 10);

	int cell_width = BOARD_WIDTH/NUMBER_OF_COLS;
	int cell_height = BOARD_HEIGHT/NUMBER_OF_ROWS;
	int cir_width = (float)cell_width/((float)1/(float)CIRCLE_RATIO);
	int cir_height = (float)cell_height/((float)1/(float)CIRCLE_RATIO);
	int cell_space_width = (cell_width - cir_width)/2;
	int cell_space_height = (cell_height - cir_height)/2;

	for (int j = 0; j < NUMBER_OF_ROWS; j++){
		for (int i = 0; i < NUMBER_OF_COLS; i++){
			COLORREF color0 = bd.get_LED(i, j, 0);
			COLORREF color1 = bd.get_LED(i, j, 1);
			SetDCPenColor(hdc, color0);
			SetDCBrushColor(hdc, color0);
			Pie(hdc,
					i*cell_width+cell_space_width,
					j*cell_height+cell_space_height,
					i*cell_width+cell_space_width+cir_width,
					j*cell_height+cell_space_height+cir_height,
					i*cell_width+cell_space_width+cir_width/2,
					j*cell_height+cell_space_height,
					i*cell_width+cell_space_width+cir_width/2,
					j*cell_height+cell_space_height+cir_height
					);
			SetDCPenColor(hdc, color1);
			SetDCBrushColor(hdc, color1);
			Pie(hdc,
					i*cell_width+cell_space_width,
					j*cell_height+cell_space_height,
					i*cell_width+cell_space_width+cir_width,
					j*cell_height+cell_space_height+cir_height,
					i*cell_width+cell_space_width+cir_width/2,
					j*cell_height+cell_space_height+cir_height,
					i*cell_width+cell_space_width+cir_width/2,
					j*cell_height+cell_space_height
					);

		}
	}



}

