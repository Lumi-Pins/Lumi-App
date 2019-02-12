#include "BoardData.h"


BoardData::BoardData(int num_row, int num_col){
	this->num_row = num_row;
	this->num_col = num_row;
	LED_no_peg = std::vector<COLORREF>(num_row * num_col, RGB(255,0,0));
	LED_pegged = std::vector<COLORREF>(num_row * num_col, RGB(0,255,0));

}

bool BoardData::set_LED(int x, int y, COLORREF color, bool selector){
	if( (x >= num_col) | (x < 0) | (y >= num_row) | (y < 0) ){
		return 0;
	}
	if(!selector){
		LED_no_peg[y * num_col + x] = color;
	}else{
		LED_pegged[y * num_col + x] = color;
	}
	return 1;
}

COLORREF BoardData::get_LED(int x, int y, bool selector){
	if( (x >= num_col) | (x < 0) | (y >= num_row) | (y < 0) ){
		return 0;
	}
	if (!selector){
		return LED_no_peg[y * num_col + x];
	}else{
		return LED_pegged[y * num_col + x];
	}
}
