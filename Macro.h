/*
 * File: lumi_main.cpp
 * Created: 02/26/19
 * Authors: Zelin Liu
 * Content: Contains macros and structs.
 *
 *
 * */

#ifndef MACRO_H
#define MACRO_H

#include <vector>
// Macro

// Windows
#define WINDOWS_EXTENSION_SAVE_LOAD_BINARY_FILE "lumidata"
#define WINDOWS_BACKGROUND_COLOR RGB(240,240,240)


// IDM for WM_COMMAND
#define IDM_MENU_NEW 101
#define IDM_MENU_SAVE 102
#define IDM_MENU_LOAD 103
#define IDM_MENU_EXIT 104
#define IDM_COLORPICKER_NEW 105
#define IDM_COLORPICKER_EDIT 106
#define IDM_COLORPICKER_REMOVE 107
#define IDM_CHECKBOX 208
#define IDM_SYNCHROIZE 209


// board
#define NUMBER_OF_ROWS 15
#define NUMBER_OF_COLS 15

#define BOARD_WIDTH 750
#define BOARD_HEIGHT 750

#define BOARD_BACKGROUND_COLOR RGB(233, 233, 233)
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
#define ARDUINO_VID "2341"
#define ARDUINO_PID "0042"
#define SERIAL_BEGIN_BYTE 0x936C
#define SERIAL_END_BYTE 0xC963
#define SERIAL_BATCH_SIZE 60;
//#define SERIAL_ATTEMPTS_FOR_HEADER 1
//#define SERIAL_ATTEMPTS_FOR_BODY 3
//#define SERIAL_ATTEMPTS_FOR_FINISHER 3


struct ColorTray{
	int color_index; // [-1, NUM_COLORS-1]
	std::vector<COLORREF> colors; // the colors currently on colortray, bounded by NUM_COLOR_PER_ROW * NUM_COLOR_ROW
};




#endif
