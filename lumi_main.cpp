#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow) {

  MessageBox(NULL, "Hello World", "Hello", MB_OK);

  return 0;
}
