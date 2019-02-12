#ifndef BOARDDATA_H
#define BOARDDATA_H

#include <vector>
#include <windows.h>



class BoardData
{
private:
	int num_row, num_col;
	std::vector<COLORREF> LED_no_peg;
	std::vector<COLORREF> LED_pegged;
public:
	BoardData(int num_row, int num_col);

	bool set_LED(int x, int y, COLORREF color, bool selector);
	COLORREF get_LED(int x, int y, bool selector);
//	bool write_binary_to(char* destination);
};


#endif
