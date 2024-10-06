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
	str += ch;
	idx++;
}


void en_buffer::clear(){
	str = "";
	idx = 0;
}

const char* en_buffer::cStr(){
	return str.c_str();
}
