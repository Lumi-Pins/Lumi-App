/*
 * File: lumi_main.cpp
 * Created: 02/26/19
 * Authors: Zelin Liu
 * Content: Contains macros and structs.
 *
 *
 * */

#ifndef MACRO_H_
#define MACRO_H_

#include <vector>
// Macro

// Windows
#define WINDOWS_EXTENSION_SAVE_LOAD_BINARY_FILE "lumidata"
#define WINDOWS_SETTINGS_SAVE_FILE_NAME "settings.lumiconf"
#define WINDOWS_BACKGROUND_COLOR RGB(240,240,240)

// Settings Window
#define SETTINGS_WINDOW_START_X 450
#define SETTINGS_WINDOW_START_Y 300
#define SETTINGS_WIDTH 500
#define SETTINGS_HEIGHT 500
#define SETTINGS_BUTTON_OK_X 250
#define SETTINGS_BUTTON_OK_Y 430
#define SETTINGS_BUTTON_CANCEL_X 375
#define SETTINGS_BUTTON_CANCEL_Y 430

#define SETTINGS_TEXT_HEIGHT 20
#define SETTINGS_TEXT_WIDTH_PER_CHAR 15

#define SETTINGS_CHECKBOX_HID_X 50
#define SETTINGS_CHECKBOX_HID_Y 105
#define SETTINGS_CHECKBOX_COM_X 50
#define SETTINGS_CHECKBOX_COM_Y 170

#define SETTINGS_CHECKBOX_HID_WIDTH 183
#define SETTINGS_CHECKBOX_COM_WIDTH 165

#define SETTINGS_TEXTBOX_ROW_X 80
#define SETTINGS_TEXTBOX_ROW_Y 50
#define SETTINGS_TEXTBOX_COL_X 330
#define SETTINGS_TEXTBOX_COL_Y 50
#define SETTINGS_TEXTBOX_VID_X 80
#define SETTINGS_TEXTBOX_VID_Y 135
#define SETTINGS_TEXTBOX_PID_X 250
#define SETTINGS_TEXTBOX_PID_Y 135
#define SETTINGS_TEXTBOX_COM_X 80
#define SETTINGS_TEXTBOX_COM_Y 200

#define SETTINGS_TEXTBOX_ROW_MAXLEN 2
#define SETTINGS_TEXTBOX_COL_MAXLEN 2
#define SETTINGS_TEXTBOX_VID_MAXLEN 4
#define SETTINGS_TEXTBOX_PID_MAXLEN 4
#define SETTINGS_TEXTBOX_COM_MAXLEN 3
#define SETTINGS_SAVEFILE_MAXLEN 256

#define SETTINGS_STATICTEXT_ROW_X 50
#define SETTINGS_STATICTEXT_ROW_Y 28
#define SETTINGS_STATICTEXT_COL_X 300
#define SETTINGS_STATICTEXT_COL_Y 28
#define SETTINGS_STATICTEXT_VID_X 50
#define SETTINGS_STATICTEXT_VID_Y 137
#define SETTINGS_STATICTEXT_PID_X 220
#define SETTINGS_STATICTEXT_PID_Y 137
#define SETTINGS_STATICTEXT_COM_X 47
#define SETTINGS_STATICTEXT_COM_Y 202

#define SETTINGS_STATICTEXT_ROW_LEN 180
#define SETTINGS_STATICTEXT_COL_LEN 180
#define SETTINGS_STATICTEXT_VID_LEN 30
#define SETTINGS_STATICTEXT_PID_LEN 30
#define SETTINGS_STATICTEXT_COM_LEN 30

#define IDM_SETTINGS_BUTTON_OK 401
#define IDM_SETTINGS_BUTTON_CANCEL 402
#define IDM_SETTINGS_CHECKBOX_HID 403
#define IDM_SETTINGS_CHECKBOX_COM 404


// IDM for WM_COMMAND
#define IDM_MENU_NEW 101
#define IDM_MENU_SAVE 102
#define IDM_MENU_LOAD 103
#define IDM_MENU_EXIT 104
#define IDM_MENU_SETTINGS 301
#define IDM_COLORPICKER_NEW 105
#define IDM_COLORPICKER_EDIT 106
#define IDM_COLORPICKER_REMOVE 107
#define IDM_CHECKBOX 208
#define IDM_SYNCHROIZE 209


// board
#define SUGGESTED_NUMBER_OF_ROWS 15
#define SUGGESTED_NUMBER_OF_COLS 15

#define BOARD_WIDTH 750
#define BOARD_HEIGHT 750

#define BOARD_GRID_COLOR RGB(111,111,111)
#define BOARD_BACKGROUND_COLOR RGB(233, 233, 233)
#define BOARD_GRID_WIDTH 2
#define CIRCLE_RATIO 0.80


//color picker

#define COLORPICKER_WIDTH 300
#define COLORPICKER_HEIGHT 150

#define COLORPICKER_COLOR_WIDTH 25
#define COLORPICKER_COLOR_HEIGHT 25
#define COLORPICKER_COLOR_PADDING 5

#define COLORPICKER_BUTTON_WIDTH 60
#define COLORPICKER_BUTTON_HEIGHT 25
#define COLORPICKER_BUTTON_PADDING 20

#define NUM_COLOR_PER_ROW 8
#define NUM_COLOR_ROW 2
#define NUM_COLORS NUM_COLOR_PER_ROW*NUM_COLOR_ROW
#define DEFAULT_COLOR RGB(0,0,0)

#define COLOR_PRESET { RGB(0,0,0), RGB(128,128,128), RGB(192,192,192), RGB(255,255,255), RGB(128,0,0), RGB(255,0,0), RGB(128,128,0), RGB(255,255,0),  RGB(0,128,0), RGB(0,255,0), RGB(0,128,128), RGB(0,255,255), RGB(0,0,128), RGB(0,0,255), RGB(128,0,128), RGB(255,0,255) }



// controls

#define CONTROL_WIDTH COLORPICKER_WIDTH
#define CONTROL_HEIGHT BOARD_HEIGHT-COLORPICKER_HEIGHT
#define CONTROL_CHECKBOX_PADDING 20
#define CONTROL_CHECKBOX_HEIGHT 10

#define CONTROL_BUTTON_WIDTH 100
#define CONTROL_BUTTON_HEIGHT 25

// Mouse LMB drag selector
#define MOUSE_SELECT_BOARD 1
#define MOUSE_SELECT_COLORPICKER 2
#define MOUSE_SELECT_NONE 0

// for communication
//#define SUGGESTED_ARDUINO_VID "2341"
//#define SUGGESTED_ARDUINO_PID "0042"
#define SERIAL_BEGIN_BYTE 0x936C
#define SERIAL_END_BYTE 0xC963
#define SERIAL_BATCH_SIZE 60;


// this struct is responsible for data of the color tray
struct ColorTray{
	int color_size;
	int color_index; // [-1, NUM_COLORS-1]
	COLORREF colors[NUM_COLORS];
};

// this struct is all settings of the application
struct LumiSettings{
	unsigned int NUM_ROW_ACTUAL;
	unsigned int NUM_COL_ACTUAL;
	bool USE_T_COM_F_HID;
	char VID_ACTUAL[5];
	char PID_ACTUAL[5];
	char COM_PORT_NUMBER[SETTINGS_TEXTBOX_COM_MAXLEN+1];
	int COM_PORT_NUMBER_DIGITS;

	ColorTray colortray;

	char filepath[MAX_PATH];
};




#endif
