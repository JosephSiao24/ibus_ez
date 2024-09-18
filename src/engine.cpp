#include "engine.h"
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
	
	vector<guint> search_idx;
	int search_pos;
	vector<GString*> cur_dict;
	int pointer_idx;
	vector<vector<GString*>> dict_pointer;
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
static void ibus_ez_engine_preedit_erase(IBusEzEngine *ez);
static void ibus_ez_engine_update_lookup_table(IBusEzEngine *ez);
static void ibus_ez_engine_on_page_down(IBusEzEngine* ez);
static void ibus_ez_engine_search_idx_append(IBusEzEngine *ez, int num);
static void ibus_ez_engine_search_idx_clear(IBusEzEngine *ez);
static void ibus_ez_engine_dict_pointer_clear(IBusEzEngine *ez);
static void ibus_ez_engine_search_idx_erase(IBusEzEngine *ez);
//helper function
void initialDict(IBusEzEngine *ez);
static void print_cursor(IBusEzEngine *ez, string a);
G_DEFINE_TYPE(IBusEzEngine, ibus_ez_engine, IBUS_TYPE_ENGINE);
static void print_search_idx(IBusEzEngine *ez, string a);
std::string trim(const std::string& str) {
    std::string result = str;
    result.erase(std::remove_if(result.begin(), result.end(),
        [](unsigned char x){ return std::isspace(x); }), result.end());
    return result;
}
void initialDict(IBusEzEngine *ez){
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
		while(getline(ss, symbol, ' ')){
			std::string cleaned_symbol = trim(symbol);	
			int idx = stoi(cleaned_symbol);
			if(cur->next.find(idx) == cur->next.end()){
				cur->next[idx] = new dictTree();
			}
			cur = cur->next[idx];
		}
		if(cur->next.find(EZSPACE) == cur->next.end())
			cur->next[EZSPACE] = new dictTree(1);
		cur = cur->next[EZSPACE];
		cur->word.push_back(g_string_new(splited[0].c_str()));
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
	ez->pointer_idx = 0;
	//page size, cursor position, cursor visible, round
	ez->table = ibus_lookup_table_new(PAGESIZE,9,true, false);
	//initial dict
	ez->dict_zh = new dictTree;
	initialDict(ez);
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
ibus_ez_engine_commit_preedit(IBusEzEngine *ez){
	IBusText *text;
	text = ibus_text_new_from_static_string(ez->preedit->str);
	ibus_engine_commit_text((IBusEngine *)ez, text);
	g_string_assign(ez->preedit, "");
	ibus_ez_engine_search_idx_clear(ez);
	ez->cursor_pos = 0;
	ibus_ez_engine_update_preedit(ez);
	ibus_ez_engine_dict_pointer_clear(ez);
	ibus_ez_engine_search_idx_clear(ez);
}

static void
isWord(IBusEzEngine *ez, string a, dictTree* cur){
	string str = "0";
	str[0] += cur->identy;
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
	
	if(cur->identy == 0)
		return false;
	ez->cur_dict = cur->word;
	return true;
}

static void
print_cursor(IBusEzEngine *ez, string a){
	string str = "0";
	str[0] += ez->cursor_pos;
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
ibus_ez_engine_update_lookup_table(IBusEzEngine *ez){
	/*if(ez->preedit->len == 0){
		ibus_engine_hide_lookup_table((IBusEngine*)ez);
		return;
	}*/
	ibus_lookup_table_clear(ez->table);
	bool dict_exist = false;
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
		dict_exist = true;
	}
	if(dict_exist){
		ibus_ez_engine_search_idx_clear(ez);
		ibus_lookup_table_set_cursor_pos(ez->table, ez->tableIdx % PAGESIZE);
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

static void
ibus_ez_engine_search_idx_erase(IBusEzEngine *ez){
	ez->search_idx.erase(ez->search_idx.begin()+ez->search_pos-1);
	ez->search_pos -= 1;
}

static void 
ibus_ez_engine_search_idx_clear(IBusEzEngine *ez){
	ez->search_idx.clear();
	ez->search_pos = 0;
}

static void
ibus_ez_engine_hide_lookup_table(IBusEzEngine *ez){
	ibus_lookup_table_clear(ez->table);
	ibus_engine_hide_lookup_table((IBusEngine*)ez);
	ez->tableIdx = 0;
}

static void 
ibus_ez_engine_dict_pointer_clear(IBusEzEngine *ez){
	ez->dict_pointer.clear();
	ez->pointer_idx = 0;
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
				return true;
			}
			ez->shift_press = false;
			return false;
		}
		
		
		return false;
	}else{
		//press shift switch to english method
		if(keyval == IBUS_Shift_L && !(modifiers & IBUS_RELEASE_MASK)){
			ez->shift_press = true;
			return false;
		}else{
			if(ez->shift_press && modifiers & IBUS_RELEASE_MASK){
				ez->shift_press = false;
				ez->mode += 1;
				ez->mode %= 2;
				return true;
			}
			ez->shift_press = false;
		}
		if(modifiers & IBUS_RELEASE_MASK)
			return false;
		
		if(keyval >= IBUS_0 && keyval <= IBUS_9){
			guint index_num = keyval - IBUS_1 + (ez->tableIdx / PAGESIZE) * PAGESIZE;
			IBusLookupTable *table = ez->table;
			if(table && index_num < ibus_lookup_table_get_number_of_candidates(table)){
				text = ibus_lookup_table_get_candidate(table, index_num);
				//ibus_engine_commit_text((IBusEngine *)ez, text);
				ibus_ez_engine_append_preedit(ez, text);
				ibus_ez_engine_update(ez);
				ibus_ez_engine_hide_lookup_table(ez);
				if(EZDEBUG){
					print_cursor(ez, " after selected\n");
				}
				return true;
			}else{
				ibus_ez_engine_search_idx_append(ez, keyval - IBUS_0);
				text = ibus_text_new_from_static_string(basic_word[keyval - IBUS_0]->str);
				ibus_ez_engine_append_preedit(ez,text);
				return true;
			}
		}
		if( ((keyval >= IBUS_A && keyval <= IBUS_Z) || (keyval >= IBUS_a && keyval <= IBUS_z)) && !(modifiers & IBUS_CONTROL_MASK)){
			guint index_alpha = 0;
			if(keyval >= IBUS_A && keyval <= IBUS_Z){
				index_alpha = keyval - IBUS_A + 10;
			}else if(keyval >= IBUS_a && keyval <= IBUS_z){
				index_alpha = keyval - IBUS_a + 10;
			}
			ibus_ez_engine_search_idx_append(ez, index_alpha);
			text = ibus_text_new_from_static_string(basic_word[index_alpha]->str);
			ibus_ez_engine_append_preedit(ez,text);
			return true;
		}
		switch(keyval){
			case IBUS_space:{
				if(ez->search_idx.size() > 0){
					ibus_ez_engine_search_idx_append(ez, EZSPACE);
					ibus_ez_engine_update_lookup_table(ez);
					return true;
				}else if(ez->preedit->len > 0){
					ibus_ez_engine_commit_preedit(ez);
					return true;
				}else{
					return false;
				}
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
					ibus_ez_engine_update_lookup_table(ez);
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
					if(ez->pointer_idx == ez->cursor_pos && ez->pointer_idx < ez->dict_pointer.size()){
						ez->pointer_idx += 1;
					}
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
					if(ez->pointer_idx == ez->cursor_pos){
						ez->pointer_idx -= 1;
					}
					ibus_ez_engine_update_preedit(ez);
					return true;
				}else{
					return false;
				}
			}
			case IBUS_semicolon:{
				ibus_ez_engine_update_lookup_table(ez);
				return true;
			}
			case IBUS_BackSpace:{
				if(ez->cursor_pos > 0){
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
