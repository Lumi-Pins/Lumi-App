/*
 * File: lumi_main.cpp
 * Created: 02/11/19
 * Authors: Zelin Liu
 * Content: Methods for BoardData class.
 *
 *
 * */

#include "BoardData.h"
#include "Macro.h"

/*
 * This is the constructor for BoardData class
 * Parameter: number of rows and columns of the LEDs
 * Return: a BoardData class object
 */
BoardData::BoardData(int num_row, int num_col){
	this->num_row = num_row;
	this->num_col = num_row;
	this->readwrite_size = sizeof(COLORREF)*this->num_row*this->num_col*2;
	LED_no_peg = std::vector<COLORREF>(num_row * num_col, RGB(0,0,0)); // initialized to all BLACK
	LED_pegged = std::vector<COLORREF>(num_row * num_col, RGB(55,55,55)); // initialized to all WHITE
}

//bool BoardData::resize(int num_row, int num_col){
//	this->num_row = num_row;
//	this->num_col = num_row;
//	this->readwrite_size = sizeof(COLORREF)*this->num_row*this->num_col*2;
//	LED_no_peg = std::vector<COLORREF>(num_row * num_col, RGB(0,0,0)); // initialized to all BLACK
//	LED_pegged = std::vector<COLORREF>(num_row * num_col, RGB(55,55,55)); // initialized to all WHITE
//
//}

/*
 * This function assigns color to a LED data
 * Parameter: column and row of LED, color to set, selector to which vector
 * Return: 1 upon success and 0 on failrue
 */
bool BoardData::set_LED(int x, int y, COLORREF color, bool selector){
	if( (x >= num_col) | (x < 0) | (y >= num_row) | (y < 0) ){ // check boundary, if failed return 0
		return 0;
	}
	if(!selector){ // assign color according to selector
		LED_no_peg[y * num_col + x] = color;
	}else{
		LED_pegged[y * num_col + x] = color;
	}
	return 1;
}

/*
 * This function fetches the color of an LED
 * Parameter: column and row of LED, selector to which vector
 * Return: the color fetched
 */
COLORREF BoardData::get_LED(int x, int y, bool selector){
	if( (x >= num_col) | (x < 0) | (y >= num_row) | (y < 0) ){ // check boundary, if failed return 0
		return 0;
	}
	if (!selector){ // get color
		return LED_no_peg[y * num_col + x];
	}else{
		return LED_pegged[y * num_col + x];
	}
}

/*
 * This function calculates the size of data
 * Parameter: None
 * Return: the size in DWORD
 */
unsigned short int BoardData::get_readwrite_size(){
	return this->readwrite_size; // this is calculated by [size of COLORREF x size of LED matrix x 2]
}

/*
 * This function writes data to binary array
 * Parameter: pointer to destination array, beginning, end
 * Return: None
 */
void BoardData::write_to_array(char* dest, unsigned short int begin, unsigned short int len){
	// boundary check
	if (begin + len > this->readwrite_size){return;}
	std::vector<COLORREF> combined;
	combined.reserve(this->readwrite_size);
	combined.insert(combined.end(), this->LED_no_peg.begin(), this->LED_no_peg.end());
	combined.insert(combined.end(), this->LED_pegged.begin(), this->LED_pegged.end());
	memcpy(dest, combined.data()+begin, len);
}

/*
 * This function reads binary array into data
 * Parameter: pointer to source array
 * Return: None
 */
void BoardData::read_from_array(char* src){
	// read into first vecotr
	for(int i = 0; i < this->num_row * this->num_col; i++){
		LED_no_peg[i] = MAKELONG(MAKEWORD((BYTE)src[i*4+0],(BYTE)src[i*4+1]), MAKEWORD((BYTE)src[i*4+2],(BYTE)src[i*4+3]));
	}
	// read into second vector
	for (int j = 0; j < this->num_row * this->num_col; j++){
		LED_pegged[j] = MAKELONG(MAKEWORD((BYTE)src[(j+this->num_row * this->num_col)*4+0],(BYTE)src[(j+this->num_row * this->num_col)*4+1]), MAKEWORD((BYTE)src[(j+this->num_row * this->num_col)*4+2],(BYTE)src[(j+this->num_row * this->num_col)*4+3]));
	}
}
