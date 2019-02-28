#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <vector>
#include <windows.h>

#define NUM_COLOR_PER_ROW 8
#define NUM_COLOR_ROW 2
#define NUM_COLORS NUM_COLOR_PER_ROW*NUM_COLOR_ROW
#define DEFAULT_COLOR RGB(0,0,0)



class ColorPicker{
private:
//	int max_color_index; // [0,  NUM_COLORS]

	int color_index; // [0, NUM_COLORS-1]
	std::vector<COLORREF> colors;

public:

	ColorPicker();



//	void set_max_color_index(int entry);
//	int get_max_color_index();

	void set_colors(COLORREF color);
	void pend_colors(COLORREF color);
	void Del_colors();

	void set_color_index(int pos);
	int get_color_index();

	int get_colors_size();

	COLORREF get_color(int pos);
	COLORREF get_color_index_color();
	void copy_colors(COLORREF* destination);

};
#endif
