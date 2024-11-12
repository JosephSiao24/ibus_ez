#include "en_buffer.h"

void en_buffer::insert(char ch, int p){
	if(p >= str.length()){
		str += ch;
		idx++;
	}else{
		str[p] = ch;
	}
}

void en_buffer::insert(char ch){
	if(idx >= str.length()){
		str += ch;
		idx++;
	}else{
		string tmp(1,ch);
		str.insert(idx, tmp);
		idx++;
	}
}

void en_buffer::l_move(){
	if(idx-1 >= 0)
		idx--;
}

void en_buffer::r_move(){
	if(idx+1 <= str.length())
		idx++;
}

void en_buffer::erase(){
	str.erase(idx-1,1);
	idx--;
}

void en_buffer::clear(){
	str = "";
	idx = 0;
}

const char* en_buffer::cStr(){
	return str.c_str();
}
