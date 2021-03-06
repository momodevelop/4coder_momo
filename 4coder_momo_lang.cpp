/* date = January 29th 2021 9:37 pm */

#ifndef FCODER_MOMO_LANG_H
#define FCODER_MOMO_LANG_H

struct Momo_Index_ParseCtx;

//~ NOTE(rjf): Indexes an entire file of a language and adds stuff to the code index.
#define MOMO_LANGUAGE_INDEXFILE(name) void name(Momo_Index_ParseCtx *ctx)
typedef MOMO_LANGUAGE_INDEXFILE(Momo_Language_IndexFile);

//~ NOTE(rjf): Initializes lexer state.
#define MOMO_LANGUAGE_LEXINIT(name) void name(void *state_ptr, String_Const_u8 contents)
typedef MOMO_LANGUAGE_LEXINIT(Momo_Language_LexInit);

//~ NOTE(rjf): Lexes an entire file to produce tokens for the language (with breaks).
#define MOMO_LANGUAGE_LEXFULLINPUT(name) b32 name(Arena *arena, Token_List *list, void *state_ptr, u64 max)
typedef MOMO_LANGUAGE_LEXFULLINPUT(Momo_Language_LexFullInput);

//~ NOTE(rjf): Figures out some language-specific contextual information given some
// position in a buffer. For example, what type I am accessing, what function I am
// calling which parameter am I accessing, etc.

struct Momo_Index_Note;
struct Momo_Language_PosContextData
{
    Momo_Language_PosContextData *next;
    Momo_Index_Note *relevant_note;
    Token *query_token;
    int argument_index;
};

#define MOMO_LANGUAGE_POSCONTEXT(name) Momo_Language_PosContextData * name(Application_Links *app, Arena *arena, Buffer_ID buffer, i64 pos)
typedef MOMO_LANGUAGE_POSCONTEXT(Momo_Language_PosContext);

//~ NOTE(rjf): Does language-specific coloring for a buffer, for special-case syntax
// highlighting and stuff. Most syntax highlighting is handled through token-base-kinds
// and index lookups, but sometimes it's useful to be able to override colors by
// language-specific token sub-kinds and flags, so that's what this hook is for.
#define MOMO_LANGUAGE_HIGHLIGHT(name) void name(Application_Links *app, Text_Layout_ID text_layout_id, Token_Array *array, Color_Table color_table)
typedef MOMO_LANGUAGE_HIGHLIGHT(Momo_Language_Highlight);

//~

struct Momo_Language
{
    Momo_Language *next;
    u64 hash;
    String_Const_u8 name;
    u64 lex_state_size;
    Momo_Language_IndexFile            *IndexFile;
    Momo_Language_LexInit              *LexInit;
    Momo_Language_LexFullInput         *LexFullInput;
    Momo_Language_PosContext           *PosContext;
    Momo_Language_Highlight            *Highlight;
};

struct Momo_Language_State
{
    b32 initialized;
    Arena arena;
    Momo_Language *language_table[4096];
};

global Momo_Language_State momo_langs = {};

internal Momo_Language *
momo_get_language_from_string(String_Const_u8 name)
{
    Momo_Language *result = 0;
    if(momo_langs.initialized)
    {
        u64 hash = table_hash_u8(name.str, name.size);
        u64 slot = hash % ArrayCount(momo_langs.language_table);
        for(Momo_Language *l = momo_langs.language_table[slot]; l; l = l->next)
        {
            if(l->hash == hash && string_match(l->name, name))
            {
                result = l;
                break;
            }
        }
    }
    return result;
}

#define momo_register_language(name, IndexFile, LexInit, LexFullInput, PosContext, Highlight, lex_state_type) _momo_register_language(name, IndexFile, (Momo_Language_LexInit *)LexInit, (Momo_Language_LexFullInput *)LexFullInput, (Momo_Language_PosContext *)PosContext, (Momo_Language_Highlight *)Highlight, sizeof(lex_state_type))

internal void
_momo_register_language(String_Const_u8 name,
                     Momo_Language_IndexFile          *IndexFile,
                     Momo_Language_LexInit            *LexInit,
                     Momo_Language_LexFullInput       *LexFullInput,
                     Momo_Language_PosContext         *PosContext,
                     Momo_Language_Highlight          *Highlight,
                     u64 lex_state_size)
{
    if(momo_langs.initialized == 0)
    {
        momo_langs.initialized = 1;
        momo_langs.arena = make_arena_system(KB(16));
    }
    
    Momo_Language *language = 0;
    u64 hash = table_hash_u8(name.str, name.size);
    u64 slot = hash % ArrayCount(momo_langs.language_table);
    for(Momo_Language *l = momo_langs.language_table[slot]; l; l = l->next)
    {
        if(l->hash == hash && string_match(l->name, name))
        {
            language = l;
            break;
        }
    }
    
    if(language == 0)
    {
        language = push_array(&momo_langs.arena, Momo_Language, 1);
        language->next = momo_langs.language_table[slot];
        momo_langs.language_table[slot] = language;
        language->hash = hash;
        language->name = push_string_copy(&momo_langs.arena, name);
        language->lex_state_size     = lex_state_size;
        language->IndexFile          = IndexFile;
        language->LexInit            = LexInit;
        language->LexFullInput       = LexFullInput;
		language->PosContext         = PosContext;
        language->Highlight          = Highlight;
    }
}

internal Momo_Language *
momo_get_language_from_buffer(Application_Links *app, Buffer_ID buffer)
{
    Momo_Language *language = 0;
    Scratch_Block scratch(app);
    String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer);
    String_Const_u8 extension = string_file_extension(file_name);
    language = momo_get_language_from_string(extension);
    return language;
}

internal void
momo_language_poscontext_push_data(Arena *arena,
                                Momo_Language_PosContextData **first_ptr,
                                Momo_Language_PosContextData **last_ptr,
                                Momo_Index_Note *note,
                                Token *query,
                                int arg_index)
{
    Momo_Language_PosContextData *first = *first_ptr;
    Momo_Language_PosContextData *last = *last_ptr;
    Momo_Language_PosContextData *func = push_array_zero(arena, Momo_Language_PosContextData, 1);
    func->relevant_note = note;
    func->query_token = query;
    func->argument_index = arg_index;
    if(last == 0)
    {
        first = last = func;
    }
    else
    {
        last->next = func;
        last = last->next;
    }
    *first_ptr = first;
    *last_ptr = last;
}

internal void
momo_language_poscontext_push_data_call(Arena *arena,
                                     Momo_Language_PosContextData **first_ptr,
                                     Momo_Language_PosContextData **last_ptr,
                                     String_Const_u8 string, int param_idx)
{
    momo_language_poscontext_push_data(arena, first_ptr, last_ptr, Momo_Index_LookupNote(string, 0), 0, param_idx);
}

internal void
momo_language_poscontext_push_data_dot(Arena *arena,
                                    Momo_Language_PosContextData **first_ptr,
                                    Momo_Language_PosContextData **last_ptr,
                                    String_Const_u8 string, Token *query)
{
    momo_language_poscontext_push_data(arena, first_ptr, last_ptr, Momo_Index_LookupNote(string, 0), query, 0);
}

internal Token_List
Momo_Language_LexFullInput_NoBreaks(Application_Links *app, Momo_Language *language, Arena *arena, String_Const_u8 text)
{
    Token_List list = {};
    if(language != 0)
    {
        Scratch_Block scratch(app, arena);
        void *state = push_array_zero(scratch, u8, language->lex_state_size);
		language->LexInit(state, text);
        language->LexFullInput(arena, &list, state, max_u64);
    }
    return list;
}


// NOTE(rjf): Include language files here.
#include "4coder_momo_lang_cpp.cpp"


// NOTE(rjf): @momo_register_languages Register languages.
function void
momo_register_languages(void)
{
    // NOTE(rjf): C/C++
    {
        String_Const_u8 extensions[] =
        {
            S8Lit("cpp"), S8Lit("cc"), S8Lit("c"), S8Lit("cxx"),
            S8Lit("C"), S8Lit("h"), S8Lit("hpp"),
        };
        for(int i = 0; i < ArrayCount(extensions); i += 1)
        {
            momo_register_language(extensions[i],
                                momo_cpp_index_file,
                                lex_full_input_cpp_init,
                                lex_full_input_cpp_breaks,
                                momo_cpp_poscontext,
                                momo_cpp_highlight,
                                Lex_State_Cpp);
        }
    }
    
#if 0
    // NOTE(rjf): Jai
    {
        momo_register_language(S8Lit("jai"),
                            Momo_Jai_IndexFile,
                            lex_full_input_jai_init,
                            lex_full_input_jai_breaks,
                            Momo_Jai_PosContext,
                            Momo_Jai_Highlight,
                            Lex_State_Jai);
    }
#endif
}

#endif 
