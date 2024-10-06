#ifndef __EN_BUFFER__
#define __EN_BUFFER__

#include <stdio.h>
#include <string>

using namespace std;

class en_buffer{
public:
	string str;
	int idx;
	
	en_buffer(){
		idx = 0;
		str = "";
	}
	void insert(char ch, int p);
	void insert(char ch);
	void clear();
	const char* cStr();	
};
#endif
