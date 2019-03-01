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
//#include "ColorPicker.h"
#include "Macro.h"



// Global variables
BoardData boarddata(NUMBER_OF_ROWS, NUMBER_OF_COLS);
ColorTray colortray;
COLORREF SUGGESTED_COLORS[16] = COLOR_PRESET;
HWND HCHECKBOX;
HWND hcolorb1;
HWND hcolorb2;
bool LMBDOWN;
int MOUSE_REGION_SELECTOR;

//ColorPicker colorpicker;


// Forward Declarations
LRESULT CALLBACK LumiWndProc(HWND, UINT, WPARAM, LPARAM);
void CloseProgram(HWND, bool);
bool Draw_All(HWND hwnd);
bool Draw_Board(HWND hwnd, HDC hdc);
bool Draw_Colorpicker(HWND hwnd, HDC hdc);
bool Draw_Control(HWND hwnd);
void Handle_WM_LBUTTONDOWN(HWND hwnd, WPARAM wParam, LPARAM lParam);
void Handle_WM_LBUTTONUP(HWND hwnd, WPARAM wParam, LPARAM lParam);
void Handle_WM_MOUSEMOVE(HWND hwnd, WPARAM wParam, LPARAM lParam);
bool mouse_action(POINT mousept);
void Handle_WM_COMMAND(HWND hwnd, WPARAM wParam, LPARAM lParam);

/*
 * The main function for Windows application
 */
int WINAPI WinMain(HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPSTR lpCmdLine,
		int nCmdShow) {

	colortray.color_index = 0;
	colortray.colors.push_back(RGB(255,255,0));
	LMBDOWN = 0;
	MOUSE_REGION_SELECTOR = MOUSE_SELECT_NONE;

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
	rect. right = BOARD_WIDTH + COLORPICKER_WIDTH;
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
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN , // Window styles
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
	case WM_CREATE:
		Draw_Control(hwnd);
		break;
	case WM_PAINT:
		Draw_All(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);	// closes background thread
		return 0;
	case WM_CLOSE:	// when X is clicked, or when the window is closed from task bar
		CloseProgram(hwnd, 0);
		return 0;
	case WM_LBUTTONDOWN:
		Handle_WM_LBUTTONDOWN(hwnd, wParam, lParam);
		break;
	case WM_LBUTTONUP:
		Handle_WM_LBUTTONUP(hwnd, wParam, lParam);
		break;
	case WM_MOUSEMOVE:
		Handle_WM_MOUSEMOVE(hwnd, wParam, lParam);
		break;
	case WM_COMMAND:	//
		Handle_WM_COMMAND(hwnd, wParam, lParam);
		break;
	case WM_ERASEBKGND:
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

bool Draw_All(HWND hwnd){

	// get dc
	HDC hdc_original = GetDC(hwnd);

	// get dc buffer
	HDC hdc_buffer = CreateCompatibleDC (hdc_original);



	HBITMAP hBitmapBuffer = CreateCompatibleBitmap (hdc_original,BOARD_WIDTH+COLORPICKER_WIDTH,BOARD_HEIGHT);  // create memory bitmap for that off screen DC

	SelectObject(hdc_buffer,hBitmapBuffer);

	SelectObject(hdc_buffer, GetStockObject(DC_PEN));
	SelectObject(hdc_buffer, GetStockObject(DC_BRUSH));
	SetDCPenColor(hdc_buffer, RGB(240,240,240));
	SetDCBrushColor(hdc_buffer, RGB(240,240,240));
	Rectangle(hdc_buffer,0,0,BOARD_WIDTH+COLORPICKER_WIDTH,BOARD_HEIGHT);

//	SetBkColor(hdc_buffer, RGB(212,208,200));
	// draw on the buffer


	Draw_Colorpicker(hwnd,hdc_buffer);

	Draw_Board(hwnd,hdc_buffer);


	// copy buffer to original dc
	BitBlt(hdc_original,0,0, BOARD_WIDTH+COLORPICKER_WIDTH,BOARD_HEIGHT,hdc_buffer,0,0,SRCCOPY);
	DeleteDC (hdc_buffer);
	DeleteObject (hBitmapBuffer);
	ReleaseDC(hwnd, hdc_original);

	// delete buffer


	if(!(colortray.color_index >= 0 && colortray.color_index < colortray.colors.size())){
		EnableWindow(hcolorb1, 0);
		EnableWindow(hcolorb2, 0);
	}


}

bool Draw_Board(HWND hwnd, HDC hdc){

//	HDC hdc = GetDC(hwnd);

	// Draw board background
	SelectObject(hdc, GetStockObject(DC_PEN));
	SelectObject(hdc, GetStockObject(DC_BRUSH));
	SetDCPenColor(hdc, BOARD_BACKGROUND_COLOR);
	SetDCBrushColor(hdc, BOARD_BACKGROUND_COLOR);
	Rectangle(hdc, 0, 0, BOARD_WIDTH, BOARD_HEIGHT);

	// get some numbers
	int cell_width = BOARD_WIDTH/NUMBER_OF_COLS;
	int cell_height = BOARD_HEIGHT/NUMBER_OF_ROWS;
	int cir_width = (float)cell_width/((float)1/(float)CIRCLE_RATIO);
	int cir_height = (float)cell_height/((float)1/(float)CIRCLE_RATIO);
	int cell_space_width = (cell_width - cir_width)/2;
	int cell_space_height = (cell_height - cir_height)/2;

	bool peg_select = (SendMessage(HCHECKBOX, BM_GETCHECK, 0, 0) == BST_CHECKED);

	for (int j = 0; j < NUMBER_OF_ROWS; j++){
		for (int i = 0; i < NUMBER_OF_COLS; i++){

			//			COLORREF color0 = boarddata.get_LED(i, j, 0);
			//			COLORREF color1 = boarddata.get_LED(i, j, 1);
			//			SetDCPenColor(hdc, color0);
			//			SetDCBrushColor(hdc, color0);
			//			Pie(hdc,
			//					i*cell_width+cell_space_width,
			//					j*cell_height+cell_space_height,
			//					i*cell_width+cell_space_width+cir_width,
			//					j*cell_height+cell_space_height+cir_height,
			//					i*cell_width+cell_space_width+cir_width/2,
			//					j*cell_height+cell_space_height,
			//					i*cell_width+cell_space_width+cir_width/2,
			//					j*cell_height+cell_space_height+cir_height
			//			);
			//			SetDCPenColor(hdc, color1);
			//			SetDCBrushColor(hdc, color1);
			//			Pie(hdc,
			//					i*cell_width+cell_space_width,
			//					j*cell_height+cell_space_height,
			//					i*cell_width+cell_space_width+cir_width,
			//					j*cell_height+cell_space_height+cir_height,
			//					i*cell_width+cell_space_width+cir_width/2,
			//					j*cell_height+cell_space_height+cir_height,
			//					i*cell_width+cell_space_width+cir_width/2,
			//					j*cell_height+cell_space_height
			//			);

			COLORREF color0 = boarddata.get_LED(i, j, peg_select);
			SetDCPenColor(hdc, color0);
			SetDCBrushColor(hdc, color0);
			Ellipse (hdc,
					i*cell_width+cell_space_width,
					j*cell_height+cell_space_height,
					i*cell_width+cell_space_width+cir_width,
					j*cell_height+cell_space_height+cir_height);
		}
	}
}

bool Draw_Colorpicker(HWND hwnd, HDC hdc){

	// draw 0 - 16 colors
	int color_y_start = (COLORPICKER_HEIGHT - COLORPICKER_BUTTON_HEIGHT - COLORPICKER_BUTTON_PADDING - NUM_COLOR_ROW * COLORPICKER_COLOR_WIDTH - NUM_COLOR_ROW * COLORPICKER_COLOR_PADDING*2 )/ 2;
	int color_x_start = BOARD_WIDTH + (COLORPICKER_WIDTH - NUM_COLOR_PER_ROW * COLORPICKER_COLOR_WIDTH - NUM_COLOR_PER_ROW * COLORPICKER_COLOR_PADDING*2 )/ 2;


	// draw 16 colors
	SelectObject(hdc, GetStockObject(DC_BRUSH));
	SelectObject(hdc, GetStockObject(DC_PEN));


	int selectid = colortray.color_index;
	for( int i = 0; i < std::min(16, (int)colortray.colors.size()); i ++){
		int i_x = color_x_start+(i % NUM_COLOR_PER_ROW)*(COLORPICKER_COLOR_WIDTH + COLORPICKER_COLOR_PADDING*2) + COLORPICKER_COLOR_PADDING;
		int i_y = color_y_start+(i / NUM_COLOR_PER_ROW)*(COLORPICKER_COLOR_HEIGHT + COLORPICKER_COLOR_PADDING*2) + COLORPICKER_COLOR_PADDING;

		if(i == selectid){
			SetDCPenColor(hdc, RGB(0,0,0));
			SetDCBrushColor(hdc, RGB(0,0,0));
			Rectangle(hdc,
					i_x - 3,
					i_y - 3,
					i_x + 3 + COLORPICKER_COLOR_WIDTH,
					i_y + 3 + COLORPICKER_COLOR_HEIGHT);
			SetDCPenColor(hdc, RGB(255,255,255));
			SetDCBrushColor(hdc, RGB(255,255,255));
			Rectangle(hdc,
					i_x - 1,
					i_y - 1,
					i_x + 1 + COLORPICKER_COLOR_WIDTH,
					i_y + 1 + COLORPICKER_COLOR_HEIGHT);

		}

		SetDCPenColor(hdc, colortray.colors[i]);
		SetDCBrushColor(hdc, colortray.colors[i]);

		Rectangle(hdc,
				i_x,
				i_y,
				i_x + COLORPICKER_COLOR_WIDTH,
				i_y + COLORPICKER_COLOR_HEIGHT);
	}
}

bool Draw_Control(HWND hwnd){
	// draw 3 buttons
	int width_between_button = (COLORPICKER_WIDTH - COLORPICKER_BUTTON_PADDING*2 - COLORPICKER_BUTTON_WIDTH*3 )/ 2;
	int b0_x = BOARD_WIDTH + COLORPICKER_BUTTON_PADDING;
	int b0_y = COLORPICKER_HEIGHT - COLORPICKER_BUTTON_PADDING - COLORPICKER_BUTTON_HEIGHT;
	int b1_x = b0_x + COLORPICKER_BUTTON_WIDTH + width_between_button;
	int b1_y = b0_y;
	int b2_x = b1_x + COLORPICKER_BUTTON_WIDTH + width_between_button;
	int b2_y = b1_y;
	HWND hcolorb0 = CreateWindowExA(
			0,
			"BUTTON",
			"New",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			b0_x,
			b0_y,
			COLORPICKER_BUTTON_WIDTH,
			COLORPICKER_BUTTON_HEIGHT,
			hwnd,
			(HMENU)105,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL
	);

	hcolorb1 = CreateWindowExA(
			0,
			"BUTTON",
			"Edit",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			b1_x,
			b1_y,
			COLORPICKER_BUTTON_WIDTH,
			COLORPICKER_BUTTON_HEIGHT,
			hwnd,
			(HMENU)106,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL
	);


	hcolorb2 = CreateWindowExA(
			0,
			"BUTTON",
			"Remove",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			b2_x,
			b2_y,
			COLORPICKER_BUTTON_WIDTH,
			COLORPICKER_BUTTON_HEIGHT,
			hwnd,
			(HMENU)107,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL
	);



	bool state = (BST_CHECKED == SendMessage(HCHECKBOX, BM_GETCHECK, 0, 0));
	HCHECKBOX = CreateWindowExA(
			0,
			"BUTTON",
			"Show activated LED colors",
			BS_CHECKBOX|WS_TABSTOP | WS_VISIBLE | WS_CHILD,
			BOARD_WIDTH + CONTROL_CHECKBOX_PADDING,
			COLORPICKER_HEIGHT + CONTROL_CHECKBOX_PADDING,
			CONTROL_WIDTH-CONTROL_CHECKBOX_PADDING*2,
			CONTROL_CHECKBOX_HEIGHT,
			hwnd,
			(HMENU)208,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL
	);
	SendMessage(HCHECKBOX, BM_SETCHECK, state, 0);
}

void Handle_WM_LBUTTONDOWN(HWND hwnd, WPARAM wParam, LPARAM lParam){
	LMBDOWN = 1;
	POINT pt;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	if (mouse_action(pt)){
		InvalidateRect(hwnd, NULL, TRUE);
	}
}


void Handle_WM_LBUTTONUP(HWND hwnd, WPARAM wParam, LPARAM lParam){

	LMBDOWN = 0;
	MOUSE_REGION_SELECTOR = MOUSE_SELECT_NONE;
	POINT pt;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	if (mouse_action(pt)){
		InvalidateRect(hwnd, NULL, TRUE);
	}


}
void Handle_WM_MOUSEMOVE(HWND hwnd, WPARAM wParam, LPARAM lParam){
	if (LMBDOWN){
		POINT pt;
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		if (mouse_action(pt)){
			InvalidateRect(hwnd, NULL, TRUE);
		}
	}
}

// if there is action, then return 1, if invalid then return 0
bool mouse_action(POINT mousept){
	int board_x_start = 0;
	int board_x_end = BOARD_WIDTH;
	int board_y_start = 0;
	int board_y_end = BOARD_HEIGHT;
	int color_y_start = (COLORPICKER_HEIGHT - COLORPICKER_BUTTON_HEIGHT - COLORPICKER_BUTTON_PADDING - NUM_COLOR_ROW * COLORPICKER_COLOR_WIDTH - NUM_COLOR_ROW * COLORPICKER_COLOR_PADDING*2 )/ 2;
	int color_x_start = BOARD_WIDTH + (COLORPICKER_WIDTH - NUM_COLOR_PER_ROW * COLORPICKER_COLOR_WIDTH - NUM_COLOR_PER_ROW * COLORPICKER_COLOR_PADDING*2 )/ 2;
	int color_y_end = color_y_start + NUM_COLOR_ROW * COLORPICKER_COLOR_WIDTH + NUM_COLOR_ROW * COLORPICKER_COLOR_PADDING*2;
	int color_x_end = color_x_start + NUM_COLOR_PER_ROW * COLORPICKER_COLOR_WIDTH + NUM_COLOR_PER_ROW * COLORPICKER_COLOR_PADDING*2;

	if( (mousept.x-board_x_start) * (board_x_end - mousept.x) >= 0 && (mousept.y-board_y_start) *( board_y_end - mousept.y) >= 0 && (MOUSE_REGION_SELECTOR == MOUSE_SELECT_NONE || MOUSE_REGION_SELECTOR == MOUSE_SELECT_BOARD)){
		int xLED = mousept.x/(BOARD_WIDTH/NUMBER_OF_COLS);
		int yLED = mousept.y/(BOARD_HEIGHT/NUMBER_OF_ROWS);

		int cell_width = BOARD_WIDTH/NUMBER_OF_COLS;
		int cell_height = BOARD_HEIGHT/NUMBER_OF_ROWS;
		int cir_width = (float)cell_width/((float)1/(float)CIRCLE_RATIO);
		int cir_height = (float)cell_height/((float)1/(float)CIRCLE_RATIO);
		int cell_space_width = (cell_width - cir_width)/2;
		int cell_space_height = (cell_height - cir_height)/2;

		int cir_square_x_start = xLED*cell_width + cell_space_width;
		int cir_square_y_start = yLED*cell_width + cell_space_height;

		if ( (mousept.x - cir_square_x_start)*(cir_square_x_start + cir_width - mousept.x) >= 0 && (mousept.y - cir_square_y_start)*(cir_square_y_start + cir_height - mousept.y) >= 0){
			if(colortray.color_index >= 0 && colortray.color_index < colortray.colors.size()){
				bool peg_select = (SendMessage(HCHECKBOX, BM_GETCHECK, 0, 0) == BST_CHECKED);
				COLORREF color_select = colortray.colors[colortray.color_index];
				if(boarddata.get_LED(xLED, yLED, peg_select) != color_select){
					boarddata.set_LED(xLED, yLED, color_select, peg_select);
					MOUSE_REGION_SELECTOR = MOUSE_SELECT_BOARD;
					return 1; // Explanation: if mouse is over the board, if mouse is inside a circle's container square, if a color is selected, if color at the current LED is not the selected color
				}
			}
		}



	}else if( (mousept.x-color_x_start) * (color_x_end - mousept.x ) >= 0 && (mousept.y-color_y_start) * ( color_y_end - mousept.y) >= 0 && (MOUSE_REGION_SELECTOR == MOUSE_SELECT_NONE || MOUSE_REGION_SELECTOR == MOUSE_SELECT_COLORPICKER)){
		int x_color_region = COLORPICKER_COLOR_WIDTH + COLORPICKER_COLOR_PADDING*2;
		int y_color_region = COLORPICKER_COLOR_HEIGHT + COLORPICKER_COLOR_PADDING*2;
		int selected = (mousept.x - color_x_start) / x_color_region + ((mousept.y - color_y_start) / y_color_region ) * NUM_COLOR_PER_ROW;

		if(selected >= 0 && selected < colortray.colors.size()){
			if(selected != colortray.color_index){
				colortray.color_index = selected;
				MOUSE_REGION_SELECTOR = MOUSE_SELECT_COLORPICKER;
				return 1;
			}
		}else{
			colortray.color_index = -1;
			MOUSE_REGION_SELECTOR = MOUSE_SELECT_COLORPICKER;
			return 1;
		}
	}

	return 0;
}




void Handle_WM_COMMAND(HWND hwnd, WPARAM wParam, LPARAM lParam){
	switch(HIWORD(wParam)){
	case BN_CLICKED:
		switch(LOWORD(wParam)){
		case 101:
			MessageBox(NULL, "New", "Lumi-Pins", MB_OK);	// placeholder
			return;
		case 102:
			MessageBox(NULL, "Save", "Lumi-Pins", MB_OK);	// placeholder
			break;
		case 103:
			MessageBox(NULL, "Load", "Lumi-Pins", MB_OK);	// placeholder
			break;
		case 104:
			CloseProgram(hwnd, 0);
			break;
		case 105:
			// new, selectcolor, get color, add it to the colorpicker

			CHOOSECOLOR cc0;
			ZeroMemory(&cc0, sizeof(cc0));
			cc0.lStructSize = sizeof(cc0);
			cc0.hwndOwner = hwnd;
			cc0.lpCustColors = SUGGESTED_COLORS;
			cc0.rgbResult = RGB(255,255,255);
			cc0.Flags = CC_FULLOPEN | CC_ANYCOLOR;
			if(ChooseColor(&cc0)){
				while(colortray.colors.size() >= NUM_COLOR_ROW*NUM_COLOR_PER_ROW){
					colortray.colors.erase(colortray.colors.begin());
				}
				colortray.colors.push_back(cc0.rgbResult);
			}
			break;
		case 106:
			// edit selected

			if(colortray.color_index >= 0 && colortray.color_index < colortray.colors.size()){
				CHOOSECOLOR cc1;
				ZeroMemory(&cc1, sizeof(cc1));
				cc1.lStructSize = sizeof(cc1);
				cc1.hwndOwner = hwnd;
				cc1.lpCustColors = SUGGESTED_COLORS;
				cc1.rgbResult = colortray.colors[colortray.color_index];
				cc1.Flags = CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;
				if(ChooseColor(&cc1)){
					colortray.colors[colortray.color_index] = cc1.rgbResult;
				}

			}

			break;
		case 107:
			// delete selected
			colortray.colors.erase(colortray.colors.begin() + colortray.color_index);
			colortray.color_index = -1;
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case 208:
			//			CloseProgram(hwnd, 0);
			if(SendMessage(HCHECKBOX, BM_GETCHECK, 0, 0) == BST_CHECKED){
				SendMessage(HCHECKBOX, BM_SETCHECK, BST_UNCHECKED, 0);
			}else{
				SendMessage(HCHECKBOX, BM_SETCHECK, BST_CHECKED, 0);
			}
			InvalidateRect(hwnd, NULL, TRUE);
			break;

		}
			break;
		case 1:
			break;
			//		case BN_CLICKED:
			//			break;
		default:
			break;
	}
}


