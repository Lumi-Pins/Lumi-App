#include <windows.h>

LRESULT CALLBACK LumiWndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow) {

//  MessageBox(NULL, "Error", "Lumi-Pins", MB_OK);

	// create a window class
	const char CLASS_NAME[]  = "class0";
	WNDCLASSEXA  wc0 = { };
	wc0.cbSize = sizeof(WNDCLASSEX);
	wc0.lpfnWndProc = LumiWndProc;
	wc0.hInstance     = hInstance;
	wc0.lpszClassName = CLASS_NAME;
	wc0.style = CS_HREDRAW | CS_VREDRAW;
	wc0.cbClsExtra = 0;
	wc0.cbWndExtra = 0;
	wc0.lpszMenuName = NULL;
	wc0.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc0.hCursor = LoadCursorA(NULL, IDC_ARROW);
	wc0.hIcon = LoadIcon(NULL, IDI_APPLICATION); // to do: create an icon and link its handle
	// register window class
	if (!RegisterClassExA(&wc0)){
		MessageBox(NULL, "Error: RegisterClassExA returned 0", "Lumi-Pins", MB_OK);
		return 0;
	}

	// create window
	HWND hwnd = CreateWindowExA(
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

	// check window creation
	if (hwnd == NULL){
		MessageBox(NULL, "Error: CreateWindowExA returned NULL", "Lumi-Pins", MB_OK);
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}




  return 0;
}

LRESULT CALLBACK LumiWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch(msg){
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
