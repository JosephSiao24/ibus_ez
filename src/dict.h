#ifndef __DICT_H__
#define __DICT_H__

#include <ibus.h>
#include <iostream>
#include <vector>
#include <map>
using namespace std;

extern vector<GString*> basic_word;
extern map<int, int> phonetic_type;
class dictTree{
public:
	map<int, dictTree*> next;
	vector<GString*> word;
	gint id;
	
	dictTree();
	dictTree(int id);
};
#endif
