#include "engine.h"

typedef struct _IBusEzEngine IBusEzEngine;
typedef struct _IBusEzEngineClass IBusEzEngineClass;

struct _IBusEzEngine{
	IBusEngine parent;

	GString *preedit;
	gint cursor_pos;
	IBusLookupTable *table;
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
static void ibus_ez_engine_update_lookup_table(IBusEzEngine *ez);

G_DEFINE_TYPE(IBusEzEngine, ibus_ez_engine, IBUS_TYPE_ENGINE);

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
	//page size, cursor position, cursor visible, round
	ez->table = ibus_lookup_table_new(9,9,true, true);
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
ibus_ez_engine_update_preedit(IBusEzEngine *ez){
	IBusText *text;
	text = ibus_text_new_from_static_string(ez->preedit->str);
	ibus_engine_update_preedit_text((IBusEngine *)ez, text, ez->cursor_pos, TRUE);
}
static void
ibus_ez_engine_commit_preedit(IBusEzEngine *ez){
	IBusText *text;
	text = ibus_text_new_from_static_string(ez->preedit->str);
	ibus_engine_commit_text((IBusEngine *)ez, text);
	g_string_assign(ez->preedit, "");
	ez->cursor_pos = 0;
}
static void
ibus_ez_engine_update_lookup_table(IBusEzEngine *ez){
	/*if(ez->preedit->len == 0){
		ibus_engine_hide_lookup_table((IBusEngine*)ez);
		return;
	}*/
	ibus_lookup_table_clear(ez->table);
	ibus_lookup_table_append_candidate(ez->table, ibus_text_new_from_string("Hello"));
	ibus_lookup_table_append_candidate(ez->table, ibus_text_new_from_string("Hi"));
	ibus_lookup_table_append_candidate(ez->table, ibus_text_new_from_string("Help"));
	ibus_engine_update_lookup_table((IBusEngine *)ez, ez->table, true);
	
}

static gboolean 
ibus_ez_engine_process_key_event(IBusEngine *engine, guint keyval, guint keycode, guint modifiers){
	IBusEzEngine *ez = (IBusEzEngine *)engine;
	IBusText *text = NULL;
	if(modifiers & IBUS_RELEASE_MASK)
		return false;
	if(keyval >= IBUS_1 && keyval <= IBUS_9){
		guint index = keyval - IBUS_1;
		IBusLookupTable *table = ez->table;
		if(table && index < ibus_lookup_table_get_number_of_candidates(table)){
			text = ibus_lookup_table_get_candidate(table, index);
			ibus_engine_commit_text((IBusEngine *)ez, text);
			ibus_ez_engine_update(ez);
			ibus_engine_hide_lookup_table((IBusEngine*)ez);
			return true;
		}
	}
	switch(keyval){
		case IBUS_space:{
			ibus_ez_engine_commit_string(ez, "HelloWorld");
			return true;
		}
		case IBUS_Left:{
			g_string_insert(ez->preedit, ez->cursor_pos, "HelloWord");
			ez->cursor_pos = ez->preedit->len;
			ibus_ez_engine_update(ez);
			return true;
		}
		case IBUS_Right:{
			if(ez->preedit->len == 0)
				return false;
			ibus_ez_engine_commit_preedit(ez);
			ibus_ez_engine_update(ez);
			return true;
		}
		case IBUS_w:{
			ibus_ez_engine_commit_string(ez, "test");
			return true;
		}
		case IBUS_h:{
			ibus_ez_engine_update_lookup_table(ez);
			return false;
		}
		case IBUS_Return:{
			ibus_ez_engine_commit_preedit(ez);
			ibus_ez_engine_update(ez);
			return true;
		}
		default:{
			return false;
		}
	}	
	return false;
}
