
internal void
momo_cpp_parse_macro(Momo_Index_ParseCtx *ctx)
{
    Token *name = 0;
    if(Momo_Index_ParsePattern(ctx, "%k", TokenBaseKind_Identifier, &name))
    {
        Momo_Index_MakeNote(ctx->app, ctx->file, 0, Momo_Index_StringFromToken(ctx, name),
                          MOMO_INDEX_NOTE_KIND_MACRO, 0, Ii64(name));
        Momo_Index_SkipSoftTokens(ctx, 1);
    }
}

internal b32
momo_cpp_parse_skippable_content(Momo_Index_ParseCtx *ctx)
{
    b32 body_found = 0;
    int nest = 0;
    
    for(;!ctx->done;)
    {
        Token *name = 0;
        
        if(Momo_Index_ParsePattern(ctx, "%k", TokenBaseKind_Comment, &name))
        {
            Momo_Index_ParseComment(ctx, name);
        }
        else if(Momo_Index_ParsePattern(ctx, "%b", TokenCppKind_PPDefine, &name))
        {
            momo_cpp_parse_macro(ctx);
        }
        else if(Momo_Index_ParsePattern(ctx, "%t", "{"))
        {
            nest += 1;
            body_found = 1;
        }
        else if(Momo_Index_ParsePattern(ctx, "%t", "}"))
        {
            nest -= 1;
            if(nest == 0)
            {
                break;
            }
        }
        else if(body_found == 0)
        {
            break;
        }
        else
        {
            Momo_Index_ParseCtx_Inc(ctx, MOMO_INDEX_TOKEN_SKIP_FLAG_WHITESPACE);
        }
    }
    return body_found;
}

function void
momo_cpp_parse_struct(Momo_Index_ParseCtx *ctx, Momo_Index_Note_Flags note_flags)
{
    Token *name = 0;
    b32 valid = 0;
    b32 need_end_name = 0;
    
    if(Momo_Index_ParsePattern(ctx, "%k", TokenBaseKind_Identifier, &name))
    {
        valid = 1;
    }
    else
    {
        need_end_name = 1;
    }
    
    if(momo_cpp_parse_skippable_content(ctx))
    {
    }
    else
    {
        note_flags |= MOMO_INDEX_NOTE_FLAG_PROTOTYPE;
    }
    
    if(need_end_name)
    {
        if(Momo_Index_ParsePattern(ctx, "%k", TokenBaseKind_Identifier, &name))
        {
            valid = 1;
        }
    }
    
    if(valid)
    {
        Momo_Index_MakeNote(ctx->app, ctx->file, 0, Momo_Index_StringFromToken(ctx, name),
                          MOMO_INDEX_NOTE_KIND_TYPE, note_flags, Ii64(name));
    }
}

function b32
momo_cpp_parse_function_body(Momo_Index_ParseCtx *ctx, b32 *prototype_ptr)
{
    b32 valid = 0;
    b32 prototype = 0;
    
    for(;!ctx->done;)
    {
        Token *token = token_it_read(&ctx->it);
        if(token == 0) { break; }
        if(token->sub_kind == TokenCppKind_Semicolon)
        {
            valid = 1;
            prototype = 1;
            break;
        }
        else if(token->sub_kind == TokenCppKind_ParenCl)
        {
        }
        else if(token->kind == TokenBaseKind_ScopeOpen)
        {
            valid = 1;
            break;
        }
        Momo_Index_ParseCtx_Inc(ctx, 0);
    }
    
    if(valid)
    {
        if(prototype == 0)
        {
            momo_cpp_parse_skippable_content(ctx);
        }
    }
    
    *prototype_ptr = prototype;
    
    return valid;
}

function void
momo_cpp_parse_enum_body(Momo_Index_ParseCtx *ctx)
{
    if(Momo_Index_ParsePattern(ctx, "%t", "{"))
    {
        for(;!ctx->done;)
        {
            Token *constant = 0;
            if(Momo_Index_ParsePattern(ctx, "%k%t", TokenBaseKind_Identifier, &constant, ","))
            {
                Momo_Index_MakeNote(ctx->app, ctx->file, 0, Momo_Index_StringFromToken(ctx, constant), MOMO_INDEX_NOTE_KIND_CONSTANT, 0, Ii64(constant));
            }
            else if(Momo_Index_ParsePattern(ctx, "%k%t", TokenBaseKind_Identifier, &constant, "="))
            {
                Momo_Index_MakeNote(ctx->app, ctx->file, 0, Momo_Index_StringFromToken(ctx, constant), MOMO_INDEX_NOTE_KIND_CONSTANT, 0, Ii64(constant));
                
                for(;!ctx->done;)
                {
                    Token *token = token_it_read(&ctx->it);
                    if(token->kind == TokenBaseKind_StatementClose)
                    {
                        Momo_Index_ParseCtx_Inc(ctx, 0);
                        break;
                    }
                    else if(token->kind == TokenBaseKind_ScopeClose ||
                            token->kind == TokenBaseKind_ScopeOpen)
                    {
                        break;
                    }
                    Momo_Index_ParseCtx_Inc(ctx, 0);
                }
            }
            else if(Momo_Index_ParsePattern(ctx, "%k", TokenBaseKind_Identifier, &constant))
            {
                Momo_Index_MakeNote(ctx->app, ctx->file, 0, Momo_Index_StringFromToken(ctx, constant), MOMO_INDEX_NOTE_KIND_CONSTANT, 0, Ii64(constant));
            }
            else if(Momo_Index_ParsePattern(ctx, "%t", "}"))
            {
                break;
            }
            else
            {
                Momo_Index_ParseCtx_Inc(ctx, 0);
            }
        }
    }
}

internal MOMO_LANGUAGE_INDEXFILE(momo_cpp_index_file)
{
    int scope_nest = 0;
    for(b32 handled = 0; !ctx->done;)
    {
        handled = 0;
        
        Token *name = 0;
        Token *base_type = 0;
        Momo_Index_Note *containing_struct = 0;
        Momo_Index_Note *note = 0;
        
        if(0){}
        
        //~ NOTE(rjf): Extern "C" scope changes (ignore) ((dude C++ syntax is so fucked up))
        // NOTE(rjf): CORRECTION: Text files in general are so fucked up, fuck all of this
        // parsing bullshit
        else if(Momo_Index_ParsePattern(ctx, "%t%t%t", "extern", "C", "{"))
        {
            handled = 1;
        }
        
        //~ NOTE(rjf): Scope Nest Changes
        else if(Momo_Index_ParsePattern(ctx, "%t", "{"))
        {
            handled = 1;
            scope_nest += 1;
        }
        else if(Momo_Index_ParsePattern(ctx, "%t", "}"))
        {
            handled = 1;
            scope_nest -= 1;
            if(scope_nest < 0)
            {
                scope_nest = 0;
            }
        }
        
        //~ NOTE(rjf): Structs
        else if(Momo_Index_ParsePattern(ctx, "%t", "struct"))
        {
            handled = 1;
            momo_cpp_parse_struct(ctx, MOMO_INDEX_NOTE_FLAG_PRODUCT_TYPE);
        }
        else if(Momo_Index_ParsePattern(ctx, "%t%t", "typedef", "struct"))
        {
            handled = 1;
            momo_cpp_parse_struct(ctx, 0);
            if (Momo_Index_ParsePattern(ctx, "%k", TokenBaseKind_Identifier, &name))
            {
                Momo_Index_MakeNote(ctx->app, ctx->file, 0, Momo_Index_StringFromToken(ctx, name),
                                  MOMO_INDEX_NOTE_KIND_TYPE, MOMO_INDEX_NOTE_FLAG_PRODUCT_TYPE, Ii64(name));
            }
        }
        
        //~ NOTE(rjf): Unions
        else if(Momo_Index_ParsePattern(ctx, "%t", "union"))
        {
            handled = 1;
            momo_cpp_parse_struct(ctx, MOMO_INDEX_NOTE_FLAG_SUM_TYPE);
        }
        else if (Momo_Index_ParsePattern(ctx, "%t%t", "typedef", "union"))
        {
            handled = 1;
            momo_cpp_parse_struct(ctx, MOMO_INDEX_NOTE_FLAG_SUM_TYPE);
            if (Momo_Index_ParsePattern(ctx, "%k", TokenBaseKind_Identifier, &name))
            {
                Momo_Index_MakeNote(ctx->app, ctx->file, 0, Momo_Index_StringFromToken(ctx, name),
                                  MOMO_INDEX_NOTE_KIND_TYPE, MOMO_INDEX_NOTE_FLAG_SUM_TYPE, Ii64(name));
            }
        }
        
        //~ NOTE(rjf): Typedef'd Enums
        else if(Momo_Index_ParsePattern(ctx, "%t%t%k", "typedef", "enum", TokenBaseKind_Identifier, &name) ||
                Momo_Index_ParsePattern(ctx, "%t%t", "typedef", "enum"))
        {
            handled = 1;
            b32 prototype = 0;
            b32 possible_name_at_end = name == 0;
            if(Momo_Index_ParsePattern(ctx, "%t", ";"))
            {
                prototype = 1;
            }
            if(prototype == 0)
            {
                momo_cpp_parse_enum_body(ctx);
            }
            if(possible_name_at_end)
            {
                if(Momo_Index_ParsePattern(ctx, "%k", TokenBaseKind_Identifier, &name))
                {}
            }
            if(name != 0)
            {
                Momo_Index_MakeNote(ctx->app, ctx->file, 0, Momo_Index_StringFromToken(ctx, name),
                                  MOMO_INDEX_NOTE_KIND_TYPE, prototype ? MOMO_INDEX_NOTE_FLAG_PROTOTYPE : 0, Ii64(name));
            }
        }
        
        //~ NOTE(rjf): Enums
        else if(Momo_Index_ParsePattern(ctx, "%t%k", "enum", TokenBaseKind_Identifier, &name) ||
                Momo_Index_ParsePattern(ctx, "%t", "enum"))
        {
            handled = 1;
            b32 prototype = 0;
            if(Momo_Index_ParsePattern(ctx, "%t", ";"))
            {
                prototype = 1;
            }
            if(prototype == 0)
            {
                momo_cpp_parse_enum_body(ctx);
            }
            if(name != 0)
            {
                Momo_Index_MakeNote(ctx->app, ctx->file, 0, Momo_Index_StringFromToken(ctx, name),
                                  MOMO_INDEX_NOTE_KIND_TYPE, prototype ? MOMO_INDEX_NOTE_FLAG_PROTOTYPE : 0, Ii64(name));
            }
        }
        
        //~ NOTE(rjf): Pure Typedefs
        else if(Momo_Index_ParsePattern(ctx, "%t", "typedef"))
        {
            handled = 1;
            int nest = 0;
            b32 sum_type = 0;
            for(;!ctx->done;)
            {
                if(Momo_Index_ParsePattern(ctx, "%t", "("))
                {
                    nest += 1;
                }
                else if(Momo_Index_ParsePattern(ctx, "%t", "("))
                {
                    nest -= 1;
                }
                else if(nest == 0 && Momo_Index_ParsePattern(ctx, "%k", TokenBaseKind_Identifier, &name))
                {
                    String8 namestr = Momo_Index_StringFromToken(ctx, name);
                    Momo_Index_Note *namenote = Momo_Index_LookupNote(namestr, 0);
                    if(namenote != 0 && namenote->kind == MOMO_INDEX_NOTE_KIND_TYPE &&
                       namenote->flags & MOMO_INDEX_NOTE_FLAG_SUM_TYPE)
                    {
                        sum_type = 1;
                    }
                }
                else if(Momo_Index_ParsePattern(ctx, "%t", ";"))
                {
                    break;
                }
                else 
                {
                    Momo_Index_ParseCtx_Inc(ctx, 0);
                }
            }
            if(name != 0)
            {
                Momo_Index_Note_Flags note_flags = 0;
                if(sum_type)
                {
                    note_flags |= MOMO_INDEX_NOTE_FLAG_SUM_TYPE;
                }
                Momo_Index_MakeNote(ctx->app, ctx->file, 0, Momo_Index_StringFromToken(ctx, name),
                                  MOMO_INDEX_NOTE_KIND_TYPE, note_flags, Ii64(name));
            }
        }
        
        //~ NOTE(rjf): Functions
        else if(scope_nest == 0 &&
                (Momo_Index_ParsePattern(ctx, "%k%o%k%t",
                                       TokenBaseKind_Identifier, &base_type,
                                       TokenBaseKind_Identifier, &name,
                                       "(") ||
                 Momo_Index_ParsePattern(ctx, "%k%o%k%t",
                                       TokenBaseKind_Keyword, &base_type,
                                       TokenBaseKind_Identifier, &name,
                                       "(")))
        {
            handled = 1;
            b32 prototype = 0;
            if(momo_cpp_parse_function_body(ctx, &prototype))
            {
                Momo_Index_MakeNote(ctx->app, ctx->file, 0, Momo_Index_StringFromToken(ctx, name),
                                  MOMO_INDEX_NOTE_KIND_FUNCTION, prototype ? MOMO_INDEX_NOTE_FLAG_PROTOTYPE : 0, Ii64(name));
            }
        }
        
        //~ NOTE(rjf): Member Functions
        else if(scope_nest == 0 &&
                (Momo_Index_ParsePattern(ctx, "%k%o%n%t%k%t",
                                       TokenBaseKind_Identifier, &base_type,
                                       MOMO_INDEX_NOTE_KIND_TYPE, &containing_struct,
                                       "::",
                                       TokenBaseKind_Identifier, &name,
                                       "(") ||
                 Momo_Index_ParsePattern(ctx, "%k%o%n%t%k%t",
                                       TokenBaseKind_Keyword, &base_type,
                                       MOMO_INDEX_NOTE_KIND_TYPE, &containing_struct,
                                       "::",
                                       TokenBaseKind_Identifier, &name,
                                       "(")))
        {
            handled = 1;
            b32 prototype = 0;
            if(momo_cpp_parse_function_body(ctx, &prototype))
            {
#if 0
                Momo_Index_MakeNote(ctx->app, ctx->file, containing_struct, Momo_Index_StringFromToken(ctx, name),
                                  MOMO_INDEX_NOTE_KIND_FUNCTION, prototype ? MOMO_INDEX_NOTE_FLAG_PRODUCT_TYPE : 0, Ii64(name));
#endif

                u8 buffer[512];
                String_u8 str = Su8(buffer, 0, ArrayCount(buffer));
                if (containing_struct != 0) {
                    string_append(&str, containing_struct->string);
                    string_append(&str, string_u8_litexpr("::"));
                    string_append(&str, Momo_Index_StringFromToken(ctx, name));
                    Momo_Index_MakeNote(ctx->app, ctx->file, 0, str.string,
                        MOMO_INDEX_NOTE_KIND_FUNCTION, prototype ? MOMO_INDEX_NOTE_FLAG_PRODUCT_TYPE : 0, Ii64(name));
                }
                else {
                    Momo_Index_MakeNote(ctx->app, ctx->file, 0, Momo_Index_StringFromToken(ctx,name),
                        MOMO_INDEX_NOTE_KIND_FUNCTION, prototype ? MOMO_INDEX_NOTE_FLAG_PRODUCT_TYPE : 0, Ii64(name));
                }

              
            }
        }
        
        //~ NOTE(rjf): Declarations
        else if(scope_nest == 0 &&
                (Momo_Index_ParsePattern(ctx, "%k%o%k%o%t",
                                       TokenBaseKind_Identifier, &base_type,
                                       TokenBaseKind_Identifier, &name,
                                       ";") ||
                 Momo_Index_ParsePattern(ctx, "%k%o%k%o%t",
                                       TokenBaseKind_Keyword, &base_type,
                                       TokenBaseKind_Identifier, &name,
                                       ";") ||
                 Momo_Index_ParsePattern(ctx, "%k%o%k%t",
                                       TokenBaseKind_Identifier, &base_type,
                                       TokenBaseKind_Identifier, &name,
                                       "=") ||
                 Momo_Index_ParsePattern(ctx, "%k%o%k%t",
                                       TokenBaseKind_Keyword, &base_type,
                                       TokenBaseKind_Identifier, &name,
                                       "=")))
        {
            handled = 1;
            Momo_Index_MakeNote(ctx->app, ctx->file, 0, Momo_Index_StringFromToken(ctx, name), MOMO_INDEX_NOTE_KIND_DECL, 0, Ii64(name));
        }
        
        //~ NOTE(rjf): Macro Functions
        else if(0 && Momo_Index_ParsePattern(ctx, "%n%t%k",
                                           MOMO_INDEX_NOTE_KIND_MACRO, &note,
                                           "(",
                                           TokenBaseKind_Identifier, &name))
        {
            b32 valid = 0;
            b32 prototype = 0;
            
            for(;!ctx->done;)
            {
                Token *token = token_it_read(&ctx->it);
                if(token == 0) { break; }
                if(token->sub_kind == TokenCppKind_Semicolon)
                {
                    prototype = 1;
                    valid = 1;
                    break;
                }
                else if(token->sub_kind == TokenCppKind_ParenCl)
                {
                }
                else if(token->kind == TokenBaseKind_ScopeOpen)
                {
                    valid = 1;
                    break;
                }
                Momo_Index_ParseCtx_Inc(ctx, 0);
            }
            
            if(valid)
            {
                handled = 1;
                Momo_Index_MakeNote(ctx->app, ctx->file, 0, Momo_Index_StringFromToken(ctx, name),
                                  MOMO_INDEX_NOTE_KIND_FUNCTION, prototype ? MOMO_INDEX_NOTE_FLAG_PRODUCT_TYPE : 0, Ii64(name));
                momo_cpp_parse_skippable_content(ctx);
            }
        }
        
        //~ NOTE(rjf): Comment Tags
        else if(Momo_Index_ParsePattern(ctx, "%k", TokenBaseKind_Comment, &name))
        {
            handled = 1;
            Momo_Index_ParseComment(ctx, name);
        }
        
        //~ NOTE(rjf): Macros
        else if(Momo_Index_ParsePattern(ctx, "%b", TokenCppKind_PPDefine, &name))
        {
            handled = 1;
            momo_cpp_parse_macro(ctx);
        }
        
        
        if(handled == 0)
        {
            Momo_Index_ParseCtx_Inc(ctx, 0);
        }
    }
}

internal MOMO_LANGUAGE_POSCONTEXT(momo_cpp_poscontext)
{
    int count = 0;
    Momo_Language_PosContextData *first = 0;
    Momo_Language_PosContextData *last = 0;
    
    Token_Array tokens = get_token_array_from_buffer(app, buffer);
    Token_Iterator_Array it = token_iterator_pos(0, &tokens, pos);
    
    // NOTE(rjf): Search for left parentheses (function call or macro invocation).
    {
        int paren_nest = 0;
        int arg_idx = 0;
        for(int i = 0; count < 4; i += 1)
        {
            Token *token = token_it_read(&it);
            if(token)
            {
                if(paren_nest == 0 &&
                   token->sub_kind == TokenCppKind_ParenOp &&
                   token_it_dec_non_whitespace(&it))
                {
                    Token *name = token_it_read(&it);
                    if(name && name->kind == TokenBaseKind_Identifier)
                    {
                        momo_language_poscontext_push_data_call(arena, &first, &last, push_buffer_range(app, arena, buffer, Ii64(name)), arg_idx);
                        count += 1;
                        arg_idx = 0;
                    }
                }
                else if(token->sub_kind == TokenCppKind_ParenOp)
                {
                    paren_nest -= 1;
                }
                else if(token->sub_kind == TokenCppKind_ParenCl && i > 0)
                {
                    paren_nest += 1;
                }
                else if(token->sub_kind == TokenCppKind_Comma && i > 0 && paren_nest == 0)
                {
                    arg_idx += 1;
                }
            }
            else { break; }
            if(!token_it_dec_non_whitespace(&it))
            {
                break;
            }
        }
    }
    
    // NOTE(rjf): Search for *.* pattern, or *->* pattern (accessing a type)
    {
#if 0
        Token *last_query_candidate = 0;
        for(;;)
        {
            Token *token = token_it_read(&it);
            if(token)
            {
                if(token->kind == TokenBaseKind_Identifier)
                {
                    last_query_candidate = token;
                }
                else if((token->sub_kind == TokenCppKind_Dot ||
                         token->sub_kind == TokenCppKind_Arrow) &&
                        token_it_dec_non_whitespace(&it))
                {
                    Token *decl_name = token_it_read(&it);
                    if(decl_name && decl_name->kind == TokenBaseKind_Identifier)
                    {
                        
                    }
                }
            }
            else { break; }
            if(!token_it_dec_non_whitespace(&it))
            {
                break;
            }
        }
#endif
    }
    
    return first;
}

internal MOMO_LANGUAGE_HIGHLIGHT(momo_cpp_highlight)
{
}
