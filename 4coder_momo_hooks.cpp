

function void
momo_render_buffer(Application_Links *app, View_ID view_id, Face_ID face_id,
                   Buffer_ID buffer, Text_Layout_ID text_layout_id,
                   Rect_f32 rect, Frame_Info frame_info)
{
    Scratch_Block scratch(app);
    ProfileScope(app, "[Fleury] Render Buffer");
    
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    Rect_f32 prev_clip = draw_set_clip(app, rect);
    
    // NOTE(allen): Token colorizing
    Token_Array token_array = get_token_array_from_buffer(app, buffer);
    if(token_array.tokens != 0)
    {
        F4_SyntaxHighlight(app, text_layout_id, &token_array);
        
        // NOTE(allen): Scan for TODOs and NOTEs
        b32 use_comment_keywords = def_get_config_b32(vars_save_string_lit("use_comment_keywords"));
        if(use_comment_keywords)
        {
            Comment_Highlight_Pair pairs[] =
            {
                {str8_lit("NOTE"), finalize_color(defcolor_comment_pop, 0)},
                {str8_lit("TODO"), finalize_color(defcolor_comment_pop, 1)},
                {def_get_config_string(scratch, vars_save_string_lit("user_name")), finalize_color(fleury_color_comment_user_name, 0)},
            };
            draw_comment_highlights(app, buffer, text_layout_id,
                                    &token_array, pairs, ArrayCount(pairs));
        }
    }
    else
    {
        Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
        paint_text_color_fcolor(app, text_layout_id, visible_range, fcolor_id(defcolor_text_default));
    }
    
    i64 cursor_pos = view_correct_cursor(app, view_id);
    view_correct_mark(app, view_id);
    
    // NOTE(allen): Scope highlight
    b32 use_scope_highlight = def_get_config_b32(vars_save_string_lit("use_scope_highlight"));
    if (use_scope_highlight){
        Color_Array colors = finalize_color_array(defcolor_back_cycle);
        draw_scope_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
    }
    
    // NOTE(rjf): Brace highlight
    {
        Color_Array colors = finalize_color_array(fleury_color_brace_highlight);
        if(colors.count >= 1 && F4_ARGBIsValid(colors.vals[0]))
        {
            F4_Brace_RenderHighlight(app, buffer, text_layout_id, cursor_pos,
                                     colors.vals, colors.count);
        }
    }
    
    // NOTE(allen): Line highlight
    {
        b32 highlight_line_at_cursor = def_get_config_b32(vars_save_string_lit("highlight_line_at_cursor"));
        String_Const_u8 name = string_u8_litexpr("*compilation*");
        Buffer_ID compilation_buffer = get_buffer_by_name(app, name, Access_Always);
        if(highlight_line_at_cursor && (is_active_view || buffer == compilation_buffer))
        {
            i64 line_number = get_line_number_from_pos(app, buffer, cursor_pos);
            draw_line_highlight(app, text_layout_id, line_number,
                                fcolor_id(defcolor_highlight_cursor_line));
        }
    }
    
    // NOTE(rjf): Error/Search Highlight
    {
        b32 use_error_highlight = def_get_config_b32(vars_save_string_lit("use_error_highlight"));
        b32 use_jump_highlight = def_get_config_b32(vars_save_string_lit("use_jump_highlight"));
        if (use_error_highlight || use_jump_highlight){
            // NOTE(allen): Error highlight
            String_Const_u8 name = string_u8_litexpr("*compilation*");
            Buffer_ID compilation_buffer = get_buffer_by_name(app, name, Access_Always);
            if (use_error_highlight){
                draw_jump_highlights(app, buffer, text_layout_id, compilation_buffer,
                                     fcolor_id(defcolor_highlight_junk));
            }
            
            // NOTE(allen): Search highlight
            if (use_jump_highlight){
                Buffer_ID jump_buffer = get_locked_jump_buffer(app);
                if (jump_buffer != compilation_buffer){
                    draw_jump_highlights(app, buffer, text_layout_id, jump_buffer,
                                         fcolor_id(defcolor_highlight_white));
                }
            }
        }
    }
    
    // NOTE(rjf): Error annotations
    {
        String_Const_u8 name = string_u8_litexpr("*compilation*");
        Buffer_ID compilation_buffer = get_buffer_by_name(app, name, Access_Always);
        momo_render_error_annotations(app, buffer, text_layout_id, compilation_buffer);
    }
    
    // NOTE(jack): Token Occurance Highlight
    if (!def_get_config_b32(vars_save_string_lit("f4_disable_cursor_token_occurance"))) 
    {
        ProfileScope(app, "[Fleury] Token Occurance Highlight");
        
        // NOTE(jack): Get the active cursor's token string
        Buffer_ID active_cursor_buffer = view_get_buffer(app, active_view, Access_Always);
        i64 active_cursor_pos = view_get_cursor_pos(app, active_view);
        Token_Array active_cursor_buffer_tokens = get_token_array_from_buffer(app, active_cursor_buffer);
        Token_Iterator_Array active_cursor_it = token_iterator_pos(0, &active_cursor_buffer_tokens, active_cursor_pos);
        Token *active_cursor_token = token_it_read(&active_cursor_it);
        
        String_Const_u8 active_cursor_string = string_u8_litexpr("");
        if(active_cursor_token)
        {
            active_cursor_string = push_buffer_range(app, scratch, active_cursor_buffer, Ii64(active_cursor_token));
            
            // Loop the visible tokens
            Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
            i64 first_index = token_index_from_pos(&token_array, visible_range.first);
            Token_Iterator_Array it = token_iterator_index(0, &token_array, first_index);
            for (;;)
            {
                Token *token = token_it_read(&it);
                if(!token || token->pos >= visible_range.one_past_last)
                {
                    break;
                }
                
                if (token->kind == TokenBaseKind_Identifier)
                {
                    Range_i64 token_range = Ii64(token);
                    String_Const_u8 token_string = push_buffer_range(app, scratch, buffer, token_range);
                    
                    // NOTE(jack) If this is the buffers cursor token, highlight it with an Underline
                    if (range_contains(token_range, view_get_cursor_pos(app, view_id)))
                    {
                        F4_RenderRangeHighlight(app, view_id, text_layout_id,
                                                token_range, F4_RangeHighlightKind_Underline,
                                                fcolor_resolve(fcolor_id(fleury_color_token_highlight)));
                    }
                    // NOTE(jack): If the token matches the active buffer token. highlight it with a Minor Underline
                    else if(active_cursor_token->kind == TokenBaseKind_Identifier && 
                            string_match(token_string, active_cursor_string))
                    {
                        F4_RenderRangeHighlight(app, view_id, text_layout_id,
                                                token_range, F4_RangeHighlightKind_MinorUnderline,
                                                fcolor_resolve(fcolor_id(fleury_color_token_minor_highlight)));
                        
                    } 
                }
                
                if(!token_it_inc_non_whitespace(&it))
                {
                    break;
                }
            }
        }
    }
    // NOTE(jack): if "f4_disable_cursor_token_occurance" is set, just highlight the cusror 
    else
    {
        ProfileScope(app, "[Fleury] Token Highlight");
        
        Token_Iterator_Array it = token_iterator_pos(0, &token_array, cursor_pos);
        Token *token = token_it_read(&it);
        if(token && token->kind == TokenBaseKind_Identifier)
        {
            F4_RenderRangeHighlight(app, view_id, text_layout_id,
                                    Ii64(token->pos, token->pos + token->size),
                                    F4_RangeHighlightKind_Underline,
                                    fcolor_resolve(fcolor_id(fleury_color_token_highlight)));
        }
    }
        
    // NOTE(allen): Color parens
    if(def_get_config_b32(vars_save_string_lit("use_paren_helper")))
    {
        Color_Array colors = finalize_color_array(defcolor_text_cycle);
        draw_paren_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
    }
    
    // NOTE(rjf): Divider Comments
    {
        F4_RenderDividerComments(app, buffer, view_id, text_layout_id);
    }
    
    // NOTE(rjf): Cursor Mark Range
    if(is_active_view && fcoder_mode == FCoderMode_Original)
    {
        F4_HighlightCursorMarkRange(app, view_id);
    }
    
    // NOTE(allen): Cursor shape
    Face_Metrics metrics = get_face_metrics(app, face_id);
    u64 cursor_roundness_100 = def_get_config_u64(app, vars_save_string_lit("cursor_roundness"));
    f32 cursor_roundness = metrics.normal_advance*cursor_roundness_100*0.01f;
    f32 mark_thickness = (f32)def_get_config_u64(app, vars_save_string_lit("mark_thickness"));
    
    // NOTE(rjf): Cursor
    switch (fcoder_mode)
    {
        case FCoderMode_Original:
        {
            F4_Cursor_RenderEmacsStyle(app, view_id, is_active_view, buffer, text_layout_id, cursor_roundness, mark_thickness, frame_info);
        }break;
        
        case FCoderMode_NotepadLike:
        {
            F4_Cursor_RenderNotepadStyle(app, view_id, is_active_view, buffer, text_layout_id, cursor_roundness,
                                         mark_thickness, frame_info);
            break;
        }
    }
    
    // NOTE(rjf): Brace annotations
    {
        F4_Brace_RenderCloseBraceAnnotation(app, buffer, text_layout_id, cursor_pos);
    }
    
    // NOTE(rjf): Brace lines
    {
        F4_Brace_RenderLines(app, buffer, view_id, text_layout_id, cursor_pos);
    }
    
    // NOTE(allen): put the actual text on the actual screen
    draw_text_layout_default(app, text_layout_id);
    
    // NOTE(rjf): Interpret buffer as calc code, if it's the calc buffer.
    {
        Buffer_ID calc_buffer_id = get_buffer_by_name(app, string_u8_litexpr("*calc*"), AccessFlag_Read);
        if(calc_buffer_id == buffer)
        {
            F4_CLC_RenderBuffer(app, buffer, view_id, text_layout_id, frame_info);
        }
    }
    
    // NOTE(rjf): Draw calc comments.
    {
        F4_CLC_RenderComments(app, buffer, view_id, text_layout_id, frame_info);
    }
    
    draw_set_clip(app, prev_clip);
    
    // NOTE(rjf): Draw tooltips and stuff.
    if(active_view == view_id)
    {
        // NOTE(rjf): Position context helper
        {
            F4_PosContext_Render(app, view_id, buffer, text_layout_id, cursor_pos);
        }
        
        // NOTE(rjf): Draw tooltip list.
        {
            Mouse_State mouse = get_mouse_state(app);
            
            Rect_f32 view_rect = view_get_screen_rect(app, view_id);
            
            Face_ID tooltip_face_id = global_small_code_face;
            Face_Metrics tooltip_face_metrics = get_face_metrics(app, tooltip_face_id);
            
            Rect_f32 tooltip_rect =
            {
                (f32)mouse.x + 16,
                (f32)mouse.y + 16,
                (f32)mouse.x + 16,
                (f32)mouse.y + 16 + tooltip_face_metrics.line_height + 8,
            };
            
            for(int i = 0; i < global_tooltip_count; ++i)
            {
                String_Const_u8 string = global_tooltips[i].string;
                tooltip_rect.x1 = tooltip_rect.x0;
                tooltip_rect.x1 += get_string_advance(app, tooltip_face_id, string) + 4;
                
                if(tooltip_rect.x1 > view_rect.x1)
                {
                    f32 difference = tooltip_rect.x1 - view_rect.x1;
                    tooltip_rect.x1 = (float)(int)(tooltip_rect.x1 - difference);
                    tooltip_rect.x0 = (float)(int)(tooltip_rect.x0 - difference);
                }
                
                F4_DrawTooltipRect(app, tooltip_rect);
                
                draw_string(app, tooltip_face_id, string,
                            V2f32(tooltip_rect.x0 + 4,
                                  tooltip_rect.y0 + 4),
                            global_tooltips[i].color);
            }
        }
    }
    
    // NOTE(rjf): Draw inactive rectangle
    if(is_active_view == 0)
    {
        Rect_f32 view_rect = view_get_screen_rect(app, view_id);
        ARGB_Color color = fcolor_resolve(fcolor_id(fleury_color_inactive_pane_overlay));
        if(F4_ARGBIsValid(color))
        {
            draw_rectangle(app, view_rect, 0.f, color);
        }
    }
    
    // NOTE(rjf): Render code peek.
#if 0
    {
        if(!view_get_is_passive(app, view_id) &&
           !is_active_view)
        {
            F4_CodePeek_Render(app, view_id, face_id, buffer, frame_info);
        }
    }
#endif 
}

//~ NOTE(rjf): Render hook

function void
momo_draw_file_bar(Application_Links *app, View_ID view_id, Buffer_ID buffer, Face_ID face_id, Rect_f32 bar)
{
    Scratch_Block scratch(app);
    
    draw_rectangle_fcolor(app, bar, 0.f, fcolor_id(defcolor_bar));
    
    FColor base_color = fcolor_id(defcolor_base);
    FColor pop2_color = fcolor_id(defcolor_pop2);
    
    i64 cursor_position = view_get_cursor_pos(app, view_id);
    Buffer_Cursor cursor = view_compute_cursor(app, view_id, seek_pos(cursor_position));

        
    if(!def_get_config_b32(vars_save_string_lit("f4_disable_progress_bar")))
    {
        f32 progress = (f32)cursor.line / (f32)buffer_get_line_count(app, buffer);
        Rect_f32 progress_bar_rect =
        {
            bar.x0 + (bar.x1 - bar.x0) * progress,
            bar.y0,
            bar.x1,
            bar.y1,
        };
        ARGB_Color progress_bar_color = fcolor_resolve(fcolor_id(fleury_color_file_progress_bar));
        if(F4_ARGBIsValid(progress_bar_color))
        {
            draw_rectangle(app, progress_bar_rect, 0, progress_bar_color);
        }
    }
    
    Fancy_Line list = {};
    String_Const_u8 unique_name = push_buffer_unique_name(app, scratch, buffer);
    push_fancy_string(scratch, &list, base_color, unique_name);
    push_fancy_stringf(scratch, &list, base_color, " - Row: %3.lld Col: %3.lld -", cursor.line, cursor.col);
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Line_Ending_Kind *eol_setting = scope_attachment(app, scope, buffer_eol_setting,
                                                     Line_Ending_Kind);
    switch (*eol_setting){
        case LineEndingKind_Binary:
        {
            push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" bin"));
        }break;
        
        case LineEndingKind_LF:
        {
            push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" lf"));
        }break;
        
        case LineEndingKind_CRLF:
        {
            push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" crlf"));
        }break;
    }
    
    u8 space[3];
    {
        Dirty_State dirty = buffer_get_dirty_state(app, buffer);
        String_u8 str = Su8(space, 0, 3);
        if (dirty != 0){
            string_append(&str, string_u8_litexpr(" "));
        }
        if (HasFlag(dirty, DirtyState_UnsavedChanges)){
            string_append(&str, string_u8_litexpr("*"));
        }
        if (HasFlag(dirty, DirtyState_UnloadedChanges)){
            string_append(&str, string_u8_litexpr("!"));
        }
        push_fancy_string(scratch, &list, pop2_color, str.string);
    }
    
    push_fancy_string(scratch, &list, base_color, S8Lit(" Syntax Mode: "));
    push_fancy_string(scratch, &list, base_color, F4_SyntaxOptionString());
    
    Vec2_f32 p = bar.p0 + V2f32(2.f, 2.f);
    draw_fancy_line(app, face_id, fcolor_zero(), &list, p);

}

function Rect_f32_Pair
momo_layout_line_number_margin(Rect_f32 rect, f32 digit_advance, i64 digit_count, f32 padding){
    f32 margin_width = (f32)digit_count*digit_advance + padding;
    return(rect_split_left_right(rect, margin_width));
}

function Rect_f32_Pair
momo_layout_line_number_margin(Application_Links *app, Buffer_ID buffer, Rect_f32 rect, f32 digit_advance, f32 padding){
    i64 line_count = buffer_get_line_count(app, buffer);
    i64 line_count_digit_count = digit_count_from_integer(line_count, 10);
    return(momo_layout_line_number_margin(rect, digit_advance, line_count_digit_count, padding));
}

function void
momo_draw_line_number_margin(Application_Links *app, View_ID view_id, Buffer_ID buffer, Face_ID face_id, Text_Layout_ID text_layout_id, Rect_f32 margin, f32 left_offset){
    ProfileScope(app, "draw line number margin");
    
    Scratch_Block scratch(app);
    FColor line_color = fcolor_id(defcolor_line_numbers_text);
    
    Rect_f32 prev_clip = draw_set_clip(app, margin);
    draw_rectangle_fcolor(app, margin, 0.f, fcolor_id(defcolor_line_numbers_back));
    
    Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
    i64 line_count = buffer_get_line_count(app, buffer);
    i64 line_count_digit_count = digit_count_from_integer(line_count, 10);
    
    Fancy_String fstring = {};
    u8 *digit_buffer = push_array(scratch, u8, line_count_digit_count);
    String_Const_u8 digit_string = SCu8(digit_buffer, line_count_digit_count);
    for (i32 i = 0; i < line_count_digit_count; i += 1){
        digit_buffer[i] = ' ';
    }
    
    Buffer_Cursor cursor = view_compute_cursor(app, view_id, seek_pos(visible_range.first));
    i64 line_number = cursor.line;
    
    Buffer_Cursor cursor_opl = view_compute_cursor(app, view_id, seek_pos(visible_range.one_past_last));
    i64 one_past_last_line_number = cursor_opl.line + 1;
    
    u8 *small_digit = digit_buffer + line_count_digit_count - 1;
    {
        u8 *ptr = small_digit;
        if (line_number == 0){
            *ptr = '0';
        }
        else{
            for (u64 X = line_number; X > 0; X /= 10){
                *ptr = '0' + (X%10);
                ptr -= 1;
            }
        }
    }
    
    for (;line_number < one_past_last_line_number &&
         line_number < line_count;){
        Range_f32 line_y = text_layout_line_on_screen(app, text_layout_id, line_number);
        Vec2_f32 p = V2f32(margin.x0 + left_offset, line_y.min);
        
        fill_fancy_string(&fstring, 0, line_color, 0, 0, digit_string);
        draw_fancy_string(app, face_id, fcolor_zero(), &fstring, p);
        
        line_number += 1;
        {
            u8 *ptr = small_digit;
            for (;;){
                if (ptr < digit_buffer){
                    break;
                }
                if (*ptr == ' '){
                    *ptr = '0';
                }
                if (*ptr == '9'){
                    *ptr = '0';
                    ptr -= 1;
                }
                else{
                    *ptr += 1;
                    break;
                }
            }
        }
    }
    
    draw_set_clip(app, prev_clip);
}

function void
momo_render(Application_Links *app, Frame_Info frame_info, View_ID view_id)
{
    F4_RecentFiles_RefreshView(app, view_id);
    
    ProfileScope(app, "[Fleury] Render");
    Scratch_Block scratch(app);
    
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    
    f32 margin_size = (f32)def_get_config_u64(app, vars_save_string_lit("f4_margin_size"));
    Rect_f32 view_rect = view_get_screen_rect(app, view_id);
    Rect_f32 region = rect_inner(view_rect, margin_size);
    
    Buffer_ID buffer = view_get_buffer(app, view_id, Access_Always);
    String_Const_u8 buffer_name = push_buffer_base_name(app, scratch, buffer);
    
    //~ NOTE(rjf): Draw background.
    {
        ARGB_Color color = fcolor_resolve(fcolor_id(defcolor_back));
        if(string_match(buffer_name, string_u8_litexpr("*compilation*")))
        {
            color = color_blend(color, 0.5f, 0xff000000);
        }
        // NOTE(rjf): Inactive background color.
        else if(is_active_view == 0)
        {
            ARGB_Color inactive_bg_color = fcolor_resolve(fcolor_id(fleury_color_inactive_pane_background));
            if(F4_ARGBIsValid(inactive_bg_color))
            {
                color = inactive_bg_color;
            }
        }
        draw_rectangle(app, region, 0.f, color);
        draw_margin(app, view_rect, region, color);
    }
    
    //~ NOTE(rjf): Draw margin.
    {
        ARGB_Color color = fcolor_resolve(fcolor_id(defcolor_margin));
        if(def_get_config_b32(vars_save_string_lit("f4_margin_use_mode_color")) &&
           is_active_view)
        {
            color = F4_GetColor(app, ColorCtx_Cursor(power_mode.enabled ? ColorFlag_PowerMode : 0,
                                                     GlobalKeybindingMode));
        }
        draw_margin(app, view_rect, region, color);
    }
    
    Rect_f32 prev_clip = draw_set_clip(app, region);
    
    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics face_metrics = get_face_metrics(app, face_id);
    f32 line_height = face_metrics.line_height;
    f32 digit_advance = face_metrics.decimal_digit_advance;
    
    // NOTE(allen): file bar
    b64 showing_file_bar = false;
    if(view_get_setting(app, view_id, ViewSetting_ShowFileBar, &showing_file_bar) && showing_file_bar)
    {
        Rect_f32_Pair pair = layout_file_bar_on_top(region, line_height);
        momo_draw_file_bar(app, view_id, buffer, face_id, pair.min);
        region = pair.max;
    }
    
    Buffer_Scroll scroll = view_get_buffer_scroll(app, view_id);
    Buffer_Point_Delta_Result delta = delta_apply(app, view_id, frame_info.animation_dt, scroll);
    
    if(!block_match_struct(&scroll.position, &delta.point))
    {
        block_copy_struct(&scroll.position, &delta.point);
        view_set_buffer_scroll(app, view_id, scroll, SetBufferScroll_NoCursorChange);
    }
    
    if(delta.still_animating)
    {
        animate_in_n_milliseconds(app, 0);
    }
    
    // NOTE(allen): query bars
    {
        Query_Bar *space[32];
        Query_Bar_Ptr_Array query_bars = {};
        query_bars.ptrs = space;
        if (get_active_query_bars(app, view_id, ArrayCount(space), &query_bars))
        {
            for (i32 i = 0; i < query_bars.count; i += 1)
            {
                Rect_f32_Pair pair = layout_query_bar_on_top(region, line_height, 1);
                draw_query_bar(app, query_bars.ptrs[i], face_id, pair.min);
                region = pair.max;
            }
        }
    }
    
    // NOTE(allen): FPS hud
    if(show_fps_hud)
    {
        Rect_f32_Pair pair = layout_fps_hud_on_bottom(region, line_height);
        draw_fps_hud(app, frame_info, face_id, pair.max);
        region = pair.min;
        animate_in_n_milliseconds(app, 1000);
    }
    
    // NOTE(allen): layout line numbers
    Rect_f32 line_number_rect = {};
    f32 margin_padding = 4.f;
    if(def_get_config_b32(vars_save_string_lit("show_line_number_margins")))
    {
        Rect_f32_Pair pair = momo_layout_line_number_margin(app, buffer, region, digit_advance, margin_padding);
        line_number_rect = pair.min;
        region = pair.max;
    }
    
    // NOTE(allen): begin buffer render
    Buffer_Point buffer_point = scroll.position;
    if(is_active_view)
    {
        buffer_point.pixel_shift.y += F4_PowerMode_ScreenShake()*1.f;
    }
    Text_Layout_ID text_layout_id = text_layout_create(app, buffer, region, buffer_point);
    
    // NOTE(allen): draw line numbers
    if(def_get_config_b32(vars_save_string_lit("show_line_number_margins")))
    {
        momo_draw_line_number_margin(app, view_id, buffer, face_id, text_layout_id, line_number_rect, margin_padding);
    }
    
    // NOTE(allen): draw the buffer
    momo_render_buffer(app, view_id, face_id, buffer, text_layout_id, region, frame_info);
    
    // NOTE(rjf): Render command server
#if OS_WINDOWS
    CS_render_caller(app, frame_info, view_id);
#endif
    
    text_layout_free(app, text_layout_id);
    draw_set_clip(app, prev_clip);
}