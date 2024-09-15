#include "engine.h"
#include "dict.h"
#include <vector>
#include <cstring>
#include <cwchar>
#include <fstream>
#include <sstream>
#include <algorithm>
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
	vector<GString*> dict;
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
//helper function
void initialDict(IBusEzEngine *ez);

G_DEFINE_TYPE(IBusEzEngine, ibus_ez_engine, IBUS_TYPE_ENGINE);
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
		while(getline(ss, field)){
			std::string cleaned_field = trim(field);
			ez->dict.push_back(g_string_new(cleaned_field.c_str()));
		}
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
	//page size, cursor position, cursor visible, round
	ez->table = ibus_lookup_table_new(PAGESIZE,9,true, false);
	//initial dict
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
}

static void
ibus_ez_engine_commit_preedit(IBusEzEngine *ez){
	IBusText *text;
	text = ibus_text_new_from_static_string(ez->preedit->str);
	ibus_engine_commit_text((IBusEngine *)ez, text);
	g_string_assign(ez->preedit, "");
	ez->cursor_pos = 0;
	ibus_ez_engine_update_preedit(ez);
}
static void
ibus_ez_engine_update_lookup_table(IBusEzEngine *ez){
	/*if(ez->preedit->len == 0){
		ibus_engine_hide_lookup_table((IBusEngine*)ez);
		return;
	}*/
	ibus_lookup_table_clear(ez->table);
	for(int i = 0; i < ez->dict.size(); i++){
		ibus_lookup_table_append_candidate(ez->table, ibus_text_new_from_string((ez->dict[i]->str)));
	}
	ibus_engine_update_lookup_table((IBusEngine *)ez, ez->table, true);
	ibus_lookup_table_set_cursor_pos(ez->table, ez->tableIdx % PAGESIZE);
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
		
		if(keyval >= IBUS_1 && keyval <= IBUS_9){
			guint index = keyval - IBUS_1 + (ez->tableIdx / PAGESIZE) * PAGESIZE;
			IBusLookupTable *table = ez->table;
			if(table && index < ibus_lookup_table_get_number_of_candidates(table)){
				text = ibus_lookup_table_get_candidate(table, index);
				//ibus_engine_commit_text((IBusEngine *)ez, text);
				ibus_ez_engine_append_preedit(ez, text);
				ibus_ez_engine_update(ez);
				ibus_lookup_table_clear(ez->table);
				ibus_engine_hide_lookup_table((IBusEngine*)ez);
				ez->tableIdx = 0;
				return true;
			}else{
				text = ibus_text_new_from_static_string(basic_word[keyval - IBUS_0]->str);
				ibus_ez_engine_append_preedit(ez,text);
				return true;
			}
		}
		if( ((keyval >= IBUS_A && keyval <= IBUS_Z) || (keyval >= IBUS_a && keyval <= IBUS_z)) && !(modifiers & IBUS_CONTROL_MASK)){
			gint index = 0;
			if(keyval >= IBUS_A && keyval <= IBUS_Z){
				index = keyval - IBUS_A + 10;
			}else if(keyval >= IBUS_a && keyval <= IBUS_z){
				index = keyval - IBUS_a + 10;
			}
			text = ibus_text_new_from_static_string(basic_word[index]->str);
			ibus_ez_engine_append_preedit(ez,text);
			return true;
		}
		switch(keyval){
			case IBUS_space:{
				ibus_ez_engine_commit_string(ez, "HelloWorld");
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
					if(ez->tableIdx < ibus_lookup_table_get_number_of_candidates(ez->table)){
						ez->tableIdx += 1;
					}
					ibus_ez_engine_on_page_down(ez);
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
					return true;
				}
			}
			case IBUS_Return:{
				if(ibus_lookup_table_get_number_of_candidates(ez->table) > 0){
					ibus_ez_engine_on_page_down(ez);
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
