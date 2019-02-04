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

// Forward Declarations
LRESULT CALLBACK LumiWndProc(HWND, UINT, WPARAM, LPARAM);
void CloseProgram(HWND, bool);

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
	wc0.style = CS_HREDRAW | CS_VREDRAW;	// class style, you can OR these macros
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

	// create window
	HWND hwnd = CreateWindowExA(	// returns an handle
			0, // Extra Window styles
			CLASS_NAME,	// WNDCLASSEXA name
			"LumiPins", // Top bar name
			WS_OVERLAPPEDWINDOW , // Window styles
			CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT, // X,Y,W,H
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
