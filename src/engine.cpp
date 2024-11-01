#include "engine.h"
#include "en_buffer.h"
#include "dict.h"
#include <vector>
#include <cstring>
#include <cwchar>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <memory>
typedef struct _IBusEzEngine IBusEzEngine;
typedef struct _IBusEzEngineClass IBusEzEngineClass;

struct _IBusEzEngine{
	IBusEngine parent;

	GString *preedit;
	gint cursor_pos;
	IBusLookupTable *table;
	gint mode;
	gint tableIdx;
	gboolean shift_press;
	dictTree* dict_zh;
	dictTree* dict_en;
	
	vector<guint> search_idx;
	int search_pos;
	int unmatch_buffer_len;
	
	vector<GString*> cur_dict;
	int pointer_idx;
	vector<vector<GString*>> dict_pointer;
	
	en_buffer* unmatch_en;
};

struct _IBusEzEngineClass{
	IBusEngineClass parent;
};
//functiond declare
static gboolean ibus_ez_engine_process_key_event(IBusEngine *engine, guint keyval, guint keycode, guint modifiers);
static void ibus_ez_engine_class_init(IBusEzEngineClass *klass);
static void ibus_ez_engine_init(IBusEzEngine *ez);
static void ibus_ez_engine_destroy(IBusEzEngine* ez);

//mainly function
static void ibus_ez_engine_commit_string(IBusEzEngine *ez, const gchar *string);
static void ibus_ez_engine_update(IBusEzEngine *ez);
static void ibus_ez_engine_update_preedit(IBusEzEngine *ez);
static void ibus_ez_engine_insert_preedit(IBusEzEngine *ez, IBusText * text);
static void ibus_ez_engine_preedit_erase(IBusEzEngine *ez);
static void ibus_ez_engine_update_lookup_table(IBusEzEngine *ez);
static void ibus_ez_engine_on_page_down(IBusEzEngine* ez);
static void ibus_ez_engine_search_idx_append(IBusEzEngine *ez, int num);
static void ibus_ez_engine_search_idx_clear(IBusEzEngine *ez);
static void ibus_ez_engine_dict_pointer_clear(IBusEzEngine *ez);
static void ibus_ez_engine_search_idx_erase(IBusEzEngine *ez);
static void ibus_ez_engine_hide_lookup_table(IBusEzEngine *ez);
static bool ibus_ez_engine_check_en_match(IBusEzEngine *ez);
//helper function
void initial_dict_zh(IBusEzEngine *ez);
void initial_dict_en(IBusEzEngine *ez);
static void print_cursor(IBusEzEngine *ez, string a);
G_DEFINE_TYPE(IBusEzEngine, ibus_ez_engine, IBUS_TYPE_ENGINE);
static void print_search_idx(IBusEzEngine *ez, string a);
std::string trim(const std::string& str) {
    std::string result = str;
    result.erase(std::remove_if(result.begin(), result.end(),
        [](unsigned char x){ return std::isspace(x); }), result.end());
    return result;
}
void initial_dict_zh(IBusEzEngine *ez){
	std::ifstream file("/home/joseph/Desktop/HelloWorldProject/outCsv.csv");
	string line;
	while(getline(file,line)){
		
		stringstream ss(line);
		string field;
		vector<string> splited;
		while(getline(ss, field, ',')){
			splited.push_back(field);
			
		}
		ss.clear();
		ss.str(splited[2]);
		string symbol;
		dictTree* cur = ez->dict_zh;
		int idx = 0;
		while(getline(ss, symbol, ' ')){
			std::string cleaned_symbol = trim(symbol);	
			idx = stoi(cleaned_symbol);
			if(cur->next.find(idx) == cur->next.end()){
				cur->next[idx] = new dictTree();
			}
			cur = cur->next[idx];
		}
		//if the end wasn't tone, then create a  space node behind current node
		if(!(idx == 3 || idx == 4 || idx == 6 || idx == 7)){
			if(cur->next.find(EZSPACE) == cur->next.end())
				cur->next[EZSPACE] = new dictTree(1);
			cur = cur->next[EZSPACE];
		}
		//insert the word array in the leaf node
		cur->word.push_back(g_string_new(splited[0].c_str()));
		//this line ineffcient, may consider remove
		cur->id = 1;
	}
	file.close();
	
}

void initial_dict_en(IBusEzEngine *ez){
	std::ifstream file("/home/joseph/Desktop/HelloWorldProject/en_dict.csv");
	string line;
	while(getline(file,line)){
		dictTree* cur = ez->dict_en;
		std::transform(line.begin(), line.end(), line.begin(),
                   		[](unsigned char c){ return std::tolower(c); });
		for(int i=0; i<line.length(); i++){
			int key = line[i] - 'a';
			if(cur->next.find(key) == cur->next.end()){
				cur->next[key] = new dictTree();
			}
			cur = cur->next[key];
		}
		cur->id = 1;
	}
	file.close();
	
}

static void 
ibus_ez_engine_on_page_down(IBusEzEngine* ez){
	if(ez->tableIdx % PAGESIZE == 0)
		ibus_lookup_table_page_down(ez->table);
	ibus_lookup_table_set_cursor_pos(ez->table, ez->tableIdx);
	ibus_engine_update_lookup_table((IBusEngine *)ez, ez->table, true);
}

static void
ibus_ez_engine_on_page_up(IBusEzEngine* ez){
	if(ez->tableIdx % PAGESIZE == 0)
		ibus_lookup_table_page_up(ez->table);
	ibus_lookup_table_set_cursor_pos(ez->table, ez->tableIdx);
	ibus_engine_update_lookup_table((IBusEngine *)ez, ez->table, true);
}

static void
ibus_ez_engine_class_init(IBusEzEngineClass *klass){
	IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS(klass);
	IBusEngineClass *engine_class = IBUS_ENGINE_CLASS(klass);
	
	
	
	ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_ez_engine_destroy;
	engine_class->process_key_event = ibus_ez_engine_process_key_event;
}

static void
ibus_ez_engine_init(IBusEzEngine *ez){
	ez->preedit = g_string_new("");
	ez->cursor_pos = 0;
	ez->mode = 0;
	ez->shift_press = false;
	ez->tableIdx = 0;
	ez->search_pos = 0;
	ez->unmatch_buffer_len = 0;
	ez->pointer_idx = 0;
	//page size, cursor position, cursor visible, round
	ez->table = ibus_lookup_table_new(PAGESIZE,9,true, false);
	//initial dict
	ez->dict_zh = new dictTree;
	ez->dict_en = new dictTree;
	ez->unmatch_en = new en_buffer();
	initial_dict_zh(ez);
	initial_dict_en(ez);
	g_object_ref_sink(ez->table);
}
static void
ibus_ez_engine_destroy(IBusEzEngine* ez){
	if(ez->preedit){
		g_string_free(ez->preedit, true);
		ez->preedit = NULL;
	}
	if(ez->table){
		g_object_unref(ez->table);
		ez->table = NULL;
	}
	delete ez->dict_zh;
	((IBusObjectClass *) ibus_ez_engine_parent_class)->destroy ((IBusObject*)ez);
}
static void
ibus_ez_engine_commit_string(IBusEzEngine *ez, const gchar *string){
	IBusText * text;
	text = ibus_text_new_from_static_string(string);
	ibus_engine_commit_text((IBusEngine *)ez, text);
}
static void
ibus_ez_engine_update(IBusEzEngine *ez){
	ibus_ez_engine_update_preedit(ez);
	//ibus_ez_engine_update_lookup_table(ez);
}

static void
ibus_ez_engine_preedit_erase(IBusEzEngine *ez){
	if(ez->cursor_pos <= 0)
		return;
	gchar *start = g_utf8_offset_to_pointer(ez->preedit->str, ez->cursor_pos - 1);
        gchar *end = g_utf8_next_char(start);
        gsize erase_len = end - start;
        g_string_erase(ez->preedit, start - ez->preedit->str, erase_len);
        ez->cursor_pos--;
	ibus_ez_engine_update_preedit(ez);
}

static void
ibus_ez_engine_update_preedit(IBusEzEngine *ez){
	IBusText *text;
	text = ibus_text_new_from_static_string(ez->preedit->str);
	ibus_engine_update_preedit_text((IBusEngine *)ez, text, ez->cursor_pos, TRUE);
}

static void
ibus_ez_engine_append_preedit(IBusEzEngine *ez, IBusText * text){
	
	g_string_append(ez->preedit, text->text);
	ez->cursor_pos += 1;
	ibus_ez_engine_update_preedit(ez);
	if(EZDEBUG){
		print_cursor(ez, " append preedit\n");
	}
}

static void
ibus_ez_engine_insert_preedit(IBusEzEngine *ez, IBusText * text){
	/*This function could get the actual len in utf-8 code string*/
	/*gsize len = g_utf8_strlen(ez->preedit->str, -1);*/
	
	/*Get the pointer of the ez->cursor_pos idx */
    	gchar* insert_pos = g_utf8_offset_to_pointer(ez->preedit->str, ez->cursor_pos); 
    	/*Use head pointer minus idx pointer to get offset*/
	g_string_insert(ez->preedit, insert_pos - ez->preedit->str, text->text);
	ez->cursor_pos += 1;
	ibus_ez_engine_update_preedit(ez);
}

static void
ibus_ez_engine_clear_preedit(IBusEzEngine  *ez){
	g_string_assign(ez->preedit, "");
	ez->cursor_pos = 0;
	ibus_ez_engine_update(ez);
}

static void
ibus_ez_engine_commit_preedit(IBusEzEngine *ez){
	IBusText *text;
	text = ibus_text_new_from_static_string(ez->preedit->str);
	ibus_engine_commit_text((IBusEngine *)ez, text);
	g_string_assign(ez->preedit, "");
	//ibus_ez_engine_search_idx_clear(ez);
	ez->cursor_pos = 0;
	ibus_ez_engine_update_preedit(ez);
	ibus_ez_engine_dict_pointer_clear(ez);
	//ibus_ez_engine_search_idx_clear(ez);
}

static void
isWord(IBusEzEngine *ez, string a, dictTree* cur){
	string str = "0";
	str[0] += cur->id;
	str += " ";
	str += "isWord ";
	str += a;
	ibus_ez_engine_commit_string(ez, str.c_str());
}

static bool
ibus_ez_engine_search_a_dict(IBusEzEngine *ez){
	if(ez->search_idx.size() == 0)
		return false;
	dictTree* cur = ez->dict_zh;
	if(EZDEBUG)
		isWord(ez, "In search a dict \n", cur);
		
	for(int i=0; i<ez->search_idx.size(); i++){
		if(cur->next.find(ez->search_idx[i]) != cur->next.end()){
			cur = cur->next[ez->search_idx[i]];
		}else{
			return false;
		}
	}
	
	if(cur->id == 0)
		return false;
	ez->cur_dict = cur->word;
	return true;
}

static bool
ibus_ez_engine_check_en_match(IBusEzEngine *ez){
	if(ez->unmatch_en->str.length() == 0)
		return false;
	//ibus_ez_engine_commit_string(ez, ez->unmatch_en->cStr());
	string str = ez->unmatch_en->str;
	std::transform(str.begin(), str.end(), str.begin(),
                   		[](unsigned char c){ return std::tolower(c); });	
	dictTree* cur = ez->dict_en;
	dictTree* pre = ez->dict_en;
	for(char ch : str){
		int key = ch - 'a';
		if(cur->next.find(key) == cur->next.end())
			return false;//cur->id == 1;
		pre = cur;
		cur = cur->next[key];
	}
	return cur->id == 1;// && pre->id == 1;
}

static bool
ibus_ez_engine_check_en_exist(IBusEzEngine *ez){
	if(ez->unmatch_en->str.length() == 0)
		return false;
	//ibus_ez_engine_commit_string(ez, ez->unmatch_en->cStr());
	string str = ez->unmatch_en->str;
	std::transform(str.begin(), str.end(), str.begin(),
                   		[](unsigned char c){ return std::tolower(c); });	
	dictTree* cur = ez->dict_en;
	dictTree* pre = ez->dict_en;
	for(char ch : str){
		int key = ch - 'a';
		if(cur->next.find(key) == cur->next.end())
			return false;//cur->id == 1;
		pre = cur;
		cur = cur->next[key];
	}
	return true;// && pre->id == 1;
}

static void
print_cursor(IBusEzEngine *ez, string a){
	string str = "0";
	str[0] += ez->cursor_pos;
	str += " ";
	str += "preedit len ";
	str += to_string(ez->preedit->len);
	str += " ";
	str += a;
	ibus_ez_engine_commit_string(ez, str.c_str());
}

static void
print_search_idx(IBusEzEngine *ez, string a){
	string str = "0";
	str[0] += ez->search_idx.size();
	str += " ";
	str += "search_idx ";
	str += a;
	ibus_ez_engine_commit_string(ez, str.c_str());
}

static void
print_dict_pointer(IBusEzEngine *ez, string a){
	string str = "0";
	str[0] += ez->dict_pointer.size();
	str += " ";
	str += "dict_pointer ";
	str += to_string(ez->pointer_idx);
	str += " pointer_idx ";
	str += to_string(ez->search_idx.size());
	str += " search_idx ";
	str += to_string(ez->cursor_pos);
	str += " cursor_pos ";
	/*for(int i=0; i< ez->dict_pointer.size(); i++){
		str += to_string(ez->dict_pointer[i].size());
		str += " pointer_dict:";
		str += to_string(i);
		str += " ";
	}*/
	str += a;
	ibus_ez_engine_commit_string(ez, str.c_str());
}
/*Some logic of this function is messed, but don't fix it now*/
static void
ibus_ez_engine_update_lookup_table(IBusEzEngine *ez){
	/*if(ez->preedit->len == 0){
		ibus_engine_hide_lookup_table((IBusEngine*)ez);
		return;
	}*/
	ibus_lookup_table_clear(ez->table);
	bool dict_exist = false;
	//print_dict_pointer(ez, " In lookup table\n");
	if(ez->pointer_idx <= ez->dict_pointer.size() && ez->dict_pointer.size() > 0 && ez->search_idx.size() == 0){
		ez->cur_dict = ez->dict_pointer[ez->pointer_idx-1];
		for(int i = 0; i < ez->cur_dict.size(); i++){
			ibus_lookup_table_append_candidate(ez->table, ibus_text_new_from_string((ez->cur_dict[i]->str)));
		}
		ibus_ez_engine_preedit_erase(ez);
		dict_exist = true;
	}else if(ibus_ez_engine_search_a_dict(ez)){
		if(EZDEBUG){
			print_cursor(ez, " In new dict\n");
		}
		for(int i = 0; i < ez->cur_dict.size(); i++){
			ibus_lookup_table_append_candidate(ez->table, ibus_text_new_from_string((ez->cur_dict[i]->str)));
		}
		//DEBUG MESSAGE
		if(EZDEBUG){
			print_search_idx(ez, " In lookup table before erase\n");
		}
		//erase preedit
		for(int i=0; i<ez->search_idx.size()-1; i++){
			if(EZDEBUG){
				print_cursor(ez, "In erase\n");
			}
			ibus_ez_engine_preedit_erase(ez);
		}
		//this step for reference words after selected candidate
		ez->dict_pointer.push_back(ez->cur_dict);
		ez->pointer_idx += 1;
		
		/*choose first candidate*/
		IBusText* text = ibus_lookup_table_get_candidate(ez->table, 0);
		//ibus_engine_commit_text((IBusEngine *)ez, text);
		ibus_ez_engine_append_preedit(ez, text);
		ibus_ez_engine_update(ez);
		ibus_ez_engine_hide_lookup_table(ez);
		
		ibus_ez_engine_search_idx_clear(ez);
		ez->unmatch_buffer_len = 0;
		/*next line is useless but not remove now*/
		
		//dict_exist = true;
	}
	if(dict_exist){
		ez->unmatch_buffer_len = 0;
		ibus_ez_engine_search_idx_clear(ez);
		//make sure it start at first page
		ibus_lookup_table_set_cursor_pos(ez->table, 0);
		
		ibus_engine_update_lookup_table((IBusEngine *)ez, ez->table, true);
	}else{
		ibus_ez_engine_search_idx_erase(ez);
	}
}
static void
ibus_ez_engine_search_idx_append(IBusEzEngine *ez, int num){
	ez->search_idx.push_back(num);
	ez->search_pos += 1;
}



/*while same kind of phonetic mutiple times, the same phonetic must be replaced*/
static void
ibus_ez_engine_search_idx_insert(IBusEzEngine *ez, int num){
	/*Search the position to replace or insert*/
	int insert_pos = ez->search_idx.size(); //defualt is append
	for(int i=0; i < ez->search_idx.size(); i++){
		/*If figure out same type that aleardy exist in search_idx*/
		if(phonetic_type[ez->search_idx[i]] == phonetic_type[num]){
			insert_pos = i;
			break;
		}
	}
	IBusText* text;
	/*Processing insert*/
	if(insert_pos == ez->search_idx.size()){
		ibus_ez_engine_search_idx_append(ez, num);
		text = ibus_text_new_from_static_string(basic_word[num]->str);
		ibus_ez_engine_append_preedit(ez,text);
	}else{
		//replace search_idx
		ez->search_idx[insert_pos] = num;
		//Adjust cursor position, rewriting preedit
		ez->cursor_pos = g_utf8_strlen(ez->preedit->str, -1) - ez->search_idx.size() + insert_pos + 1;
		ibus_ez_engine_preedit_erase(ez);
		text = ibus_text_new_from_static_string(basic_word[num]->str);
		ibus_ez_engine_insert_preedit(ez, text);
		//move cursor_pos to end
		ez->cursor_pos = g_utf8_strlen(ez->preedit->str, -1);
	}
	
	ibus_ez_engine_update_preedit(ez);
}

static void
ibus_ez_engine_search_idx_erase(IBusEzEngine *ez){
	//before erasing check the vector whether is empty
	if(ez->search_idx.size() > 0){
		ez->search_idx.erase(ez->search_idx.begin()+ez->search_pos-1);
		ez->search_pos -= 1;
	}
}

static void 
ibus_ez_engine_search_idx_clear(IBusEzEngine *ez){
	ez->search_idx.clear();
	ez->search_pos = 0;
	ez->unmatch_buffer_len = 0;
	ez->unmatch_en->clear();
}

static void
ibus_ez_engine_hide_lookup_table(IBusEzEngine *ez){
	ibus_lookup_table_clear(ez->table);
	ibus_engine_hide_lookup_table((IBusEngine*)ez);
	ez->tableIdx = 0;
}
static void
ibus_ez_engine_dict_pointer_erase(IBusEzEngine *ez){
	if(ez->pointer_idx > 0){
		ez->dict_pointer.erase(ez->dict_pointer.begin()+ez->pointer_idx-1);
		ez->pointer_idx -= 1;
	}
}

//not write dict_pointer_clear
static void 
ibus_ez_engine_dict_pointer_clear(IBusEzEngine *ez){
	ez->dict_pointer.clear();
	ez->pointer_idx = 0;
}

static void
ibus_ez_engine_shift_mode_clear(IBusEzEngine *ez){
	ibus_ez_engine_clear_preedit(ez);
	//ibus_ez_engine_search_idx_clear(ez);
	ez->search_idx.clear();
	ez->search_pos = 0;
	//ez->unmatch_buffer_len = 0;
	ibus_ez_engine_dict_pointer_clear(ez);
	//ez->unmatch_en->clear();
}

static void
print_unmatch_buffer_len(IBusEzEngine *ez, string a){
	string str = "0";
	str[0] += ez->unmatch_buffer_len;
	str += " ";
	str += ez->unmatch_en->str;
	str += " ";
	str += a;
	ibus_ez_engine_commit_string(ez, str.c_str());
}

static bool
ibus_ez_engine_check_str_match(string str , IBusEzEngine *ez){
	if(str.length() == 0)
		return false;
	//ibus_ez_engine_commit_string(ez, ez->unmatch_en->cStr());
	std::transform(str.begin(), str.end(), str.begin(),
                   		[](unsigned char c){ return std::tolower(c); });	
	dictTree* cur = ez->dict_en;
	dictTree* pre = ez->dict_en;
	for(char ch : str){
		int key = ch - 'a';
		if(cur->next.find(key) == cur->next.end())
			return false;//cur->id == 1;
		pre = cur;
		cur = cur->next[key];
	}
	return cur->id == 1;// && pre->id == 1;
}

vector<guint> convert_to_search_idx(string str){
	vector<guint> rtn;
	for(char ch : str){
		if(ch >= 'a' && ch <= 'z'){
			rtn.push_back(ch - 'a' + 10);
		}else if(ch >= '0' && ch <= '9'){
			rtn.push_back(ch - '0');
		}
	}
	return rtn;
}

static void
ibus_ez_engine_split_and_check(IBusEzEngine *ez){
	if(ez->unmatch_en->str.length() == 0)
		return;
	//ibus_ez_engine_commit_string(ez, ez->unmatch_en->cStr());
	string str = ez->unmatch_en->str;
	int i;
	int len = str.length();
	for(i=len-1; i>0; i--){
		str.erase(i,1);
		if(ibus_ez_engine_check_str_match(str, ez))
			break;
	}	
	if(i == 0)
		return;
	string str_search_idx = "";
	for(int j = i; j<len; j++)
		str_search_idx += ez->unmatch_en->str[j];
	ez->search_idx = convert_to_search_idx(str_search_idx);
}

static gboolean 
ibus_ez_engine_process_key_event(IBusEngine *engine, guint keyval, guint keycode, guint modifiers){
	IBusEzEngine *ez = (IBusEzEngine *)engine;
	IBusText *text = NULL;
	//只按下shift， 而没有按下任何其他键
	//mode 0 present english input method ,another is pomofo
	if(ez->mode == 0){
		//check shift key for switch input method
		if(keyval == IBUS_Shift_L && !(modifiers & IBUS_RELEASE_MASK)){
			ez->shift_press = true;
			return false;
		}else{
			if(ez->shift_press && modifiers & IBUS_RELEASE_MASK){
				ez->shift_press = false;
				ez->mode += 1;
				ez->mode %= 2;
				
				
				/*Initial the background variable*/
				ez->unmatch_en->clear();	
				ibus_ez_engine_shift_mode_clear(ez);
				return true;
			}
			ez->shift_press = false;
			if(!(modifiers & IBUS_RELEASE_MASK)){
				if( ((keyval >= IBUS_A && keyval <= IBUS_Z) || (keyval >= IBUS_a && keyval <= IBUS_z)) && !(modifiers & IBUS_CONTROL_MASK)){
					guint index_alpha = 0;
					char en_agru;
					if(keyval >= IBUS_A && keyval <= IBUS_Z){
						index_alpha = keyval - IBUS_A + 10;
						en_agru = keyval - IBUS_A + 'a';
					}else if(keyval >= IBUS_a && keyval <= IBUS_z){
						index_alpha = keyval - IBUS_a + 10;
						en_agru = keyval - IBUS_a + 'a';
					}
					ibus_ez_engine_search_idx_append(ez, index_alpha);
					ez->unmatch_en->insert(en_agru);
				}
				
				if(keyval >= IBUS_0 && keyval <= IBUS_9 || keyval == IBUS_space){
					ibus_ez_engine_search_idx_append(ez, keyval - IBUS_0);
					if(keyval != IBUS_space){
						ez->unmatch_en->insert(keyval - IBUS_0 + '0');
					}else{
						ez->unmatch_en->insert(' ');
					}
				}
				
				if(keyval == IBUS_space || keyval == IBUS_3 || keyval == IBUS_4 || keyval == IBUS_6 || keyval == IBUS_7){
					ibus_ez_engine_split_and_check(ez);
					if(ibus_ez_engine_search_a_dict(ez)){
						ez->mode += 1;
						ez->mode %= 2;
						
						//ibus_ez_engine_process_key_event(engine, IBUS_BackSpace, keycode, modifiers);
						//ibus_engine_forward_key_event(engine, IBUS_BackSpace, IBUS_KEY_BackSpace, 0);
						//ibus_engine_forward_key_event(engine, IBUS_BackSpace, IBUS_KEY_BackSpace, IBUS_RELEASE_MASK);
						//ibus_ez_engine_commit_string(ez, "\b\b");
						//print_search_idx(ez, "e\n");
						ibus_engine_delete_surrounding_text(engine, -1 * (ez->search_idx.size()-1), ez->search_idx.size()-1);
						//text = ibus_text_new_from_static_string("\b\b");
						//ibus_ez_engine_append_preedit(ez, text);
						ibus_ez_engine_update_lookup_table(ez);	
						return true;
					}else{
						ibus_ez_engine_search_idx_clear(ez);
						if(keyval == IBUS_space)
							ez->unmatch_en->clear();
					}
				}
				/*If enter project than p will match, then unmatch string being empty.*/
				/*if(!ibus_ez_engine_check_en_exist(ez)){
					ibus_ez_engine_search_idx_clear(ez);
					
					if( ((keyval >= IBUS_A && keyval <= IBUS_Z) || (keyval >= IBUS_a && keyval <= IBUS_z)) && !(modifiers & IBUS_CONTROL_MASK)){
						guint index_alpha = 0;
						if(keyval >= IBUS_A && keyval <= IBUS_Z){
							index_alpha = keyval - IBUS_A + 10;
							
						}else if(keyval >= IBUS_a && keyval <= IBUS_z){
							index_alpha = keyval - IBUS_a + 10;
						}
						ibus_ez_engine_search_idx_append(ez, index_alpha);
					}
					
					if(keyval >= IBUS_0 && keyval <= IBUS_9 && keyval == IBUS_space){
						ibus_ez_engine_search_idx_append(ez, keyval - IBUS_0);
					}
				}*/
				//print_unmatch_buffer_len(ez, "\n");
				return false;
			}
		}
		return false;
	}else{
		//press shift switch to english method
			//print_unmatch_buffer_len(ez, "every time\n");
		if(keyval == IBUS_Shift_L && !(modifiers & IBUS_RELEASE_MASK)){
			ez->shift_press = true;
			return false;
		}else{
			if(ez->shift_press && modifiers & IBUS_RELEASE_MASK){
				ez->shift_press = false;
				ez->mode += 1;
				ez->mode %= 2;
				/*Initial the background variable*/
				ez->unmatch_en->clear();
				ibus_ez_engine_shift_mode_clear(ez);
				return true;
			}
			ez->shift_press = false;
		}
		
		if(ez->unmatch_buffer_len > 5 || (ibus_ez_engine_check_en_match(ez) && ez->unmatch_buffer_len >= 4)){
			ez->mode += 1;
			ez->mode %= 2;
			
			/*Commiting already typed word, before commit to remove the phonetic symbols*/
			for(int remove_p; remove_p < ez->search_idx.size(); remove_p++){
				ibus_ez_engine_preedit_erase(ez);
			}
			text = ibus_text_new_from_static_string(ez->unmatch_en->cStr());
			ibus_ez_engine_append_preedit(ez, text);
			if(ez->cursor_pos > 0){
				ibus_ez_engine_commit_preedit(ez);
			}
			ibus_ez_engine_shift_mode_clear(ez);
			return true;
		}
		
		if(modifiers & IBUS_RELEASE_MASK)
			return false;
		
		if(keyval >= IBUS_0 && keyval <= IBUS_9){
			guint index_num = keyval - IBUS_1 + (ez->tableIdx / PAGESIZE) * PAGESIZE;
			IBusLookupTable *table = ez->table;
			if(table && index_num < ibus_lookup_table_get_number_of_candidates(table)){
				text = ibus_lookup_table_get_candidate(table, index_num);
				//ibus_engine_commit_text((IBusEngine *)ez, text);
				ibus_ez_engine_insert_preedit(ez, text);
				ibus_ez_engine_update(ez);
				ibus_ez_engine_hide_lookup_table(ez);
				if(EZDEBUG){
					print_cursor(ez, " after selected\n");
				}
				return true;
			}
		}
		if( ((keyval >= IBUS_A && keyval <= IBUS_Z) || (keyval >= IBUS_a && keyval <= IBUS_z)) && !(modifiers & IBUS_CONTROL_MASK)){
			guint index_alpha = 0;
			char en_agru;
			if(keyval >= IBUS_A && keyval <= IBUS_Z){
				index_alpha = keyval - IBUS_A + 10;
				en_agru = keyval - IBUS_A + 'a';
			}else if(keyval >= IBUS_a && keyval <= IBUS_z){
				index_alpha = keyval - IBUS_a + 10;
				en_agru = keyval - IBUS_a + 'a';
			}
			/*Part of ch to en*/
			ez->unmatch_buffer_len += 1;
			ez->unmatch_en->insert(en_agru);
			
			ibus_ez_engine_search_idx_insert(ez, index_alpha);
			return true;
		}
		switch(keyval){
			case IBUS_space:{
				if(ez->search_idx.size() > 0){
					//flat tone is not added to preedit, so the preedit erase step awalys erase size of search_idx minus one length
					/*
						shift ch to en automatically
					*/
					ez->unmatch_buffer_len += 1;
					ez->unmatch_en->insert(keyval - IBUS_space + ' ');
					
					ibus_ez_engine_search_idx_append(ez, EZSPACE);
					ibus_ez_engine_update_lookup_table(ez);
					return true;
				}else if(ez->preedit->len > 0){
					text = ibus_text_new_from_static_string(" ");
					ibus_ez_engine_append_preedit(ez, text);
					ibus_ez_engine_update(ez);
					return true;
				}else{
					return false;
				}
			}
			//if it's a common symbol, continue read input
			case IBUS_1:
			case IBUS_2:
			case IBUS_5:
			case IBUS_8:
			case IBUS_9:
			case IBUS_0:{
				ez->unmatch_buffer_len += 1;
				ibus_ez_engine_search_idx_insert(ez, keyval - IBUS_0);
				return true;
			}
			//if it's a tone, then go to dict process
			case IBUS_3:
			case IBUS_4:
			case IBUS_6:
			case IBUS_7:{
				ez->unmatch_buffer_len += 1;
				ibus_ez_engine_search_idx_append(ez, keyval - IBUS_0);
				ibus_ez_engine_update_lookup_table(ez);	
				return true;
			}
			case IBUS_minus:{
				ez->unmatch_buffer_len += 1;
				ibus_ez_engine_search_idx_insert(ez, 36);
				return true;
			}
			case IBUS_semicolon:{
				ez->unmatch_buffer_len += 1;
				ibus_ez_engine_search_idx_insert(ez, 37);
				return true;
			}
			case IBUS_slash:{
				ez->unmatch_buffer_len += 1;
				ibus_ez_engine_search_idx_insert(ez, 38);
				return true;
			}
			case IBUS_comma:{
				ez->unmatch_buffer_len += 1;
				ibus_ez_engine_search_idx_insert(ez, 40);
				return true;
			}
			case IBUS_period:{
				ez->unmatch_buffer_len += 1;
				ibus_ez_engine_search_idx_insert(ez, 39);
				return true;
			}
			case IBUS_Up:{
				if(ibus_lookup_table_get_number_of_candidates(ez->table) > 0){
					if(ez->tableIdx > 0){
						ez->tableIdx -= 1;
					}
					ibus_ez_engine_on_page_up(ez);
					return true;
				}else{
					return false;
				}
			}
			case IBUS_Down:{
				if(ibus_lookup_table_get_number_of_candidates(ez->table) > 0){
					if(ez->tableIdx < ibus_lookup_table_get_number_of_candidates(ez->table)-1){
						ez->tableIdx += 1;
					}
					ibus_ez_engine_on_page_down(ez);
					return true;
				}else if(ez->dict_pointer.size() > 0){
					if(EZDEBUG)
						print_dict_pointer(ez, " In switch IBUS down\n");
					ibus_ez_engine_update_lookup_table(ez);
					return true;
				}else{
					return false;
				}
			}
			case IBUS_Right:{
				if(ibus_lookup_table_get_number_of_candidates(ez->table) > 0){
					if(ez->tableIdx / 9 < ibus_lookup_table_get_number_of_candidates(ez->table) / 9){
						ez->tableIdx = (ez->tableIdx / 9 + 1) * 9;
					}
					
					ibus_ez_engine_on_page_down(ez);
					return true;
				}else if(ez->cursor_pos < ez->preedit->len){
					ez->cursor_pos += 1;
					//move the index of search_idx
					if(ez->search_pos < ez->search_idx.size()-1){
						ez->search_pos += 1;
					}
					//this part may cause pointer dict error 
					if(ez->pointer_idx == ez->cursor_pos-1 && ez->pointer_idx < ez->dict_pointer.size()){
						ez->pointer_idx += 1;
					}
					if(EZDEBUG)
						print_dict_pointer(ez, " In switch IBUS right\n");
					ibus_ez_engine_update_preedit(ez);
					return true;
				}else{
					return false;
				}
			}
			case IBUS_Left:{
				if(ibus_lookup_table_get_number_of_candidates(ez->table) > 0){
					if(ez->tableIdx / 9 > 0){
						ez->tableIdx = (ez->tableIdx / 9 - 1) * 9;
					}
					ibus_ez_engine_on_page_up(ez);
					return true;
				}else if(ez->cursor_pos > 0){
				
					ez->cursor_pos -= 1;
					//move the index of search_idx
					if(ez->search_pos > 0)
						ez->search_pos -= 1;
					//becuase the cursor_pos was decrease so cursor_pos plus one while compare with pointer idx
					if(ez->pointer_idx == ez->cursor_pos+1){
						ez->pointer_idx -= 1;
					}
					//print_cursor(ez, " In switch IBUS left \n");
					if(EZDEBUG)
						print_dict_pointer(ez, " In switch IBUS left\n");
					ibus_ez_engine_update_preedit(ez);
					return true;
				}else{
					return false;
				}
			}
			
			case IBUS_BackSpace:{
				if(ez->cursor_pos > 0){
					if(ez->pointer_idx == ez->cursor_pos){
							ibus_ez_engine_dict_pointer_erase(ez);
					}
					ibus_ez_engine_preedit_erase(ez);
					ibus_ez_engine_search_idx_erase(ez);
					
					return true;
				}
			}
			case IBUS_Return:{
				if(ibus_lookup_table_get_number_of_candidates(ez->table) > 0){
					text = ibus_lookup_table_get_candidate(ez->table, ez->tableIdx);
					ibus_ez_engine_append_preedit(ez, text);
					ibus_ez_engine_update(ez);
					ibus_ez_engine_hide_lookup_table(ez);
					return true;
				}else if(ez->cursor_pos > 0){
					ibus_ez_engine_commit_preedit(ez);
					ibus_ez_engine_search_idx_clear(ez);
					return true;
				}else{
					return false;
				}
			}
			default:{
				return false;
			}
		}	
	}
	return false;
}
