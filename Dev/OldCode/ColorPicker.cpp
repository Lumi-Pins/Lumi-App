//#include "ColorPicker.h"
//#include <algorithm>
//#include <iterator>
//
//
//
//ColorPicker::ColorPicker(){
////	this->max_color_index = 0;
//	this->colors.reserve(NUM_COLORS);
//	this->color_index = 0;
//	this->colors.push_back(DEFAULT_COLOR);
//	for(int i = 0; i < 15; i ++){
//		this->colors.push_back(RGB(i*10, i*10, i*10));
//	}
//}
//
////void ColorPicker::set_max_color_index(int entry){
////	this->max_color_index = std::max(std::min(NUM_COLORS , entry), 0); // [0, NUM_COLORS]
////}
////
////int ColorPicker::get_max_color_index(){
////	return this->max_color_index;
////}
//
//
//// pos limit [1, max_color_index+1]
//void ColorPicker::set_colors(COLORREF color){
//	this->colors[this->color_index] = color;
//}
//
//void ColorPicker::pend_colors(COLORREF color){
//	// if already full
//	if (this->colors.size() == NUM_COLORS){
//		this->colors.erase(this->colors.begin());
//	}
//	this->colors.push_back(color);
//
//}
//
//void ColorPicker::Del_colors(){
//	if(this->color_index > 0){
//		this->colors.erase(this->colors.begin() + this->color_index);
//	}
//}
//
//void ColorPicker::set_color_index(int pos){
//	if(pos >= 0 && pos < (int)this->colors.size()){
//		this->color_index = pos;
//	}
//}
//
//int ColorPicker::get_color_index(){
//	return this->color_index;
//}
//
//int ColorPicker::get_colors_size(){
//	return this->colors.size();
//}
//
//COLORREF ColorPicker::get_color(int pos){
//	if(pos >= 0 && pos <= (int)this->colors.size()){
//		return this->colors[pos];
//	}
//	return DEFAULT_COLOR;
//}
//
//COLORREF ColorPicker::get_color_index_color(){
//	if (this->color_index >= 0){
//		return this->colors[this->color_index];
//	}
//	return DEFAULT_COLOR;
//
//}
//
//void ColorPicker::copy_colors(COLORREF* colors){
//	std::copy(this->colors.begin(), this->colors.end(), colors);
//}
