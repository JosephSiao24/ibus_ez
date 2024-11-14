#include "dict.h"
std::vector<GString*> basic_word{
	g_string_new("ㄢ"), //0
	g_string_new("ㄅ"),
	g_string_new("ㄉ"), //2
	g_string_new("ˇ"),
	g_string_new("ˋ"),  //4
	g_string_new("ㄓ"),
	g_string_new("ˊ"),  //6
	g_string_new("˙"),
	g_string_new("ㄚ"), //8
	g_string_new("ㄞ"),
	g_string_new("ㄇ"), //10
	g_string_new("ㄖ"),
	g_string_new("ㄏ"), //12
	g_string_new("ㄋ"),
	g_string_new("ㄍ"), //14
	g_string_new("ㄑ"),
	g_string_new("ㄕ"), //16
	g_string_new("ㄘ"),
	g_string_new("ㄛ"), //18
	g_string_new("ㄨ"),
	g_string_new("ㄜ"), //20
	g_string_new("ㄠ"),
	g_string_new("ㄩ"), //22
	g_string_new("ㄥ"),
	g_string_new("ㄟ"), //24
	g_string_new("ㄣ"),
	g_string_new("ㄆ"), //26
	g_string_new("ㄐ"),
	g_string_new("ㄋ"), //28
	g_string_new("ㄔ"),
	g_string_new("ㄧ"), //30
	g_string_new("ㄒ"),
	g_string_new("ㄊ"), //32
	g_string_new("ㄌ"),
	g_string_new("ㄗ"), //34
	g_string_new("ㄈ"),
	g_string_new("ㄦ"), //36
	g_string_new("ㄤ"),
	g_string_new("ㄥ"), //38
	/*Error on order*/
	g_string_new("ㄡ"),
	g_string_new("ㄝ"), //40
	
};

dictTree::dictTree(){
	id = 0;
}

dictTree::dictTree(int identity){
	id = identity;
}
/*
0:聲母
1:韻母
2:介母
*/
map<int, int> phonetic_type{
	{23, 0},
	{17, 0},
	{34, 0},
	{11, 0},
	{16, 0},
	{29, 0},
	{31, 0},
	{5,0},
	{15, 0},
	{27, 0},
	{12, 0},
	{13, 0},
	{14, 0},
	{33, 0},
	{28, 0},
	{32, 0},
	{2, 0},
	{35, 0},
	{10,0},
	{26, 0},
	{1, 0},
	{36, 1},
	{38, 1},
	{37, 1},
	{25, 1},
	{24, 1},
	{8, 1},
	{18, 1},
	{20, 1},
	{39, 1},
	{9, 1},
	{21, 1},
	{40, 1},
	{0, 1},
	{30, 2},
	{19,2},
	{22,2}
};
















