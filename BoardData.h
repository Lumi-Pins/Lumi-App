/*
 * File: lumi_main.cpp
 * Created: 02/11/19
 * Authors: Zelin Liu
 * Content: Header for BoardData class.
 *
 *
 * */

#ifndef BOARDDATA_H_
#define BOARDDATA_H_

#include <vector>
#include <windows.h>



class BoardData
{
private:
	int num_row, num_col;
	std::vector<COLORREF> LED_no_peg;
	std::vector<COLORREF> LED_pegged;
	unsigned int readwrite_size;
public:
	BoardData(int num_row, int num_col);

//	bool resize(int num_row, int num_col);

	bool set_LED(int x, int y, COLORREF color, bool selector);
	COLORREF get_LED(int x, int y, bool selector);
	unsigned int get_readwrite_size();
	bool write_to_array(char* dest, unsigned int begin, unsigned int len);
	bool read_from_array(char* src);
};


#endif
