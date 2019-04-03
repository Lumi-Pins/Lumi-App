/*
 * File: lumi_main.cpp
 * Created: 01/25/19
 * Authors: Zelin Liu
 * Content: This is the main file for Lumi-pins app.
 *
 *
 * */

/*
 * Memo
 * Things to add:
 * Launching app from save file by Windows extension (impractical)
 * Config file to save color tray automatically, save last file path, save other options (stretch)
 * Give App an icon (todo)
 * configure arduino for batch_size not divisible by total_size
 * options -> manual set com port, set vidpid, set row*num
 * when opening options, field should be filled by correct previous value
 * when closing options, things should refresh.
 */

#include <windows.h>
#include <stdio.h>
#include "BoardData.h"
#include "Macro.h"
#include "SerialComm.h"
#include <string.h>
//#include "libusbk.h"


//#include "stdafx.h"
//#include <initguid.h>
//#include <windows.h>
//#include <Setupapi.h>

//Buffer length
//#define BUFF_LEN 20
//#include <libusb.h>



// Global variables
BoardData *boarddata; // see boarddata->h
ColorTray colortray; // see Macro.h
COLORREF SUGGESTED_COLORS[16] = COLOR_PRESET; // Colors suggested inside Windows ChooseColor child window.
HWND HCHECKBOX; // Handle for Checkbox "Show activated LED colors"
HWND HCOLORB1; // Handle for colorpicker button 1 "Edit"
HWND HCOLORB2; // Handle for colorpicker button 2 "Remove"
bool LMBDOWN; // A flag to check if LMB(Left Mouse Button) is still down
int MOUSE_REGION_SELECTOR; // A flag for the region LMB is first held down, this is implemented so cursor only trigger events in the region LMB is first held down in
bool isSaved; // A flag for whether prompt save message box is used when closing the program
char WINDOWS_BINARY_EXTENDSION[] = WINDOWS_EXTENSION_SAVE_LOAD_BINARY_FILE; // The windows file extension
const char CLASS_NAME_SETTINGS[]  = "wc_settings";
HWND HSETTINGS_TEXTBOX_ROW;
HWND HSETTINGS_TEXTBOX_COL;
HWND HSETTINGS_TEXTBOX_VID;
HWND HSETTINGS_TEXTBOX_PID;
HWND HSETTINGS_TEXTBOX_COM;
HWND HSETTINGS_CHECKBOX_HID;
HWND HSETTINGS_CHECKBOX_COM;
HWND MAIN_WINDOW;
bool USE_T_COM_F_HID;
int NUM_ROW_ACTUAL;
int NUM_COL_ACTUAL;
char VID_ACTUAL[5] = "2341";
char PID_ACTUAL[5] = "0042";
char COM_PORT_NUMBER[SETTINGS_TEXTBOX_COM_MAXLEN+1];
int COM_PORT_NUMBER_DIGITS;



// Forward Declarations
LRESULT CALLBACK LumiWndProc(HWND, UINT, WPARAM, LPARAM);
void CloseProgram(HWND);
void Draw_All(HWND hwnd);
void Draw_Board(HWND hwnd, HDC hdc);
void Draw_Colorpicker(HWND hwnd, HDC hdc);
void Draw_Control(HWND hwnd);
void Handle_WM_LBUTTONDOWN(HWND hwnd, WPARAM wParam, LPARAM lParam);
void Handle_WM_LBUTTONUP(HWND hwnd, WPARAM wParam, LPARAM lParam);
void Handle_WM_MOUSEMOVE(HWND hwnd, WPARAM wParam, LPARAM lParam);
bool mouse_action(POINT mousept);
void Handle_WM_COMMAND(HWND hwnd, WPARAM wParam, LPARAM lParam);
void NewFile(HWND hwnd);
void SaveFile(HWND hwnd);
void LoadFile(HWND hwnd);
void SyncData();
bool TransferBoardData(SerialComm sc);
bool OpenSettingsWindow(HWND);
LRESULT CALLBACK SettingsWndCallback(HWND, UINT, WPARAM, LPARAM);
void draw_settings_all(HWND hwnd);
void SaveSettings();
void LoadSettings();



/*
 * The main function for Windows application
 * https://docs.microsoft.com/en-us/windows/desktop/learnwin32/winmain--the-application-entry-point
 */
int WINAPI WinMain(HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPSTR lpCmdLine,
		int nCmdShow) {



	// first initialize some global variables;
	colortray.color_index = 0;
	colortray.colors.push_back(RGB(255,0,0)); // Initial color provided on the color tray ... red green blue
	colortray.colors.push_back(RGB(0,255,0));
	colortray.colors.push_back(RGB(0,0,255));
	LMBDOWN = 0;
	MOUSE_REGION_SELECTOR = MOUSE_SELECT_NONE;
	isSaved = 1;

	NUM_ROW_ACTUAL = SUGGESTED_NUMBER_OF_ROWS;
	NUM_COL_ACTUAL = SUGGESTED_NUMBER_OF_ROWS;

	COM_PORT_NUMBER_DIGITS=1;
	COM_PORT_NUMBER[0] = '0';
	COM_PORT_NUMBER[1] = '\0';
	COM_PORT_NUMBER[2] = '0';
	COM_PORT_NUMBER[3] = '0';



	BoardData bd_dummy(NUM_ROW_ACTUAL,NUM_COL_ACTUAL);
	boarddata = &bd_dummy;

	LoadSettings();


	// create a window class
	const char CLASS_NAME[]  = "class0";
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
		return false;
	}

	// create a window class

	WNDCLASSEXA  wc1 = { };
	wc1.cbSize = sizeof(WNDCLASSEX);	// default size
	wc1.lpfnWndProc = SettingsWndCallback;	// callback process
	wc1.hInstance     = NULL;	// instance handle
	wc1.lpszClassName = CLASS_NAME_SETTINGS;	// set class name
	wc1.style = CS_HREDRAW | CS_VREDRAW| CS_OWNDC;	// class style, you can OR these macros // original setting
	wc1.cbClsExtra = 0;
	wc1.cbWndExtra = 0;
	wc1.lpszMenuName = NULL;	// set menu resource
	wc1.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE+1);	// background color
	wc1.hCursor = LoadCursorA(NULL, IDC_ARROW);	//	default cursor
	wc1.hIcon = LoadIcon(NULL, IDI_APPLICATION); // class icon ***PLANNED***

	// register window class
	if (!RegisterClassExA(&wc1)){	// returns a class atom on success, 0 on failure
		MessageBox(NULL, "Error: RegisterClassExA returned 0", "Lumi-Pins", MB_OK);
		return false;
	}

	// calculate window size by client size
	RECT rect;
	rect.top = 0;
	rect.bottom = BOARD_HEIGHT;
	rect.left = 0;
	rect. right = BOARD_WIDTH + COLORPICKER_WIDTH;
	if (!AdjustWindowRectEx( // https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-adjustwindowrect
			&rect,
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
			1,
			0)){
		MessageBox(NULL, "Error: AdjustWindowRectEx returned 0", "Lumi-Pins", MB_OK);
		return false;
	}

	// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-createwindowa
	HWND hwnd = CreateWindowExA(	// returns an handle
			0, // Extra Window styles
			CLASS_NAME,	// WNDCLASSEXA name
			"LumiPins", // Top bar name
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN , // Window styles
			CW_USEDEFAULT,CW_USEDEFAULT,rect.right-rect.left,rect.bottom-rect.top, // X,Y,W,H
			NULL, // Parent handle
			NULL, // Menu handle
			hInstance, // Instance handle
			NULL // Passing value
	);

	// Check window creation, handle would be null on failure
	if (hwnd == NULL){
		MessageBox(NULL, "Error: CreateWindowExA returned NULL", "Lumi-Pins", MB_OK);
		return false;
	}

	MAIN_WINDOW = hwnd;

	ShowWindow(hwnd, nCmdShow);	// show window
	ShowWindow(::GetConsoleWindow(), SW_HIDE); // hide console if it shows
	UpdateWindow(hwnd); // sends WM_PAINT to draw client area


	// sent message loop to the callback function, WIN32 API stuff no need to change
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}


	return false;
}

/*
 * The call back function for Windows application
 * https://msdn.microsoft.com/en-us/library/windows/desktop/ms633573%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
 */
LRESULT CALLBACK LumiWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
	case WM_CREATE: // Draw Buttons when created, these Buttons do not get redrawn through out life cycle
		Draw_Control(hwnd); // see function
		break;
	case WM_PAINT: // Redraw on this msg
		Draw_All(hwnd); // see function
		break;
	case WM_DESTROY: // When Window is destroyed, make sure to close the background thread as well
		PostQuitMessage(0);	// closes background thread
		return false;
	case WM_CLOSE:	// when X is clicked, or when the window is closed from task bar
		CloseProgram(hwnd); // see function
		return false;
	case WM_LBUTTONDOWN: // When LMB is pressed
		Handle_WM_LBUTTONDOWN(hwnd, wParam, lParam); // see function
		break;
	case WM_LBUTTONUP: // When LMB is released
		Handle_WM_LBUTTONUP(hwnd, wParam, lParam); // see function
		break;
	case WM_MOUSEMOVE: // When cursor moves
		Handle_WM_MOUSEMOVE(hwnd, wParam, lParam); // see function
		break;
	case WM_COMMAND:	// When a menu item or a button is activated
		Handle_WM_COMMAND(hwnd, wParam, lParam); // see function
		break;
	case WM_ERASEBKGND: // When InvalidateRect() is called, it'll send WM_ERASEBKGND
		return false; // Do nothing
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

/*
 * This function handles when user choose to close the program
 * Parameter: Handle for window
 * Return: None
 */
void CloseProgram(HWND hwnd){
	if(isSaved){ // if already saved or not modified, then close window and send WM_DESTROY
		DestroyWindow(hwnd);
	}else{ // if not saved prompt to save
		switch(MessageBox(hwnd, "You have unsaved work, would you like to save?", "Error", MB_YESNOCANCEL)){
		case 6:	// Yes
			SaveFile(hwnd); // see function
			break;
		case 7:	// No
			DestroyWindow(hwnd);	// manually destroys window, sends WM_DESTROY
			break;
		case 2:	// Cancel
			break; // do nothing
		}
	}
}

/*
 * This function draws(redraws) the client area with the exception of controls
 * This function implements a double buffering system
 * Parameter: Handle for the window
 * Return: None
 */
void Draw_All(HWND hwnd){

	HDC hdc_original = GetDC(hwnd); // get DC (device context)


	HDC hdc_buffer = CreateCompatibleDC (hdc_original); // get a second DC, this one is not connected to a client area, just exists in memory
	HBITMAP hBitmapBuffer = CreateCompatibleBitmap (hdc_original,BOARD_WIDTH+COLORPICKER_WIDTH,BOARD_HEIGHT);  // create memory bitmap for that off screen DC, parameter needs to be original DC

	// Draw a back ground in memory DC
	SelectObject(hdc_buffer,hBitmapBuffer);
	SelectObject(hdc_buffer, GetStockObject(DC_PEN));
	SelectObject(hdc_buffer, GetStockObject(DC_BRUSH));
	SetDCPenColor(hdc_buffer, WINDOWS_BACKGROUND_COLOR);
	SetDCBrushColor(hdc_buffer, WINDOWS_BACKGROUND_COLOR);
	Rectangle(hdc_buffer,0,0,BOARD_WIDTH+COLORPICKER_WIDTH,BOARD_HEIGHT);

	// Draw Board and Colorpicker on memory DC, see functions
	Draw_Colorpicker(hwnd,hdc_buffer);
	Draw_Board(hwnd,hdc_buffer);

	// copy buffer to original DC
	BitBlt(hdc_original,0,0, BOARD_WIDTH+COLORPICKER_WIDTH,BOARD_HEIGHT,hdc_buffer,0,0,SRCCOPY);
	// delete buffer
	DeleteDC (hdc_buffer);
	DeleteObject (hBitmapBuffer);
	// release DC
	ReleaseDC(hwnd, hdc_original);

	// if no color is selected, color_index should be -1
	// this disables "edit" and "remove" button if no color is selected
	if(!(colortray.color_index >= (signed)0 && colortray.color_index < (signed)colortray.colors.size())){
		EnableWindow(HCOLORB1, 0);
		EnableWindow(HCOLORB2, 0);
	}else{
		EnableWindow(HCOLORB1, 1);
		EnableWindow(HCOLORB2, 1);
	}
}

/*
 * This function draws the board
 * Parameter: Handle to the window, Handle to the Device Context
 * Return: None
 */
void Draw_Board(HWND hwnd, HDC hdc){

	// Draw board background
	SelectObject(hdc, GetStockObject(DC_PEN));
	SelectObject(hdc, GetStockObject(DC_BRUSH));
	SetDCPenColor(hdc, BOARD_BACKGROUND_COLOR);
	SetDCBrushColor(hdc, BOARD_BACKGROUND_COLOR);
	Rectangle(hdc, 0, 0, BOARD_WIDTH, BOARD_HEIGHT);

	// get some numbers
	int cell_width = BOARD_WIDTH/NUM_COL_ACTUAL;
	int cell_height = BOARD_HEIGHT/NUM_ROW_ACTUAL;
	int cir_width = (float)cell_width/((float)1/(float)CIRCLE_RATIO);
	int cir_height = (float)cell_height/((float)1/(float)CIRCLE_RATIO);
	int cell_space_width = (cell_width - cir_width)/2;
	int cell_space_height = (cell_height - cir_height)/2;
	// select which array to read from
	bool peg_select = (SendMessage(HCHECKBOX, BM_GETCHECK, 0, 0) == BST_CHECKED);

	// for every row
	for (int j = 0; j < NUM_ROW_ACTUAL; j++){
		// for every element in a row
		for (int i = 0; i < NUM_COL_ACTUAL; i++){
			// get color from BoardData class
			COLORREF color0 = boarddata->get_LED(i, j, peg_select);
			// set color
			SetDCPenColor(hdc, color0);
			SetDCBrushColor(hdc, color0);
			// draw
			// this function draws a circle by defining the x,y of top left and bottom right corner of the rectangle containing the ellipse
			Ellipse (hdc,
					i*cell_width+cell_space_width, // x of top left
					j*cell_height+cell_space_height, // y of top left
					i*cell_width+cell_space_width+cir_width, // x of bottom right
					j*cell_height+cell_space_height+cir_height); // y of bottom right
		}
	}
}

/*
 * This function draws the colorpicker
 * Parameter: Handle for window, Handle for Device Context
 * Return: None
 */
void Draw_Colorpicker(HWND hwnd, HDC hdc){

	// First Determine the x,y of the top left start of the color region
	int color_y_start = (COLORPICKER_HEIGHT - COLORPICKER_BUTTON_HEIGHT - COLORPICKER_BUTTON_PADDING - NUM_COLOR_ROW * COLORPICKER_COLOR_WIDTH - NUM_COLOR_ROW * COLORPICKER_COLOR_PADDING*2 )/ 2;
	int color_x_start = BOARD_WIDTH + (COLORPICKER_WIDTH - NUM_COLOR_PER_ROW * COLORPICKER_COLOR_WIDTH - NUM_COLOR_PER_ROW * COLORPICKER_COLOR_PADDING*2 )/ 2;

	// draw 16 colors
	SelectObject(hdc, GetStockObject(DC_BRUSH));
	SelectObject(hdc, GetStockObject(DC_PEN));

	// for every color in colors, bounded by total number of colors
	for( int i = 0; i < std::min(NUM_COLOR_PER_ROW*NUM_COLOR_ROW, (int)colortray.colors.size()); i ++){
		// get the x,y box of each color
		int i_x = color_x_start+(i % NUM_COLOR_PER_ROW)*(COLORPICKER_COLOR_WIDTH + COLORPICKER_COLOR_PADDING*2) + COLORPICKER_COLOR_PADDING;
		int i_y = color_y_start+(i / NUM_COLOR_PER_ROW)*(COLORPICKER_COLOR_HEIGHT + COLORPICKER_COLOR_PADDING*2) + COLORPICKER_COLOR_PADDING;

		// if the current color is selected, draw a black outline
		// this works by drawing a bigger black rectangle, then draw a slightly bigger white rectangle over, color rectangle is then drawn over
		if(i == colortray.color_index){
			// select black color
			SetDCPenColor(hdc, RGB(0,0,0));
			SetDCBrushColor(hdc, RGB(0,0,0));
			// draw bigger rect
			Rectangle(hdc,
					i_x - 3,
					i_y - 3,
					i_x + 3 + COLORPICKER_COLOR_WIDTH,
					i_y + 3 + COLORPICKER_COLOR_HEIGHT);
			// select white color
			SetDCPenColor(hdc, RGB(255,255,255));
			SetDCBrushColor(hdc, RGB(255,255,255));
			// draw smaller rect
			Rectangle(hdc,
					i_x - 1,
					i_y - 1,
					i_x + 1 + COLORPICKER_COLOR_WIDTH,
					i_y + 1 + COLORPICKER_COLOR_HEIGHT);

		}

		// set color
		SetDCPenColor(hdc, colortray.colors[i]);
		SetDCBrushColor(hdc, colortray.colors[i]);
		// draw color rect
		Rectangle(hdc,
				i_x,
				i_y,
				i_x + COLORPICKER_COLOR_WIDTH,
				i_y + COLORPICKER_COLOR_HEIGHT);
	}
}

/*
 * This function draws control
 * Parameter: Handle to window
 * Return: None
 */
void Draw_Control(HWND hwnd){
	// first determine some coordinates
	int width_between_button = (COLORPICKER_WIDTH - COLORPICKER_BUTTON_PADDING*2 - COLORPICKER_BUTTON_WIDTH*3 )/ 2;
	int b0_x = BOARD_WIDTH + COLORPICKER_BUTTON_PADDING;
	int b0_y = COLORPICKER_HEIGHT - COLORPICKER_BUTTON_PADDING - COLORPICKER_BUTTON_HEIGHT;
	int b1_x = b0_x + COLORPICKER_BUTTON_WIDTH + width_between_button;
	int b1_y = b0_y;
	int b2_x = b1_x + COLORPICKER_BUTTON_WIDTH + width_between_button;
	int b2_y = b1_y;
	int b3_x = (COLORPICKER_WIDTH - CONTROL_BUTTON_WIDTH) / 2 + BOARD_WIDTH;
	int b3_y = COLORPICKER_HEIGHT + (BOARD_HEIGHT - COLORPICKER_HEIGHT - CONTROL_BUTTON_HEIGHT) /2;
	// Draw New button
	CreateWindowExA(
			0,
			"BUTTON",
			"New",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			b0_x,
			b0_y,
			COLORPICKER_BUTTON_WIDTH,
			COLORPICKER_BUTTON_HEIGHT,
			hwnd,
			(HMENU)IDM_COLORPICKER_NEW,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL
	);
	// Draw Edit button
	HCOLORB1 = CreateWindowExA(
			0,
			"BUTTON",
			"Edit",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			b1_x,
			b1_y,
			COLORPICKER_BUTTON_WIDTH,
			COLORPICKER_BUTTON_HEIGHT,
			hwnd,
			(HMENU)IDM_COLORPICKER_EDIT,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL
	);
	// Draw Remove button
	HCOLORB2 = CreateWindowExA(
			0,
			"BUTTON",
			"Remove",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			b2_x,
			b2_y,
			COLORPICKER_BUTTON_WIDTH,
			COLORPICKER_BUTTON_HEIGHT,
			hwnd,
			(HMENU)IDM_COLORPICKER_REMOVE,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL
	);

	// draw the checkbox
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
			(HMENU)IDM_CHECKBOX,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL
	);

	// draw the Synchronize button
	HANDLE hbutton3 = CreateWindowExA(
			0,
			"BUTTON",
			"Synchronize",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			b3_x,
			b3_y,
			CONTROL_BUTTON_WIDTH,
			CONTROL_BUTTON_HEIGHT,
			hwnd,
			(HMENU)IDM_SYNCHROIZE,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL
	);



}

/*
 * This function handles LMB press
 * Parameter: Handle for the window, wParam(additional key like CTRL), lParam(cursor coords)
 * Return: None
 */
void Handle_WM_LBUTTONDOWN(HWND hwnd, WPARAM wParam, LPARAM lParam){
	LMBDOWN = 1; // set flag ON
	// get xy cursor coord
	POINT pt;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	// call mouse_action, see function
	if (mouse_action(pt)){
		InvalidateRect(hwnd, NULL, TRUE); // sends WM_ERASEBACKGND and WM_PAINT
	}
}

/*
 * This function handles LMB release
 * Parameter: Handle for the window, wParam(additional key like CTRL), lParam(mouse coords)
 * Return: None
 */
void Handle_WM_LBUTTONUP(HWND hwnd, WPARAM wParam, LPARAM lParam){
	LMBDOWN = 0;// set flag OFF
	// get xy cursor coord

	POINT pt;
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	// call mouse_action, see function
	if (mouse_action(pt)){
		MOUSE_REGION_SELECTOR = MOUSE_SELECT_NONE;
		InvalidateRect(hwnd, NULL, TRUE); // sends WM_ERASEBACKGND and WM_PAINT
	}
	MOUSE_REGION_SELECTOR = MOUSE_SELECT_NONE;
}

/*
 * This function handles cursor movements
 * Parameter: Handle for the window, wParam(additional key like CTRL), lParam(mouse coords)
 * Return: None
 */
void Handle_WM_MOUSEMOVE(HWND hwnd, WPARAM wParam, LPARAM lParam){
	// Do if LMB is held down
	if (LMBDOWN){
		// get xy cursor coord
		POINT pt;
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		// call mouse_action, see function
		if (mouse_action(pt)){
			InvalidateRect(hwnd, NULL, TRUE); // sends WM_ERASEBACKGND and WM_PAINT
		}
	}
	// do nothing if LMB is not held down
}


/*
 * This function checks mouse coordinates, then checks if there is an valid action done, then takes action
 * Parameter: XY coordinate of cursor
 * Return: there is a valid action, then return 1, otherwise return false
 */
bool mouse_action(POINT mousept){
	// first get a bunch of values, namely x,y coords of corners of board and colorpicker
	int board_x_start = 0;
	int board_x_end = BOARD_WIDTH;
	int board_y_start = 0;
	int board_y_end = BOARD_HEIGHT;
	int color_y_start = (COLORPICKER_HEIGHT - COLORPICKER_BUTTON_HEIGHT - COLORPICKER_BUTTON_PADDING - NUM_COLOR_ROW * COLORPICKER_COLOR_WIDTH - NUM_COLOR_ROW * COLORPICKER_COLOR_PADDING*2 )/ 2;
	int color_x_start = BOARD_WIDTH + (COLORPICKER_WIDTH - NUM_COLOR_PER_ROW * COLORPICKER_COLOR_WIDTH - NUM_COLOR_PER_ROW * COLORPICKER_COLOR_PADDING*2 )/ 2;
	int color_y_end = color_y_start + NUM_COLOR_ROW * COLORPICKER_COLOR_WIDTH + NUM_COLOR_ROW * COLORPICKER_COLOR_PADDING*2;
	int color_x_end = color_x_start + NUM_COLOR_PER_ROW * COLORPICKER_COLOR_WIDTH + NUM_COLOR_PER_ROW * COLORPICKER_COLOR_PADDING*2;

	// check if cursor is on the board, checks if press region is board or none
	if( (mousept.x-board_x_start) * (board_x_end - mousept.x) >= 0 && (mousept.y-board_y_start) *( board_y_end - mousept.y) >= 0 && (MOUSE_REGION_SELECTOR == MOUSE_SELECT_NONE  || MOUSE_REGION_SELECTOR == MOUSE_SELECT_BOARD)){
		// figure out which LED (x,y)
		int xLED = mousept.x/(BOARD_WIDTH/NUM_COL_ACTUAL);
		int yLED = mousept.y/(BOARD_HEIGHT/NUM_ROW_ACTUAL);

		// some LED coordinates
		int cell_width = BOARD_WIDTH/NUM_COL_ACTUAL;
		int cell_height = BOARD_HEIGHT/NUM_ROW_ACTUAL;
		int cir_width = (float)cell_width/((float)1/(float)CIRCLE_RATIO);
		int cir_height = (float)cell_height/((float)1/(float)CIRCLE_RATIO);
		int cell_space_width = (cell_width - cir_width)/2;
		int cell_space_height = (cell_height - cir_height)/2;
		int cir_square_x_start = xLED*cell_width + cell_space_width;
		int cir_square_y_start = yLED*cell_width + cell_space_height;

		// check if cursor is in the rectangle box of ellipse
		if ( (mousept.x - cir_square_x_start)*(cir_square_x_start + cir_width - mousept.x) >= 0 && (mousept.y - cir_square_y_start)*(cir_square_y_start + cir_height - mousept.y) >= 0 ){
			// check if there is a currently selected color
			if(colortray.color_index >= (signed)0 && colortray.color_index < (signed)colortray.colors.size()){
				// get which LED vector, activated(pegged) or not
				bool peg_select = (SendMessage(HCHECKBOX, BM_GETCHECK, 0, 0) == BST_CHECKED);
				// get color
				COLORREF color_select = colortray.colors[colortray.color_index];
				// check if color is already set
				if(boarddata->get_LED(xLED, yLED, peg_select) != color_select){
					// if not then set color
					boarddata->set_LED(xLED, yLED, color_select, peg_select);
					// set mouse region flag
					MOUSE_REGION_SELECTOR = MOUSE_SELECT_BOARD;
					// data has been modified, so turn save flag OFF
					isSaved = 0;
					return 1; // Explanation: if mouse is over the board, if mouse is inside a circle's container square, if a color is selected, if color at the current LED is not the selected color
				}
			}
		}

		// check if cursor is on the colorpciker, check if press region is colorpicker or none
	}else if( (mousept.x-color_x_start) * (color_x_end - mousept.x ) >= 0 && (mousept.y-color_y_start) * ( color_y_end - mousept.y) >= 0 && (MOUSE_REGION_SELECTOR == MOUSE_SELECT_NONE || MOUSE_REGION_SELECTOR == MOUSE_SELECT_COLORPICKER)){
		// some values
		int x_color_region = COLORPICKER_COLOR_WIDTH + COLORPICKER_COLOR_PADDING*2;
		int y_color_region = COLORPICKER_COLOR_HEIGHT + COLORPICKER_COLOR_PADDING*2;
		// get which color is selected
		int selected = (mousept.x - color_x_start) / x_color_region + ((mousept.y - color_y_start) / y_color_region ) * NUM_COLOR_PER_ROW;
		// check selected in range of colors
		if(selected >= (signed)0 && selected < (signed)colortray.colors.size()){
			// check if selected is already selected(by value of color_index)
			if(selected != colortray.color_index){
				// if not then assign new color_index
				colortray.color_index = selected;
				// set mouse region flag
				MOUSE_REGION_SELECTOR = MOUSE_SELECT_COLORPICKER;
				return 1; // return true
			}
		}else{
			// check color_index is not already -1
			if( colortray.color_index != -1){
				// if out of bound, then set color_index to -1, deselects
				colortray.color_index = -1;
				// set mouse region flag
				MOUSE_REGION_SELECTOR = MOUSE_SELECT_COLORPICKER;
				return 1; // return true
			}
		}
	}

	return false; // no valid action taken, return false
}

/*
 * This function handles all the WM_COMMAND message, these messages are received when a control element or menu element is activated.
 * Parameter: Handle for window, wParam, lParam ( see https://docs.microsoft.com/en-us/windows/desktop/menurc/wm-command )
 * Return: None
 */
void Handle_WM_COMMAND(HWND hwnd, WPARAM wParam, LPARAM lParam){
	switch(HIWORD(wParam)){
	case BN_CLICKED:// menu or button
		switch(LOWORD(wParam)){ // identifier
		case IDM_MENU_NEW: // menu: new
			NewFile(hwnd); // see function
			return;
		case IDM_MENU_SAVE: // menu: save
			SaveFile(hwnd); // see function
			break;

		case IDM_MENU_LOAD: // menu: load
			LoadFile(hwnd); // see function
			break;
		case IDM_MENU_EXIT: // menu: exit
			CloseProgram(hwnd); // see function
			break;
		case IDM_MENU_SETTINGS:
			if(!OpenSettingsWindow(hwnd)){
				MessageBox(NULL, "Opening Settings Window failed", "Lumi-pins", MB_OK);
			}
			break;
		case IDM_COLORPICKER_NEW: // colorpicker: new
			// first create and initialize a CHOOSECOLOR struct
			CHOOSECOLOR cc0;
			ZeroMemory(&cc0, sizeof(cc0));
			cc0.lStructSize = sizeof(cc0); // size
			cc0.hwndOwner = hwnd; // owner
			cc0.lpCustColors = SUGGESTED_COLORS; // the pointer to 16 suggested colors displayed when this window is displayed
			cc0.rgbResult = RGB(255,255,255); // result color, initialized to black
			cc0.Flags = CC_FULLOPEN | CC_ANYCOLOR; // flags, see https://docs.microsoft.com/en-us/windows/desktop/api/commdlg/ns-commdlg-tagchoosecolora
			// starts the ChooseColor window, allowing user to select a color, ChooseColor returns true if use clicks okay, false if user closes window
			if(ChooseColor(&cc0)){
				// if colors already full, then delete from beginning (FIFO)
				while(colortray.colors.size() >= NUM_COLOR_ROW*NUM_COLOR_PER_ROW){
					colortray.colors.erase(colortray.colors.begin());
				}
				colortray.colors.push_back(cc0.rgbResult); // append new color
				colortray.color_index = colortray.colors.size()-1;
			}
			// redraw
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case IDM_COLORPICKER_EDIT: // colorpicker: edit
			// edit selected
			// check color_index for valid selected color
			if(colortray.color_index >= (signed)0 && colortray.color_index < (signed)colortray.colors.size()){
				// initialize CHOOSECOLOR struct
				CHOOSECOLOR cc1;
				ZeroMemory(&cc1, sizeof(cc1));
				cc1.lStructSize = sizeof(cc1);
				cc1.hwndOwner = hwnd;
				cc1.lpCustColors = SUGGESTED_COLORS;
				cc1.rgbResult = colortray.colors[colortray.color_index]; // the default color when window is first open is the selected color, hence "edit"
				cc1.Flags = CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;
				if(ChooseColor(&cc1)){
					// if user clicks ok then replace old color with new
					colortray.colors[colortray.color_index] = cc1.rgbResult;
				}
			}
			// redraw
			InvalidateRect(hwnd, NULL, TRUE);

			break;

		case IDM_COLORPICKER_REMOVE: // colorpicker remove
			// check color_index for valid selected color
			if(colortray.color_index >= (signed)0 && colortray.color_index < (signed)colortray.colors.size()){
				// delete selected
				colortray.colors.erase(colortray.colors.begin() + colortray.color_index);
				// change color_index to unselected
				colortray.color_index = -1;
				// redraw
				InvalidateRect(hwnd, NULL, TRUE);
			}
			break;
		case IDM_CHECKBOX: // checkbox
			// checks if checkbox is checked
			if(SendMessage(HCHECKBOX, BM_GETCHECK, 0, 0) == BST_CHECKED){
				// if yes, change to unchecked
				SendMessage(HCHECKBOX, BM_SETCHECK, BST_UNCHECKED, 0);
			}else{
				// if no, change to checked
				SendMessage(HCHECKBOX, BM_SETCHECK, BST_CHECKED, 0);
			}
			InvalidateRect(hwnd, NULL, TRUE);
			break;
		case IDM_SYNCHROIZE: // synchronize
			SyncData();
			break;

		}

		break;
		case 1: // accelerator
			break;
		default:
			break;
	}
}

/*
 * This function re-initialze boarddata and calls for redraw
 * Parameter: Handle for window
 * Return: None
 */
void NewFile(HWND hwnd){
	*boarddata = BoardData(NUM_ROW_ACTUAL, NUM_COL_ACTUAL);
	InvalidateRect(hwnd, NULL, TRUE);
}

/*
 * This function starts a file dialog box to select a path&filename, creates/opens the file, then writes data to the file
 * Parameter: Handle for window
 * Return: None
 */
void SaveFile(HWND hwnd){
	char path[256]; // drive + path + filename + extension
	// Creates and initializes OPENFILENAME struct
	OPENFILENAME ofn; // see https://docs.microsoft.com/en-us/windows/desktop/api/commdlg/ns-commdlg-tagofna
	ZeroMemory( &ofn , sizeof( ofn));
	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = hwnd; // handle to window owner
	ofn.lpstrFile = path; // contains the drive designator, path, file name, and extension of the selected file (return parameter)
	ofn.lpstrFile[0] = '\0'; // according to MicroSoft documentation, the first byte of path must be null terminate character
	ofn.nMaxFile = sizeof(path); // The size, in characters, of the buffer pointed to by lpstrFile
	ofn.lpstrInitialDir = NULL; // The initial directory.
	ofn.lpstrTitle = NULL; // A string to be placed in the title bar of the dialog box. If this member is NULL, the system uses the default title (that is, Save As or Open).
	ofn.lpstrFilter = "Lumi-Pins Data\0*.lumidata\0"; // file types, look up @ https://docs.microsoft.com/en-us/windows/desktop/api/commdlg/ns-commdlg-tagofna
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex =0;
	ofn.lpstrDefExt = WINDOWS_BINARY_EXTENDSION; // default extension

	// returns false if user cancels the dialog window
	if(!GetSaveFileName( &ofn )){
		return; // do nothing
	}

	HANDLE hFile; // handle for the file
	// Attempts to open existing file and delete its content, specified by path
	hFile = CreateFile(path,                // name of the write
			GENERIC_WRITE,          // open for writing
			0,                      // do not share
			NULL,                   // default security
			TRUNCATE_EXISTING,             // create new file only
			FILE_ATTRIBUTE_NORMAL,  // normal file
			NULL);                  // no attr. template
	// if the call to CreateFile failed, then INVALID_HANDLE_VALUE will be returned, no existing file is most likely the cause
	if(   hFile == INVALID_HANDLE_VALUE){
		// Attempts to create a new file, specified by path
		hFile = CreateFile(path,                // name of the write
				GENERIC_WRITE,          // open for writing
				0,                      // do not share
				NULL,                   // default security
				CREATE_NEW,             // create new file only
				FILE_ATTRIBUTE_NORMAL,  // normal file
				NULL);               // no attr. template
		if( hFile  == INVALID_HANDLE_VALUE){ // if BOTH attempts (open and delete && create new) fail
			MessageBox(NULL, "Cannot open or create file", "Lumi-pins SaveFile", MB_OK);
			return; // give a message and return
		}
	}

	DWORD dwBytesToWrite = boarddata->get_readwrite_size(); // get data size in bytes
	DWORD dwBytesWritten = 0;
	char* DataBuffer =(char*) malloc(dwBytesToWrite); // Get a binary buffer for the data
	boarddata->write_to_array(DataBuffer, 0, boarddata->get_readwrite_size()); // transfer data to buffer

	// write buffer to file
	bool write_result = WriteFile(
			hFile,           // open file handle
			DataBuffer,      // start of data to write
			dwBytesToWrite,  // number of bytes to write
			&dwBytesWritten, // number of bytes that were written
			NULL);            // no overlapped structure

	// free resources
	free(DataBuffer);
	CloseHandle(hFile);

	// if write failed
	if(!write_result){
		// show error code
		DWORD ERRORc = (DWORD)GetLastError();
		char buff [100];
		sprintf(buff, "WriteFile Failed: Error %lu", ERRORc);
		MessageBox(NULL, buff, "Lumi-pins SaveFile", MB_OK);
		return;
	}

	// if ToWrite and Written mismatch
	if(dwBytesWritten!=dwBytesToWrite){
		// show mismatch
		char buff [64];
		sprintf(buff, "dwBytesWritten: %lu  ;;;;; dwBytesToWrit: %lu", dwBytesWritten,dwBytesToWrite);
		MessageBox(NULL, buff, "Lumi-pins SaveFile", MB_OK);
		return;
	}

	// Otherwise successful
	MessageBox(NULL, "Save successful!", "Lumi-pins SaveFile", MB_OK);
	isSaved = 1; // set Flag to already saved
	return;
}

/*
 * This function starts a file dialog box to select a path&filename, opens the file, then read data from the file
 * Parameter: Handle for window
 * Return: None
 */
void LoadFile(HWND hwnd){
	// see SaveFile
	char path[256];
	OPENFILENAME ofn;
	ZeroMemory( &ofn , sizeof( ofn));
	ofn.lStructSize = sizeof ( ofn );
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = path;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(path);
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.lpstrFilter = "Lumi-Pins Data\0*.lumidata\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex =0;

	if(!GetOpenFileName( &ofn )){
		return;
	}

	HANDLE hFile;
	// Attempts to open file specified by path
	hFile = CreateFile(path,               // file to open
			GENERIC_READ,          // open for reading
			FILE_SHARE_READ,       // share for reading
			NULL,                  // default security
			OPEN_EXISTING,         // existing file only
			FILE_ATTRIBUTE_NORMAL, // normal file
			NULL);                 // no attr. template

	if(   hFile == INVALID_HANDLE_VALUE){
		MessageBox(NULL, "Cannot open file", "Lumi-pins LoadFile", MB_OK);
		return;
	}


	DWORD dwBytesToRead = boarddata->get_readwrite_size(); // get data size in bytes
	char *DataBuffer = (char*)malloc(dwBytesToRead); // set memory for read buffer
	DWORD dwBytesRead = 0;

	// Attempts to read file
	bool read_result = ReadFile(
			hFile,
			DataBuffer,
			dwBytesToRead,
			&dwBytesRead,
			NULL
	);

	CloseHandle(hFile); // release resources

	// if read failed
	if(!read_result){
		// print error code
		DWORD ERRORc = (DWORD)GetLastError();
		char buff [100];
		sprintf(buff, "ReadFile Failed: Error %lu", ERRORc);
		MessageBox(NULL, buff, "Lumi-pins SaveFile", MB_OK);
		free(DataBuffer); // release memory
		return;
	}

	// if ToRead and Read mismatch
	if(dwBytesToRead!=dwBytesRead){
		// show mismatch
		char buff [64];
		sprintf(buff, "dwBytesRead: %lu  ;;;;; dwBytesToRead: %lu", dwBytesRead,dwBytesToRead);
		MessageBox(NULL, buff, "Lumi-pins LoadFile", MB_OK);
		free(DataBuffer); // release memory
		return;
	}

	// transfer data from buffer to boarddata
	boarddata->read_from_array(DataBuffer);
	free(DataBuffer); // release memory

	InvalidateRect(hwnd, NULL, TRUE); // redraw
	isSaved = 1; // so save message doesn't pop up when closing
	MessageBox(NULL, "Load Successful!", "Lumi-pins LoadFile", MB_OK);

	return;
}

void SyncData(){
	SerialComm serialcomm; // see SerialComm.h

	if(!USE_T_COM_F_HID){
		if(!serialcomm.findPortbyPIDVID(VID_ACTUAL, PID_ACTUAL)){
			MessageBox(NULL, "Error: serialcomm.findPortbyPIDVID(ARDUINO_VID, ARDUINO_PID)", "Lumi-pins SyncData", MB_OK);
			if(!serialcomm.close()){
				MessageBox(NULL, "Error: serialcomm.close()", "Lumi-pins SyncData", MB_OK);
			}
			return;
		}
	}else{
		serialcomm.setPort(COM_PORT_NUMBER, COM_PORT_NUMBER_DIGITS);
	}


	if(!serialcomm.connect()){
		MessageBox(NULL, "Error: serialcomm.connect()", "Lumi-pins SyncData", MB_OK);
		if(!serialcomm.close()){
			MessageBox(NULL, "Error: serialcomm.close()", "Lumi-pins SyncData", MB_OK);
		}
		return;
	}
	if(!serialcomm.init_param()){
		MessageBox(NULL, "Error: serialcomm.init_param()", "Lumi-pins SyncData", MB_OK);
		if(!serialcomm.close()){
			MessageBox(NULL, "Error: serialcomm.close()", "Lumi-pins SyncData", MB_OK);
		}
		return;
	}

	//	char test[] = "this is a freaking test to see if checksum shit works";
	//	if(serialcomm.single_cycle(test, 50, 3)){
	//		MessageBox(NULL, "Sync successful!", "Lumi-pins SyncData", MB_OK);
	//	}else{
	//		MessageBox(NULL, "Sync failed!", "Lumi-pins SyncData", MB_OK);
	//	}

	if(TransferBoardData(serialcomm)){
		if(!serialcomm.close()){
			MessageBox(NULL, "Error: serialcomm.close()", "Lumi-pins SyncData", MB_OK);
			return;
		}
		MessageBox(NULL, "Sync successful!", "Lumi-pins SyncData", MB_OK);
	}else{
		if(!serialcomm.close()){
			MessageBox(NULL, "Error: serialcomm.close()", "Lumi-pins SyncData", MB_OK);
			return;
		}
		MessageBox(NULL, "Sync failed!", "Lumi-pins SyncData", MB_OK);
	}
	return;

}

bool TransferBoardData(SerialComm sc){

	unsigned short int begin_bytes = SERIAL_BEGIN_BYTE;
	unsigned short int total_size = boarddata->get_readwrite_size();
	unsigned short int batch_size = SERIAL_BATCH_SIZE;
	unsigned short int batch_count = 1+((total_size -  1) / batch_size); // ceiling division
	//	char buff[30];
	//	sprintf(buff, "batch count %d", batch_count);
	//	MessageBox(NULL, buff, "Lumi-pins SyncData", MB_OK);


	char header[6];
	SerialComm::USHORT2CHARARRAY(begin_bytes, header);
	SerialComm::USHORT2CHARARRAY(total_size, header+2);
	SerialComm::USHORT2CHARARRAY(batch_size, header+4);
	if(!sc.single_cycle(header,6)){
		return false;
	}


	char* complete_array = (char*)malloc(total_size*sizeof(char));
	boarddata->write_to_array(complete_array, 0, total_size);
	unsigned short int final_checksum = SerialComm::fletcher16((unsigned char*)complete_array, (unsigned int)total_size);

	//	char buffz[30];
	//	sprintf(buffz, "final_checksum %d", final_checksum);
	//	MessageBox(NULL, buffz, "Lumi-pins SyncData", MB_OK);

	char* buffer = (char*)malloc(batch_size+2*sizeof(char));
	for ( unsigned short int i = 0; i < batch_count; i ++){

		SerialComm::USHORT2CHARARRAY(i,buffer);
		memcpy(buffer+2, i*batch_size+complete_array, batch_size);
		if(!sc.single_cycle(buffer, batch_size+2)){
			free(buffer);
			free(complete_array);
			return false;
		}
	}

	free(buffer);
	free(complete_array);



	char array_checksum[2];
	SerialComm::USHORT2CHARARRAY(final_checksum, array_checksum);
	if(!sc.single_cycle(array_checksum, 2)){
		return false;
	}


	return 1;
}

bool OpenSettingsWindow(HWND hwnd_parent){
	INT color = COLOR_APPWORKSPACE;
	COLORREF toset = WINDOWS_BACKGROUND_COLOR;
	if(!SetSysColors(1,&color,&toset)){
		MessageBox(NULL, "Error: Unable to set background color", "Lumi-Pins", MB_OK);
	}





	// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-createwindowa
	HWND hwnd_settings = CreateWindowExA(	// returns an handle
			0, // Extra Window styles
			CLASS_NAME_SETTINGS,	// WNDCLASSEXA name
			"Settings", // Top bar name
			WS_POPUPWINDOW | WS_CAPTION , // Window styles
			SETTINGS_WINDOW_START_X,SETTINGS_WINDOW_START_Y,SETTINGS_WIDTH,SETTINGS_HEIGHT, // X,Y,W,H
			hwnd_parent, // Parent handle
			NULL, // Menu handle
			NULL, // Instance handle
			NULL // Passing value
	);

	// Check window creation, handle would be null on failure
	if (hwnd_settings == NULL){
		MessageBox(NULL, "Error: CreateWindowExA returned NULL", "Lumi-Pins Settings", MB_OK);
		return false;
	}

	ShowWindow(hwnd_settings, SW_SHOW);	// show window
	InvalidateRect(hwnd_settings, NULL, TRUE);

	return true;
}
LRESULT CALLBACK SettingsWndCallback(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
	case WM_PAINT:
		draw_settings_all(hwnd);
		break;
	case WM_ERASEBKGND:

		break;

	case WM_COMMAND:
		switch(HIWORD(wParam)){
		case BN_CLICKED:
			switch(LOWORD(wParam)){

			case IDM_SETTINGS_BUTTON_OK:
				SaveSettings();
				DestroyWindow(hwnd);
				InvalidateRect(MAIN_WINDOW, NULL, TRUE);
				break;
			case IDM_SETTINGS_BUTTON_CANCEL:
				DestroyWindow(hwnd);
				InvalidateRect(MAIN_WINDOW, NULL, TRUE);
				break;
			case IDM_SETTINGS_CHECKBOX_HID:
				if(SendMessage(HSETTINGS_CHECKBOX_HID, BM_GETCHECK, 0, 0) == BST_CHECKED){
					// if yes, change to unchecked
					SendMessage(HSETTINGS_CHECKBOX_HID, BM_SETCHECK, BST_UNCHECKED, 0);
					SendMessage(HSETTINGS_CHECKBOX_COM, BM_SETCHECK, BST_CHECKED, 0);
					EnableWindow(HSETTINGS_TEXTBOX_VID, 0);
					EnableWindow(HSETTINGS_TEXTBOX_PID, 0);
					EnableWindow(HSETTINGS_TEXTBOX_COM, 1);
				}else{
					// if no, change to checked
					SendMessage(HSETTINGS_CHECKBOX_HID, BM_SETCHECK, BST_CHECKED, 0);
					SendMessage(HSETTINGS_CHECKBOX_COM, BM_SETCHECK, BST_UNCHECKED, 0);
					EnableWindow(HSETTINGS_TEXTBOX_VID, 1);
					EnableWindow(HSETTINGS_TEXTBOX_PID, 1);
					EnableWindow(HSETTINGS_TEXTBOX_COM, 0);
				}
				break;
			case IDM_SETTINGS_CHECKBOX_COM:

				if(SendMessage(HSETTINGS_CHECKBOX_COM, BM_GETCHECK, 0, 0) == BST_CHECKED){
					// if yes, change to unchecked
					SendMessage(HSETTINGS_CHECKBOX_COM, BM_SETCHECK, BST_UNCHECKED, 0);
					SendMessage(HSETTINGS_CHECKBOX_HID, BM_SETCHECK, BST_CHECKED, 0);
					EnableWindow(HSETTINGS_TEXTBOX_VID, 1);
					EnableWindow(HSETTINGS_TEXTBOX_PID, 1);
					EnableWindow(HSETTINGS_TEXTBOX_COM, 0);
				}else{
					// if no, change to checked
					SendMessage(HSETTINGS_CHECKBOX_COM, BM_SETCHECK, BST_CHECKED, 0);
					SendMessage(HSETTINGS_CHECKBOX_HID, BM_SETCHECK, BST_UNCHECKED, 0);
					EnableWindow(HSETTINGS_TEXTBOX_VID, 0);
					EnableWindow(HSETTINGS_TEXTBOX_PID, 0);
					EnableWindow(HSETTINGS_TEXTBOX_COM, 1);
				}
				break;
			}
			break;
		}
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void draw_settings_all(HWND hwnd){
	char display_buffer_row[SETTINGS_TEXTBOX_ROW_MAXLEN+1];
	char display_buffer_col[SETTINGS_TEXTBOX_ROW_MAXLEN+1];
	itoa(NUM_ROW_ACTUAL, display_buffer_row, 10);
	itoa(NUM_COL_ACTUAL, display_buffer_col, 10);



	HWND hsettings_button_ok = CreateWindowExA(
			0,
			"BUTTON",
			"OK",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			SETTINGS_BUTTON_OK_X,
			SETTINGS_BUTTON_OK_Y,
			CONTROL_BUTTON_WIDTH,
			CONTROL_BUTTON_HEIGHT,
			hwnd,
			(HMENU)IDM_SETTINGS_BUTTON_OK,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL
	);

	HWND hsettings_button_cancel = CreateWindowExA(
			0,
			"BUTTON",
			"Cancel",
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			SETTINGS_BUTTON_CANCEL_X,
			SETTINGS_BUTTON_CANCEL_Y,
			CONTROL_BUTTON_WIDTH,
			CONTROL_BUTTON_HEIGHT,
			hwnd,
			(HMENU)IDM_SETTINGS_BUTTON_CANCEL,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL
	);

	HSETTINGS_TEXTBOX_ROW=CreateWindowExA(
			0,
			TEXT("Edit"),
			display_buffer_row,
			WS_CHILD | WS_VISIBLE | WS_BORDER|ES_NUMBER|ES_CENTER,
			SETTINGS_TEXTBOX_ROW_X,
			SETTINGS_TEXTBOX_ROW_Y,
			SETTINGS_TEXT_WIDTH_PER_CHAR*SETTINGS_TEXTBOX_ROW_MAXLEN,
			SETTINGS_TEXT_HEIGHT,
			hwnd,
			NULL,
			NULL,
			NULL
	);

	HSETTINGS_TEXTBOX_COL=CreateWindowExA(
			0,
			TEXT("Edit"),
			display_buffer_col,
			WS_CHILD | WS_VISIBLE | WS_BORDER|ES_NUMBER|ES_CENTER,
			SETTINGS_TEXTBOX_COL_X,
			SETTINGS_TEXTBOX_COL_Y,
			SETTINGS_TEXT_WIDTH_PER_CHAR*SETTINGS_TEXTBOX_COL_MAXLEN,
			SETTINGS_TEXT_HEIGHT,
			hwnd,
			NULL,
			NULL,
			NULL
	);

	HSETTINGS_TEXTBOX_VID=CreateWindowExA(
			0,
			TEXT("Edit"),
			VID_ACTUAL,
			WS_CHILD | WS_VISIBLE | WS_BORDER|ES_UPPERCASE|ES_CENTER,
			SETTINGS_TEXTBOX_VID_X,
			SETTINGS_TEXTBOX_VID_Y,
			SETTINGS_TEXT_WIDTH_PER_CHAR*SETTINGS_TEXTBOX_VID_MAXLEN,
			SETTINGS_TEXT_HEIGHT,
			hwnd,
			NULL,
			NULL,
			NULL
	);

	HSETTINGS_TEXTBOX_PID=CreateWindowExA(
			0,
			TEXT("Edit"),
			PID_ACTUAL,
			WS_CHILD | WS_VISIBLE | WS_BORDER|ES_UPPERCASE|ES_CENTER,
			SETTINGS_TEXTBOX_PID_X,
			SETTINGS_TEXTBOX_PID_Y,
			SETTINGS_TEXT_WIDTH_PER_CHAR*SETTINGS_TEXTBOX_PID_MAXLEN,
			SETTINGS_TEXT_HEIGHT,
			hwnd,
			NULL,
			NULL,
			NULL
	);

	HSETTINGS_TEXTBOX_COM=CreateWindowExA(
			0,
			TEXT("Edit"),
			COM_PORT_NUMBER,
			WS_CHILD | WS_VISIBLE |WS_BORDER|ES_NUMBER|ES_CENTER,
			SETTINGS_TEXTBOX_COM_X,
			SETTINGS_TEXTBOX_COM_Y,
			SETTINGS_TEXT_WIDTH_PER_CHAR*SETTINGS_TEXTBOX_COM_MAXLEN,
			SETTINGS_TEXT_HEIGHT,
			hwnd,
			NULL,
			NULL,
			NULL
	);

	HSETTINGS_CHECKBOX_HID = CreateWindowExA(
			0,
			"BUTTON",
			"Use Hardware ID to sync",
			BS_CHECKBOX|WS_TABSTOP | WS_VISIBLE | WS_CHILD,
			SETTINGS_CHECKBOX_HID_X,
			SETTINGS_CHECKBOX_HID_Y,
			SETTINGS_CHECKBOX_HID_WIDTH,
			SETTINGS_TEXT_HEIGHT,
			hwnd,
			(HMENU)IDM_SETTINGS_CHECKBOX_HID,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL
	);

	HSETTINGS_CHECKBOX_COM = CreateWindowExA(
			0,
			"BUTTON",
			"Use COM port to sync",
			BS_CHECKBOX|WS_TABSTOP | WS_VISIBLE | WS_CHILD,
			SETTINGS_CHECKBOX_COM_X,
			SETTINGS_CHECKBOX_COM_Y,
			SETTINGS_CHECKBOX_COM_WIDTH,
			SETTINGS_TEXT_HEIGHT,
			hwnd,
			(HMENU)IDM_SETTINGS_CHECKBOX_COM,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL
	);

	HWND hsettings_statictext_row=CreateWindowExA(
			0,
			TEXT("Static"),
			TEXT("Number of Rows:"),
			WS_CHILD | WS_VISIBLE,
			SETTINGS_STATICTEXT_ROW_X,
			SETTINGS_STATICTEXT_ROW_Y,
			SETTINGS_STATICTEXT_ROW_LEN,
			SETTINGS_TEXT_HEIGHT,
			hwnd,
			NULL,
			NULL,
			NULL
	);

	HWND hsettings_statictext_col=CreateWindowExA(
			0,
			TEXT("Static"),
			TEXT("Number of Cols:"),
			WS_CHILD | WS_VISIBLE,
			SETTINGS_STATICTEXT_COL_X,
			SETTINGS_STATICTEXT_COL_Y,
			SETTINGS_STATICTEXT_COL_LEN,
			SETTINGS_TEXT_HEIGHT,
			hwnd,
			NULL,
			NULL,
			NULL
	);

	HWND hsettings_statictext_vid=CreateWindowExA(
			0,
			TEXT("Static"),
			TEXT("VID:"),
			WS_CHILD | WS_VISIBLE,
			SETTINGS_STATICTEXT_VID_X,
			SETTINGS_STATICTEXT_VID_Y,
			SETTINGS_STATICTEXT_VID_LEN,
			SETTINGS_TEXT_HEIGHT,
			hwnd,
			NULL,
			NULL,
			NULL
	);

	HWND hsettings_statictext_pid=CreateWindowExA(
			0,
			TEXT("Static"),
			TEXT("PID:"),
			WS_CHILD | WS_VISIBLE,
			SETTINGS_STATICTEXT_PID_X,
			SETTINGS_STATICTEXT_PID_Y,
			SETTINGS_STATICTEXT_PID_LEN,
			SETTINGS_TEXT_HEIGHT,
			hwnd,
			NULL,
			NULL,
			NULL
	);

	HWND hsettings_statictext_com=CreateWindowExA(
			0,
			TEXT("Static"),
			TEXT("COM"),
			WS_CHILD | WS_VISIBLE,
			SETTINGS_STATICTEXT_COM_X,
			SETTINGS_STATICTEXT_COM_Y,
			SETTINGS_STATICTEXT_COM_LEN,
			SETTINGS_TEXT_HEIGHT,
			hwnd,
			NULL,
			NULL,
			NULL
	);

	SendMessage(HSETTINGS_TEXTBOX_ROW, EM_SETLIMITTEXT , SETTINGS_TEXTBOX_ROW_MAXLEN, 0);
	SendMessage(HSETTINGS_TEXTBOX_COL, EM_SETLIMITTEXT , SETTINGS_TEXTBOX_COL_MAXLEN, 0);
	SendMessage(HSETTINGS_TEXTBOX_VID, EM_SETLIMITTEXT , SETTINGS_TEXTBOX_VID_MAXLEN, 0);
	SendMessage(HSETTINGS_TEXTBOX_PID, EM_SETLIMITTEXT , SETTINGS_TEXTBOX_PID_MAXLEN, 0);
	SendMessage(HSETTINGS_TEXTBOX_COM, EM_SETLIMITTEXT , SETTINGS_TEXTBOX_COM_MAXLEN, 0);

	if(USE_T_COM_F_HID){
		// if yes, change to unchecked
		SendMessage(HSETTINGS_CHECKBOX_HID, BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(HSETTINGS_CHECKBOX_COM, BM_SETCHECK, BST_CHECKED, 0);
		EnableWindow(HSETTINGS_TEXTBOX_VID, 0);
		EnableWindow(HSETTINGS_TEXTBOX_PID, 0);
		EnableWindow(HSETTINGS_TEXTBOX_COM, 1);
	}else{
		// if no, change to checked
		SendMessage(HSETTINGS_CHECKBOX_HID, BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(HSETTINGS_CHECKBOX_COM, BM_SETCHECK, BST_UNCHECKED, 0);
		EnableWindow(HSETTINGS_TEXTBOX_VID, 1);
		EnableWindow(HSETTINGS_TEXTBOX_PID, 1);
		EnableWindow(HSETTINGS_TEXTBOX_COM, 0);
	}
}
void SaveSettings(){

	char input_row[SETTINGS_TEXTBOX_ROW_MAXLEN+1];
	char input_col[SETTINGS_TEXTBOX_COL_MAXLEN+1];
	char input_vid[SETTINGS_TEXTBOX_VID_MAXLEN+1];
	char input_pid[SETTINGS_TEXTBOX_PID_MAXLEN+1];
	char input_com[SETTINGS_TEXTBOX_COM_MAXLEN+1];
	int input_row_len = GetWindowTextA(HSETTINGS_TEXTBOX_ROW, input_row, SETTINGS_TEXTBOX_ROW_MAXLEN+1);
	int input_col_len = GetWindowTextA(HSETTINGS_TEXTBOX_COL, input_col, SETTINGS_TEXTBOX_COL_MAXLEN+1);
	int input_vid_len = GetWindowTextA(HSETTINGS_TEXTBOX_VID, input_vid, SETTINGS_TEXTBOX_VID_MAXLEN+1);
	int input_pid_len = GetWindowTextA(HSETTINGS_TEXTBOX_PID, input_pid, SETTINGS_TEXTBOX_PID_MAXLEN+1);
	int input_com_len = GetWindowTextA(HSETTINGS_TEXTBOX_COM, input_com, SETTINGS_TEXTBOX_COM_MAXLEN+1);
	if(input_row_len == 0 || input_col_len ==0){
		MessageBox(NULL, "Cannot save settings:Invalid row/column configuration", "Lumi-Pins Settings", MB_OK);
		return;
	}
	char input_checkbox_t_com_f_hid = 0x00;
	if(SendMessage(HSETTINGS_CHECKBOX_HID, BM_GETCHECK, 0, 0) == BST_CHECKED){ // if hid is checked
		input_checkbox_t_com_f_hid = 'F';
		USE_T_COM_F_HID = false;
		if (input_vid_len != 4 || input_pid_len != 4){
			MessageBox(NULL, "Cannot save settings:Invalid Hardware ID", "Lumi-Pins Settings", MB_OK);
			return;
		}
	}else{
		if (input_com_len == 0){
			MessageBox(NULL, "Cannot save settings:Invalid COM port", "Lumi-Pins Settings", MB_OK);
			return;
		}
		input_checkbox_t_com_f_hid = 'T';
		USE_T_COM_F_HID = true;
	}

	int row_num = atoi(input_row);
	int col_num = atoi(input_col);
	if(row_num<1 || col_num<1 || row_num>99 || col_num>99){
		return;
	}

	if(USE_T_COM_F_HID){
		if(input_com_len < 1 || input_com_len > 3){return;}

	}else {
		if(input_vid_len != 4 || input_pid_len != 4){return;}
	}

	char total_save_buffer[SETTINGS_SAVEFILE_MAXLEN];
	int current_buffer_position = 0;

	memcpy(total_save_buffer + current_buffer_position, input_row, input_row_len);
	current_buffer_position += input_row_len;
	total_save_buffer[current_buffer_position] = 0xFF;
	current_buffer_position += 1;

	memcpy(total_save_buffer + current_buffer_position, input_col, input_col_len);
	current_buffer_position += input_col_len;
	total_save_buffer[current_buffer_position] = 0xFF;
	current_buffer_position += 1;

	memcpy(total_save_buffer + current_buffer_position, &input_checkbox_t_com_f_hid, 1);
	current_buffer_position += 1;
	total_save_buffer[current_buffer_position] = 0xFF;
	current_buffer_position += 1;

	memcpy(total_save_buffer + current_buffer_position, input_vid, input_vid_len);
	current_buffer_position += input_vid_len;
	total_save_buffer[current_buffer_position] = 0xFF;
	current_buffer_position += 1;

	memcpy(total_save_buffer + current_buffer_position, input_pid, input_pid_len);
	current_buffer_position += input_pid_len;
	total_save_buffer[current_buffer_position] = 0xFF;
	current_buffer_position += 1;

	memcpy(total_save_buffer + current_buffer_position, input_com, input_com_len);
	current_buffer_position += input_com_len;
	total_save_buffer[current_buffer_position] = 0xFF;
	current_buffer_position += 1;


	char exe_Path[MAX_PATH];
	if(!GetModuleFileName( NULL, exe_Path, MAX_PATH )){
		MessageBox(NULL, "Error: cannot retrieve current path", "Lumi-Pins", MB_OK);
		return;
	}
	int dir_Len = (strrchr(exe_Path, '\\') - exe_Path + 1)*sizeof(*exe_Path);
	char SaveFileName[] = WINDOWS_SETTINGS_SAVE_FILE_NAME;
	char SaveFileFullPath[dir_Len+strlen(SaveFileName)];
	memcpy(SaveFileFullPath,exe_Path,dir_Len);
	memcpy(SaveFileFullPath+dir_Len, SaveFileName, strlen(SaveFileName)+1);
	//	SaveFileFullPath[dir_Len+strlen(SaveFileName)]='\0';
	//	MessageBox(NULL, SaveFileFullPath, "Lumi-Pins", MB_OK);

	HANDLE hSettingsFile; // handle for the file
	// Attempts to open existing file and delete its content, specified by path
	hSettingsFile = CreateFile(SaveFileFullPath,                // name of the write
			GENERIC_WRITE,          // open for writing
			0,                      // do not share
			NULL,                   // default security
			TRUNCATE_EXISTING,             // create new file only
			FILE_ATTRIBUTE_NORMAL,  // normal file
			NULL);                  // no attr. template
	// if the call to CreateFile failed, then INVALID_HANDLE_VALUE will be returned, no existing file is most likely the cause
	if(   hSettingsFile == INVALID_HANDLE_VALUE){
		// Attempts to create a new file, specified by path
		hSettingsFile = CreateFile(SaveFileFullPath,                // name of the write
				GENERIC_WRITE,          // open for writing
				0,                      // do not share
				NULL,                   // default security
				CREATE_NEW,             // create new file only
				FILE_ATTRIBUTE_NORMAL,  // normal file
				NULL);               // no attr. template
		if( hSettingsFile  == INVALID_HANDLE_VALUE){ // if BOTH attempts (open and delete && create new) fail
			MessageBox(NULL, "Cannot open or create file", "Lumi-pins SettingsFile", MB_OK);
			MessageBox(NULL, SaveFileFullPath, "Lumi-pins SettingsFile", MB_OK);
			return; // give a message and return
		}
	}

	DWORD dwBytesToWrite = current_buffer_position; // get data size in bytes
	DWORD dwBytesWritten = 0;

	// write buffer to file
	bool write_result = WriteFile(
			hSettingsFile,           // open file handle
			total_save_buffer,      // start of data to write
			dwBytesToWrite,  // number of bytes to write
			&dwBytesWritten, // number of bytes that were written
			NULL);            // no overlapped structure

	// free resources
	CloseHandle(hSettingsFile);

	// if write failed
	if(!write_result){
		// show error code
		DWORD ERRORc = (DWORD)GetLastError();
		char buff [100];
		sprintf(buff, "WriteFile Failed: Error %lu", ERRORc);
		MessageBox(NULL, buff, "Lumi-pins SettingsFile", MB_OK);
		return;
	}

	// if ToWrite and Written mismatch
	if(dwBytesWritten!=dwBytesToWrite){
		// show mismatch
		char buff [64];
		sprintf(buff, "dwBytesWritten: %lu  ;;;;; dwBytesToWrit: %lu", dwBytesWritten,dwBytesToWrite);
		MessageBox(NULL, buff, "Lumi-pins SettingsFile", MB_OK);
		return;
	}



	if(NUM_ROW_ACTUAL != row_num || NUM_COL_ACTUAL != col_num){
		NUM_ROW_ACTUAL = row_num;
		NUM_COL_ACTUAL = col_num;
		boarddata->resize(row_num, col_num);
	}

	memcpy(VID_ACTUAL, input_vid, SETTINGS_TEXTBOX_VID_MAXLEN);
	memcpy(PID_ACTUAL, input_pid, SETTINGS_TEXTBOX_PID_MAXLEN);

	COM_PORT_NUMBER_DIGITS = input_com_len;
	memcpy(COM_PORT_NUMBER, input_com, input_com_len);
	COM_PORT_NUMBER[input_com_len] = '\0';


}

void LoadSettings(){

	char exe_Path[MAX_PATH];
	if(!GetModuleFileName( NULL, exe_Path, MAX_PATH )){
		MessageBox(NULL, "Error: cannot retrieve current path", "Lumi-Pins", MB_OK);
		return;
	}
	int dir_Len = (strrchr(exe_Path, '\\') - exe_Path + 1)*sizeof(*exe_Path);
	char SaveFileName[] = WINDOWS_SETTINGS_SAVE_FILE_NAME;
	char SaveFileFullPath[dir_Len+strlen(SaveFileName)];
	memcpy(SaveFileFullPath,exe_Path,dir_Len);
	memcpy(SaveFileFullPath+dir_Len, SaveFileName, strlen(SaveFileName));
	//	SaveFileFullPath[dir_Len+strlen(SaveFileName)]='\0';
	//	MessageBox(NULL, SaveFileFullPath, "Lumi-Pins", MB_OK);


	HANDLE hSettingsFile; // handle for the file
	// Attempts to open existing file and delete its content, specified by path
	hSettingsFile = CreateFile(SaveFileFullPath,                // name of the write
			GENERIC_READ,          // open for writing
			0,                      // do not share
			NULL,                   // default security
			OPEN_EXISTING,             // create new file only
			FILE_ATTRIBUTE_NORMAL,  // normal file
			NULL);                  // no attr. template


	if(hSettingsFile == INVALID_HANDLE_VALUE){

		return;
	}


	DWORD dwBytesToRead =SETTINGS_SAVEFILE_MAXLEN; // get data size in bytes
	char SettingsBuffer[SETTINGS_SAVEFILE_MAXLEN];// set memory for read buffer
	DWORD dwBytesRead = 0;

	// Attempts to read file
	bool read_result = ReadFile(
			hSettingsFile,
			SettingsBuffer,
			dwBytesToRead,
			&dwBytesRead,
			NULL
	);


	CloseHandle(hSettingsFile); // release resources

	// if read failed
	if(!read_result){
		// print error code
		DWORD ERRORc = (DWORD)GetLastError();
		char buff [100];
		sprintf(buff, "ReadFile Failed: Error %lu", ERRORc);
		MessageBox(NULL, buff, "Lumi-pins SaveFile", MB_OK);

		return;
	}

	int current_buffer_position = 0;

	int row_len = (strchr(SettingsBuffer+current_buffer_position, 0xFF) - (SettingsBuffer+current_buffer_position)) * sizeof(*SettingsBuffer);
	char row_buff[row_len+1];
	memcpy(row_buff, SettingsBuffer,row_len);
	row_buff[row_len] = '\0';
	int row_num = atoi(row_buff);
	current_buffer_position += row_len + 1;
	MessageBox(NULL, row_buff, "Lumi-pins SaveFile", MB_OK);

	char pbuff [20];
	sprintf(pbuff, "%s,%d", row_buff, row_len);
	MessageBox(NULL, pbuff, "Lumi-pins SaveFile", MB_OK);


	int col_len = (strchr(SettingsBuffer+current_buffer_position, 0xFF) - (SettingsBuffer+current_buffer_position)) * sizeof(*SettingsBuffer);
	char col_buff[row_len+1];
	memcpy(col_buff, SettingsBuffer+current_buffer_position,col_len);
	col_buff[col_len] = '\0';
	int col_num = atoi(col_buff);
	current_buffer_position += col_len + 1;

	char pbuffc [20];
	sprintf(pbuffc, "%s,%d", col_buff, col_len);
	MessageBox(NULL, pbuffc, "Lumi-pins SaveFile", MB_OK);

	int chk_len = (strchr(SettingsBuffer+current_buffer_position, 0xFF) - (SettingsBuffer+current_buffer_position)) * sizeof(*SettingsBuffer);
	char chk_buff[chk_len+1];
	memcpy(chk_buff, SettingsBuffer+current_buffer_position,chk_len);
	row_buff[chk_len] = '\0';
	current_buffer_position += chk_len + 1;

	char pbuffh [20];
	sprintf(pbuffh, "%s,%d", chk_buff, chk_len);
	MessageBox(NULL, pbuffh, "Lumi-pins SaveFile", MB_OK);

	int vid_len = (strchr(SettingsBuffer+current_buffer_position, 0xFF) - (SettingsBuffer+current_buffer_position)) * sizeof(*SettingsBuffer);
	char vid_buff[vid_len+1];
	memcpy(vid_buff, SettingsBuffer+current_buffer_position,vid_len);
	vid_buff[vid_len] = '\0';
	current_buffer_position += vid_len + 1;


	char pbuffv [20];
	sprintf(pbuffv, "%s,%d, %d", vid_buff, vid_len, strlen(vid_buff));
	MessageBox(NULL, pbuffv, "Lumi-pins SaveFile", MB_OK);

	int pid_len = (strchr(SettingsBuffer+current_buffer_position, 0xFF) - (SettingsBuffer+current_buffer_position)) * sizeof(*SettingsBuffer);
	char pid_buff[pid_len+1];
	memcpy(pid_buff, SettingsBuffer+current_buffer_position,pid_len);
	pid_buff[pid_len] = '\0';
	current_buffer_position += pid_len + 1;

	char pbuffp [20];
	sprintf(pbuffp, "%s,%d, %d", pid_buff, pid_len, strlen(vid_buff));
	MessageBox(NULL, pbuffp, "Lumi-pins SaveFile", MB_OK);



	int com_len = (strchr(SettingsBuffer+current_buffer_position, 0xFF) - (SettingsBuffer+current_buffer_position)) * sizeof(*SettingsBuffer);
	char com_buff[com_len+1];
	memcpy(com_buff, SettingsBuffer+current_buffer_position,com_len);
	com_buff[com_len] = '\0';

	char pbuffm [20];
	sprintf(pbuffm, "%s,%d", com_buff, com_len);
	MessageBox(NULL, pbuffm, "Lumi-pins SaveFile", MB_OK);

	if(row_num<1 || col_num<1 || row_num>99 || col_num>99){
		return;
	}

	if(chk_len != 1){return;}
	bool t_com_f_hid;

	if(chk_buff[0] == 'T'){
		if(com_len < 1 || com_len > 3){return;}
		t_com_f_hid = true;
	}else if(chk_buff[0] == 'F'){
		if(vid_len != 4 || pid_len != 4){return;}
		t_com_f_hid = false;
	}else{
		MessageBox(NULL, "loadsettings completeerrr", "Lumi-pins SaveFile", MB_OK);
		return;
	}

	MessageBox(NULL, "loadsettings 2 complete", "Lumi-pins SaveFile", MB_OK);
	if(NUM_ROW_ACTUAL != row_num || NUM_COL_ACTUAL != col_num){
		NUM_ROW_ACTUAL = row_num;
		NUM_COL_ACTUAL = col_num;
		boarddata->resize(row_num, col_num);
	}


	USE_T_COM_F_HID = t_com_f_hid;

	memcpy(VID_ACTUAL, vid_buff, 4);
	memcpy(PID_ACTUAL, pid_buff, 4);

	COM_PORT_NUMBER_DIGITS = com_len;
	memcpy(COM_PORT_NUMBER, com_buff, com_len);
	COM_PORT_NUMBER[com_len]='\0';

	MessageBox(NULL, "loadsettings complete", "Lumi-pins SaveFile", MB_OK);


}

