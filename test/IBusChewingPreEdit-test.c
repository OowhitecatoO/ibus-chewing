#include <string.h>
#include <glib.h>
#include "IBusChewingPreEdit.h"
#include "IBusChewingUtil.h"
#ifdef GSETTINGS_SUPPORT
#include "GSettingsBackend.h"
#elif GCONF2_SUPPORT
#include "GConf2Backend.h"
#endif
static IBusChewingPreEdit *self = NULL;
#define RESULT_STRING_LEN 1000

gchar resultString[RESULT_STRING_LEN] = "";

void free_test()
{
    ibus_chewing_pre_edit_free(self);
}

void key_press_from_key_sym(KSym keySym, KeyModifiers modifiers)
{
    if (modifiers & IBUS_SHIFT_MASK) {
	ibus_chewing_pre_edit_process_key(self, IBUS_KEY_Shift_L, 0);
    }
    switch (keySym) {
    case IBUS_KEY_Shift_L:
    case IBUS_KEY_Shift_R:
	ibus_chewing_pre_edit_process_key(self, keySym, modifiers);
	ibus_chewing_pre_edit_process_key(self, keySym,
					  modifiers | IBUS_RELEASE_MASK |
					  IBUS_SHIFT_MASK);
	break;
    default:
	ibus_chewing_pre_edit_process_key(self, keySym, modifiers);
	ibus_chewing_pre_edit_process_key(self, keySym,
					  modifiers | IBUS_RELEASE_MASK);
	break;

    }
    if (modifiers & IBUS_SHIFT_MASK) {
	ibus_chewing_pre_edit_process_key(self, IBUS_KEY_Shift_L,
					  IBUS_SHIFT_MASK |
					  IBUS_RELEASE_MASK);
    }

    printf
	("key_press_from_key_sym(%x(%s),%x), buffer=|%s| outgoing=|%s|\n",
	 keySym, key_sym_get_name(keySym), modifiers,
	 ibus_chewing_pre_edit_get_pre_edit(self),
	 ibus_chewing_pre_edit_get_outgoing(self));
}

void key_press_from_string(const gchar * keySeq)
{
    gint i;
    for (i = 0; i < strlen(keySeq); i++) {
	key_press_from_key_sym((guint) keySeq[i], 0);
    }
}

void check_pre_edit(const gchar * outgoing, const gchar * pre_edit)
{
    g_assert_cmpstr(outgoing, ==,
		    ibus_chewing_pre_edit_get_outgoing(self));
    g_assert_cmpstr(pre_edit, ==,
		    ibus_chewing_pre_edit_get_pre_edit(self));
}


/* Chinese mode: "中文" (5j/ jp6) and Enter*/
void process_key_normal_test()
{
    key_press_from_string("5j/ jp6");
    check_pre_edit("", "中文");
    g_assert_cmpint(2, ==, self->wordLen);
    key_press_from_key_sym(IBUS_KEY_Return, 0);
    check_pre_edit("中文", "");
    g_assert_cmpint(0, ==, self->wordLen);

    ibus_chewing_pre_edit_clear(self);
    check_pre_edit("", "");

}

/* 他不重，他是我兄弟。 */
void process_key_text_with_symbol_test()
{
    key_press_from_string("w8 ");
    key_press_from_key_sym(IBUS_KEY_Down, 0);
    key_press_from_string("2");

    key_press_from_string("1j65j/4");
    key_press_from_key_sym(IBUS_KEY_Down, 0);
    key_press_from_string("3");

    key_press_from_key_sym(IBUS_KEY_less, IBUS_SHIFT_MASK);

    key_press_from_string("w8 g4ji3vm/ 2u4");

    key_press_from_key_sym(IBUS_KEY_greater, IBUS_SHIFT_MASK);
    key_press_from_key_sym(IBUS_KEY_Return, 0);

    check_pre_edit("他不重，他是我兄弟。", "");

    ibus_chewing_pre_edit_clear(self);
    check_pre_edit("", "");
}

/* Mix english and chinese */
/* " 這是ibus-chewing 輸入法"*/
void process_key_mix_test()
{
    key_press_from_string(" 5k4g4");
    key_press_from_key_sym(IBUS_KEY_Shift_L, 0);
    key_press_from_string("ibus-chewing ");
    key_press_from_key_sym(IBUS_KEY_Shift_L, 0);
    key_press_from_string("gj bj4z83");
    key_press_from_key_sym(IBUS_KEY_Return, 0);
    check_pre_edit(" 這是ibus-chewing 輸入法", "");

    ibus_chewing_pre_edit_clear(self);
    check_pre_edit("", "");
}

void process_key_incomplete_char_test()
{
    key_press_from_string("u");
    ibus_chewing_pre_edit_force_commit(self);
    check_pre_edit("ㄧ", "");

    ibus_chewing_pre_edit_clear(self);
    check_pre_edit("", "");
}

void process_key_buffer_full_handling_test()
{
    key_press_from_string("ji3ru8 ap6fu06u.3vul3ck6");
    key_press_from_key_sym(',', IBUS_SHIFT_MASK);
    key_press_from_string("c.4au04u.3g0 qi ");
    key_press_from_key_sym(IBUS_KEY_Return, 0);
    check_pre_edit("我家門前有小河，後面有山坡", "");

    ibus_chewing_pre_edit_clear(self);
    check_pre_edit("", "");
}

void plain_zhuyin_test()
{
    ibus_chewing_pre_edit_set_apply_property_boolean(self,
						     "plain-zhuyin", TRUE);
    g_assert(ibus_chewing_pre_edit_get_property_boolean(self, "plain-zhuyin"));

    key_press_from_string("y ");

    /* Candidate window should be shown */
    g_assert(ibus_chewing_pre_edit_has_flag(self, FLAG_TABLE_SHOW));
    /* The default is the most frequently used character, not
     * necessary "資"
     */
    /* check_pre_edit("","資"); */
    key_press_from_string("4");
    check_pre_edit("吱", "");
    /* Candidate window should be hidden */
    g_assert(!ibus_chewing_pre_edit_has_flag(self, FLAG_TABLE_SHOW));

    ibus_chewing_pre_edit_clear(self);
    check_pre_edit("", "");
}

/*  很好，*/
void plain_zhuyin_shift_symbol_test()
{
    ibus_chewing_pre_edit_set_apply_property_boolean(self,
	    "plain-zhuyin", TRUE);
    g_assert(ibus_chewing_pre_edit_get_property_boolean(self, "plain-zhuyin"));

    key_press_from_string("cp31cl31");

    /* ，*/
    key_press_from_key_sym(IBUS_KEY_less, IBUS_SHIFT_MASK);
    /* Candidate window should be shown */
    g_assert(ibus_chewing_pre_edit_has_flag(self, FLAG_TABLE_SHOW));
    key_press_from_string("1");

    /* Candidate window should be hidden */
    g_assert(!ibus_chewing_pre_edit_has_flag(self, FLAG_TABLE_SHOW));

    check_pre_edit("很好，", "");

    ibus_chewing_pre_edit_clear(self);
    check_pre_edit("", "");
}

#if 0
guint ibus_chewing_pre_edit_length(self);
guint ibus_chewing_pre_edit_word_limit(IBusChewingPreEdit * self);
guint ibus_chewing_pre_edit_word_length(IBusChewingPreEdit * self);
#define ibus_chewing_pre_edit_is_empty(self) (ibus_chewing_pre_edit_length(self) ==0)
#define ibus_chewing_pre_edit_is_full(self) (self->wordLen >= ibus_chewing_pre_edit_word_limit(self))
gchar *ibus_chewing_pre_edit_get_pre_edit(IBusChewingPreEdit * self);
void ibus_chewing_pre_edit_clear(IBusChewingPreEdit * self);
#endif

gint main(gint argc, gchar ** argv)
{
    g_test_init(&argc, &argv, NULL);
#ifdef GSETTINGS_SUPPORT
    MkdgBackend *backend =
	mkdg_g_settings_backend_new(QUOTE_ME(PROJECT_SCHEMA_ID),
				    "/desktop/ibus/engine/Chewing", NULL);
#elif GCONF2_SUPPORT
    MkdgBackend *backend =
	gconf2_backend_new("/desktop/ibus/engine", NULL);
#else
    MkdgBackend *backend = NULL;
    g_error("Flag GSETTINGS_SUPPORT or GCONF2_SUPPORT are required!");
    return 1;
#endif				/* GSETTINGS_SUPPORT */
    mkdg_log_set_level(DEBUG);
    self = ibus_chewing_pre_edit_new(backend);

    ibus_chewing_pre_edit_set_apply_property_int(self,
						 "max-chi-symbol-len", 8);

    ibus_chewing_pre_edit_set_apply_property_string(self,
						    "sel-keys",
						    "1234567890");

    ibus_chewing_pre_edit_set_apply_property_boolean(self,
						     "plain-zhuyin",
						     FALSE);

    g_assert(self != NULL);

    g_test_add_func
	("/ibus-chewing/IBusChewingPreEdit/process_key_normal_test",
	 process_key_normal_test);

    g_test_add_func
	("/ibus-chewing/IBusChewingPreEdit/process_key_text_with_symbol_test",
	 process_key_text_with_symbol_test);

    g_test_add_func
	("/ibus-chewing/IBusChewingPreEdit/process_key_mix_test",
	 process_key_mix_test);

    g_test_add_func
	("/ibus-chewing/IBusChewingPreEdit/process_key_incomplete_char_test",
	 process_key_incomplete_char_test);

    g_test_add_func
	("/ibus-chewing/IBusChewingPreEdit/process_key_buffer_full_handling_test",
	 process_key_buffer_full_handling_test);

    printf("###2 plain-zhuyin=%x\n",
	   ibus_chewing_pre_edit_get_property_boolean(self,
						      "plain-zhuyin"));

    ibus_chewing_pre_edit_set_apply_property_boolean(self,
						     "plain-zhuyin", TRUE);
    g_test_add_func
	("/ibus-chewing/IBusChewingPreEdit/plain_zhuyin_test",
	 plain_zhuyin_test);
    ibus_chewing_pre_edit_set_apply_property_boolean(self,
						     "plain-zhuyin",
						     FALSE);
    g_test_add_func
	("/ibus-chewing/IBusChewingPreEdit/plain_zhuyin_shift_symbol_test",
	 plain_zhuyin_shift_symbol_test);

    g_test_add_func
	("/ibus-chewing/IBusChewingPreEdit/free_test", free_test);
    return g_test_run();
}
