/***********************************************************************
 * Copyright (C) 2009  Andrei Taranchenko
 * Contact: http://www.nulidex.com/contact
   Licensed to the Apache Software Foundation (ASF) under one
   or more contributor license agreements.  See the NOTICE file
   distributed with this work for additional information
   regarding copyright ownership.  The ASF licenses this file
   to you under the Apache License, Version 2.0 (the
   "License"); you may not use this file except in compliance
   with the License.  You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing,
   software distributed under the License is distributed on an
   "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
   KIND, either express or implied.  See the License for the
   specific language governing permissions and limitations
   under the License.
 **********************************************************************/

/*
 * Copyright (C) 2014 James Higley

   Changed from std:string to std:wstring
*/

#ifndef html_template_c
#define html_template_c
#include "html_template.h"
#endif
#include <cstring>

using namespace std;

namespace tmpl {

//----------------------------------------------------------------------------
// table cell
//----------------------------------------------------------------------------
row_cell_s::row_cell_s() {
    // we are not creating a table object yet, just because we may not need it in
    // the first place.
    p_table = 0;
}
//----------------------------------------------------------------------------

row_cell_s::row_cell_s(const row_cell_s & arg) {
    str_val = arg.str_val;
    p_table = 0; // IMPORTANT, if not zero, the code will assume memory
    // allocated for table, while this cell actually may
    // contain a string value only

    if (arg.p_table) // this is why zero-init above is important, but it's
        // for the object being assigned
    {
        p_table = new loop_s;
        *p_table = *arg.p_table;
    }
}
//----------------------------------------------------------------------------

row_cell_s::~row_cell_s() {
    if (p_table) {
        delete p_table;
    }
}
//----------------------------------------------------------------------------

row_cell_s & row_cell_s::operator= (const loop_s & arg) {
    p_table = new loop_s;
    *p_table = arg;
    return *this;
}

//----------------------------------------------------------------------------
    // table row
//----------------------------------------------------------------------------

row_cell_s & row_s::operator() (const std::wstring & str_name) {
    wstring str_name_uc = str_name;
    // uppercase
    str_name_uc = str_name;
    uc(str_name_uc);

    // create new table cell
    row_cell_s cell;
    // insert cell into row container
    cells_c.insert(std::make_pair(str_name_uc, cell));
    // return cell so that we can assign a value to it
    return cells_c[str_name_uc];
}

//----------------------------------------------------------------------------
    // tag type object
//----------------------------------------------------------------------------
tag_type_s::tag_type_s() {
    str_tag_class = L"";
    str_tag_type  = L"";
    b_block_tag   = false;
}
//----------------------------------------------------------------------------

tag_type_s::tag_type_s(const std::wstring & arg_type,
                       const std::wstring & arg_class,
                       bool arg_block_tag /*= false*/) {
    str_tag_class = arg_class;
    str_tag_type  = arg_type;
    b_block_tag   = arg_block_tag;
}
//----------------------------------------------------------------------------

bool tag_type_s::operator== (const tag_type_s & rhs) const {
    // two tags are equal if they are of the same CLASS, so ELSE is equal to IF
    return (str_tag_class == rhs.str_tag_class);
}
//----------------------------------------------------------------------------

bool tag_type_s::operator!= (const tag_type_s & rhs) const {
    return !(*this == rhs);
}

//----------------------------------------------------------------------------
    // tag object
//----------------------------------------------------------------------------
tag_s::tag_s() {
    Reset();
}
//----------------------------------------------------------------------------

void tag_s::Reset() {
    str_name      = L"";
    begin         = wstring::npos;
    end           = wstring::npos;
    b_termination = false;
    b_splitter    = false;
    escape_mode   = ESC_NONE;
}
//----------------------------------------------------------------------------

tag_s::tag_s(const tag_type_s & arg_type) {
    tag_type = arg_type;
}
//----------------------------------------------------------------------------

void tag_s::Shift(const ptrdiff_t i_offset) {
    // do not shit an empty tag
    if (begin == wstring::npos) return;

    begin = static_cast<size_t>(begin + i_offset);
    end   = static_cast<size_t>(end   + i_offset);
}
//----------------------------------------------------------------------------

const bool tag_s::Empty() const {
    // tag is empty if none of its properties have been defined
    return (
               str_name.empty() == true
               && b_termination == false
               && b_splitter == false
          );
}
//----------------------------------------------------------------------------

const size_t tag_s::Length() const {
    return (end - begin) + 1;
}
//----------------------------------------------------------------------------

bool tag_s::operator< (const tag_s & rhs) const {
    // compare start positions of two tags to find which is further up
    return begin < rhs.begin;
}
//----------------------------------------------------------------------------

bool tag_s::operator> (const tag_s & rhs) const {
    return begin > rhs.begin;
}
//----------------------------------------------------------------------------

bool tag_s::operator== (const tag_s & rhs) const {
    return (rhs.begin == begin);
}
//----------------------------------------------------------------------------

un_predicate_count_tag_type::un_predicate_count_tag_type(const tag_type_s & arg) {
    tag_type = arg;
}
//----------------------------------------------------------------------------

bool un_predicate_count_tag_type::operator()(const tag_s & arg_tag) {
    return (arg_tag.Get_Tag_Type() == tag_type);
}

//----------------------------------------------------------------------------
// block object
//----------------------------------------------------------------------------
block_s::block_s(const tag_s & arg_open) {
    b_deleted  = false;
    Set_Open_Tag(arg_open);
}
//----------------------------------------------------------------------------

void block_s::Set_Open_Tag(const tag_s & arg) {
    tag_open = arg;
    block_type = arg.Get_Tag_Type();
}
//----------------------------------------------------------------------------

block_s::block_s() {
    b_deleted  = false;
}
//----------------------------------------------------------------------------

void block_s::Shift(const ptrdiff_t i_offset, const size_t ui_start,
                    const size_t ui_end) {
    // do nothing if the block is no longer in the set
    if (Deleted() == true) return;

    // store all tags in a container, for easy iteration
    vector <tag_s*> tags_c;
    tags_c.push_back(&tag_open);
    tags_c.push_back(&tag_split);
    tags_c.push_back(&tag_close);

    for (size_t n=0; n < tags_c.size(); n++) {
        /*not const*/ tag_s* itr_tag = tags_c[n];

        // do nothing if the tag is above the start position or below the end
        // posisiton
        if (itr_tag->Start() < ui_start || itr_tag->Stop() > ui_end) {
            continue;
        }

        itr_tag->Shift(i_offset);
    }
}
//----------------------------------------------------------------------------

bool block_s::Contains(const block_s & rhs) const {
    // if this is a block with no terminating tag, it can't contain anything
    if (this->Get_Type().b_block_tag == false) {
        return false;
    }

    return (rhs.Get_Open_Tag() > this->Get_Open_Tag() &&
            rhs.Get_Close_Tag() < this->Get_Close_Tag());
}
//----------------------------------------------------------------------------

bool block_s::operator== (const tag_s & rhs) const {
    return (rhs.Start() == tag_open.Start()
            || rhs.Start() == tag_close.Start()
            || rhs.Start() == tag_split.Start());
}
//----------------------------------------------------------------------------

bool block_s::operator!= (const tag_s & rhs) const {
    return !(*this == rhs);
}
//----------------------------------------------------------------------------

bool block_s::operator== (const block_s & rhs) const {
    return (rhs.tag_open == tag_open);
}
//----------------------------------------------------------------------------

bool block_s::operator!= (const block_s & rhs) const {
    return !(*this == rhs);
}
//----------------------------------------------------------------------------

bool block_s::operator> (const block_s & rhs) const {
    // block is greater than another block if the start of the block is after the
    // close of another block
    return (tag_open > rhs.tag_close);
}
//----------------------------------------------------------------------------

un_predicate_count_tag::un_predicate_count_tag(const tag_s & arg) {
    tag = arg;
}
//----------------------------------------------------------------------------

bool un_predicate_count_tag::operator()(const block_s & arg_block) {
    return (arg_block == tag);
}

//----------------------------------------------------------------------------
// template class public
//----------------------------------------------------------------------------
html_template::html_template() {
    tag_type_prefix = L"TMPL_";
    init();
}
//----------------------------------------------------------------------------

html_template::html_template(const std::wstring & arg_file_name) {
    tag_type_prefix = L"TMPL_";
    init();
    Set_Template_File(arg_file_name);
}
//----------------------------------------------------------------------------

void html_template::Set_Template_File(const std::wstring & arg_file_name) {
    str_tmpl_file_name = arg_file_name;

    if (arg_file_name.empty()) {
        throw("Template file name not specified");
    }

#ifdef DEBUG
    wcout << L"\n!!!!!!!!!!!!!!!\ntemplate " << str_tmpl_file_name << L" loaded" << endl;
#endif
}
//----------------------------------------------------------------------------

const std::wstring & html_template::Process() {
    std::string file_name;
    size_t len = str_tmpl_file_name.length();
    if (len > 0)
    {
        char* w = new char[len + 1];
        size_t s = wcstombs(w, str_tmpl_file_name.c_str(), len);
        if (s != len)
            w[0] = 0;
        else
            w[s] = 0;
        file_name = std::string(w);
        delete[] w;
    }

    std::wifstream in_stream;
    in_stream.open(file_name, ios::binary);

    if(!in_stream.is_open()) {
        runtime_ex ex(L"Could not open template file");
        throw_exception(ex);
    }

    // read in the file
    std::wstringstream oss;
    oss << in_stream.rdbuf();
    str_tmpl_txt = oss.str();
    in_stream.close();

    // this will expand the include files, and build the map of all the variables -
    // once if there are no includes or (1 + # of includes)
    expand_includes();

    str_tmpl_txt_cpy = str_tmpl_txt;

#ifdef DEBUG
    print_block_map(block_map);
#endif

    // tag iterator
    tag_map_t::const_iterator itr_tag;

    // check that every tag is part of a block
    for (itr_tag = tag_map.begin(); itr_tag != tag_map.end(); ++itr_tag) {
        block_map_t::const_iterator itr_block
            = find(block_map.begin(), block_map.end(), *itr_tag);

        if (itr_block == block_map.end()) {
            syntax_ex error(L"Unmatched tag");
            error.line = get_line_from_pos(itr_tag->Start());
            error.detail = L"Unmatched '"
                           + itr_tag->Get_Tag_Type().str_tag_type
                           + L"' tag";
            throw_exception(error);
        }
    }

#ifdef DEBUG

    //
    // assert that every tag appears only once in each block
    //
    for (itr_tag = tag_map.begin(); itr_tag != tag_map.end(); ++itr_tag) {
        un_predicate_count_tag predicate(*itr_tag);

        const size_t ui_occurances
            = std::count_if(block_map.begin(), block_map.end(), predicate);

        assert(ui_occurances == 1 && L"Each tag must appear in a block only ONCE");
    }

#endif

    //**************************************************************************
    //**************************************************************************
    //
    // start variable substitution
    //

    // process IF blocks first to take out unwanted data right away
    process_conditionals(block_map, str_tmpl_txt_cpy, variables_c);

    // process loops
    process_loops(block_map, str_tmpl_txt_cpy, variables_c);

    //
    // process simple variables
    //
    process_simple_vars(block_map, str_tmpl_txt_cpy, variables_c);

    //
    // end variable substitution
    //
    //**************************************************************************
    //**************************************************************************

    return str_tmpl_txt_cpy;
}
//----------------------------------------------------------------------------

void html_template::print_block_map(block_map_t & r_block_map) {
    // block iterator
    block_map_t::const_iterator itr_block = r_block_map.begin();

    wcout << L"\n*************************************************"
          << L"\nBLOCK MAP"
          << L"\n*************************************************\n";

    for (; itr_block != r_block_map.end(); ++itr_block) {
        wcout << itr_block->Get_Type().str_tag_class
              << L"\t" << itr_block->Get_Name();

        if (itr_block->Deleted())
            wcout << L" [DELETED]";

        wcout << L"\nOpen\tL " << get_line_from_pos(itr_block->Get_Open_Tag().Start())
              << L" (" << itr_block->Get_Open_Tag().Start()
              << L", " << itr_block->Get_Open_Tag().Stop() << L")";

        if (itr_block->Get_Split_Tag().Start() != wstring::npos)
            wcout << L"\nSplit\tL "
                  << get_line_from_pos(itr_block->Get_Split_Tag().Start())
                  << L" (" << itr_block->Get_Split_Tag().Start()
                  << L", " << itr_block->Get_Split_Tag().Stop() << L")";

        if (itr_block->Get_Close_Tag().Start() != wstring::npos)
            wcout << L"\nClose\tL "
                  << get_line_from_pos(itr_block->Get_Close_Tag().Start())
                  << L" (" << itr_block->Get_Close_Tag().Start()
                  << L", " << itr_block->Get_Close_Tag().Stop() << L")";

        wcout << L"\n-------------\n";
    }
}
//----------------------------------------------------------------------------

void html_template::print_tag_map() {
//!
//! NOTE - the tag map is used to create the block map. After the block map
//! is created, the tag map is no longer used for anything -  you can't debug
//! tag shifting with it. Use the block map for that.
//!

    // tag iterator
    tag_map_t::const_iterator itr_tag;

    wcout << L"\n*************************************************"
          << L"\nTAG MAP"
          << L"\n*************************************************\n";

    for (itr_tag = tag_map.begin(); itr_tag != tag_map.end(); ++itr_tag) {
        wcout << L"L " << get_line_from_pos(itr_tag->Start()) << L"\t";

        if (itr_tag->Is_Termination())
            wcout << L"</";
        else
            wcout << L"<";

        wcout << itr_tag->Get_Tag_Type().str_tag_type << L">";

        if (itr_tag->Is_Named())
            wcout << L"\t= ";

        wcout << itr_tag->Get_Name()
             << L" (" << itr_tag->Start() << L", " << itr_tag->Stop() << L")"
             << endl;
    }
}
//----------------------------------------------------------------------------

html_template::~html_template() {
#ifdef DEBUG
    wcout << L"template " << str_tmpl_file_name << L" destroyed" << endl;
#endif
}
//----------------------------------------------------------------------------

cls_variable & html_template::operator() (const std::wstring & arg_var_name) {
    wstring arg_var_name_uc = arg_var_name;
    // uppercase
    uc(arg_var_name_uc);

    // create variable
    cls_variable var(arg_var_name_uc);

    // save it
    variables_c[arg_var_name_uc] = var;

#ifdef DEBUG3
    wcout << L"creating variable '" << arg_var_name_uc << L"'" << endl;
#endif

    // return the variable so a value can be assigned to it
    return variables_c[arg_var_name_uc];
}
//----------------------------------------------------------------------------

const size_t
html_template::Get_Tag_Type_Count(const tag_type_s & tag_type) const {
    un_predicate_count_tag_type predicate(tag_type);

    const size_t ui_count = std::count_if(
                                tag_map.begin(), tag_map.end(), predicate
                           );

    return ui_count;
}
//----------------------------------------------------------------------------
// template class private
//----------------------------------------------------------------------------
void html_template::init() {
    //
    // declare all known tag types
    //
    // the last boolean flag defines if the tag is a block (has open and close
    // tags)
    tag_types_c[L"SIMPLE"]  = tag_type_s(L"VAR",     L"VAR");
    tag_types_c[L"LOOP"]    = tag_type_s(L"LOOP",    L"LOOP", true);
    tag_types_c[L"IF"]      = tag_type_s(L"IF",      L"IF",   true);
    tag_types_c[L"ELSE"]    = tag_type_s(L"ELSE",    L"IF");
    tag_types_c[L"UNLESS"]  = tag_type_s(L"UNLESS",  L"IF",   true);
    tag_types_c[L"INCLUDE"] = tag_type_s(L"INCLUDE", L"INCLUDE");

    // create the tag strings we expect to see in a document, based on the tag
    // types
    tag_types_t::const_iterator itr_tag_type = tag_types_c.begin();

    for (; itr_tag_type != tag_types_c.end(); ++itr_tag_type) {
        tag_type_s tag_type = itr_tag_type->second;
        wstring str_tag_type_ref = tag_type_prefix + tag_type.str_tag_type;
        // save
        tag_strings_c[ str_tag_type_ref ] = 1;
    }

    // populate reserved words container (helps us during parsing)
    reserved_words_c[ L"ESCAPE" ]     = 1;
    reserved_words_c[ L"ESC" ]        = 1;
    reserved_words_c[ L"HTML" ]       = 1;
    reserved_words_c[ L"JS" ]         = 1;
    reserved_words_c[ L"JAVASCRIPT" ] = 1;
    reserved_words_c[ L"URL" ]        = 1;
    reserved_words_c[ L"XML" ]        = 1;
}

//----------------------------------------------------------------------------

void html_template::expand_includes() {
    build_block_map();

    // block iterator
    block_map_t::const_iterator itr_block;

    itr_block = block_map.begin();

    unsigned short ush_processed_includes = 0;

    for (; itr_block != block_map.end(); ++itr_block) {
        if (itr_block->Get_Type() != tag_types_c[L"INCLUDE"]) continue;

        if (itr_block->Deleted())                            continue;

        // increment count
        ++ush_processed_includes;

        // read in the file contents
        wstring str_file_name = itr_block->Get_Name();


        // the file may have either absolute or relative path. If relative, combine
        // it with the path of the parent file
        if (str_file_name.find_first_of(L"\\/", 0, 1) == wstring::npos) {
            // this file is relative to main template location

            // get the main template dir
            const wstring str_parent_tmpl_dir = file_directory(str_tmpl_file_name);

            str_file_name = str_parent_tmpl_dir + str_file_name;
        }

        std::string file_name;
        size_t len = str_file_name.length();
        if (len > 0)
        {
            char* w = new char[len + 1];
            size_t s = wcstombs(w, str_file_name.c_str(), len);
            if (s != len)
                w[0] = 0;
            else
                w[s] = 0;
            file_name = std::string(w);
            delete[] w;
        }

        std::wifstream in_stream;
        in_stream.open(file_name);

        if(!in_stream.is_open()) {
            runtime_ex ex(L"Could not open file for reading");
            throw_exception(ex);
        }

        // read in the file
        std::wstringstream oss;
        oss << in_stream.rdbuf();
        const wstring str_replace_with = oss.str();

        in_stream.close();

        const size_t block_len = itr_block->Get_Open_Tag().Stop()
                                 - itr_block->Get_Open_Tag().Start()
                                 + 1;

        const ptrdiff_t i_offset = get_offset(block_len, str_replace_with);

        // shift tags in rest of document

        shift_tags(
            str_tmpl_txt, block_map, i_offset,
            itr_block->Get_Open_Tag().Stop()
       );

        str_tmpl_txt.replace(
            itr_block->Get_Open_Tag().Start(),
            block_len,
            str_replace_with
       );
    }

    // recurse if any tags were processed
    if (ush_processed_includes) {
        expand_includes(); // RECURSION
    }

    // no need to build the map again - if there was at least one include, the
    // map would be rebuilt by the recursive call
}
//----------------------------------------------------------------------------

void html_template::process_conditionals(block_map_t & r_block_map,
        std::wstring & str_text,
        variables_t & r_variables_c) {
    // block iterator
    block_map_t::iterator itr_block = r_block_map.begin();

    for (; itr_block != r_block_map.end(); ++itr_block) {
        if (itr_block->Get_Type() != tag_types_c[L"IF"]) continue;

        if (itr_block->Deleted())                       continue;

        // ignore this conditional if it's within a loop scope (defined inside the
        // given block map
        block_map_t::const_iterator itr_block_2 = r_block_map.begin();
        bool b_this_scope = true;

        for (; itr_block_2 != r_block_map.end(); ++itr_block_2) {
            if (itr_block_2->Get_Type() == tag_types_c[L"LOOP"]) {
                if (itr_block_2->Contains(*itr_block)) {
                    b_this_scope = false;
                    break;;
                }
            }
        }

        if (!b_this_scope) continue;

#ifdef DEBUG2
        wcout << L"processing conditional " << itr_block->Get_Name() << endl;
#endif

        size_t replace_len        = 0;
        size_t replace_with_begin = 0;
        size_t replace_with_end   = 0;
        size_t block_len          = 0;
        wstring str_replace_with   = L"";
        bool b_eval_as_true       = false;

        // part of an IF block may be deleted. In that case, there are two tags
        // between each other tags will need to be deleted as well
        tag_s delete_btwn_tag_begin;
        tag_s delete_btwn_tag_end;

        // space between these tags will be the offset to shift other tags by
        tag_s shift_offset_tag_begin;
        tag_s shift_offset_tag_end;

        // these define the block where tag shifting will occur
        tag_s shift_tag_from;
        tag_s shift_tag_until;

        // if variable for this IF found, evaluate
        variables_t::iterator itr_var;
        itr_var = r_variables_c.find(itr_block->Get_Name());

        if (itr_var != r_variables_c.end()) {
            cls_variable & r_var = itr_var->second;

            // evaluate the IF
            b_eval_as_true = evaluate(r_var);

            // if this is an UNLESS, flip the evaluation result
            if (itr_block->Get_Type().str_tag_type == L"UNLESS") {
                b_eval_as_true = (b_eval_as_true == false) ? true : false;
            }
        } else {
            // if this is an unless, the absent variable should eval to TRUE
            if (itr_block->Get_Type().str_tag_type == L"UNLESS") {
                b_eval_as_true = true;
            }
        }

        // determine what we are replacing the IF with

        // if this is a two part block
        if (itr_block->Has_Split_Tag()) {
            // replace with first part of statement if true
            if (b_eval_as_true) {
                // replace the whole if with what's between the IF and the ELSE
                replace_with_begin = itr_block->Get_Open_Tag().Stop();
                replace_with_end   = itr_block->Get_Split_Tag().Start();

                // delete between the ELSE and the /IF
                delete_btwn_tag_begin = itr_block->Get_Split_Tag();
                delete_btwn_tag_end   = itr_block->Get_Close_Tag();

                shift_offset_tag_begin = itr_block->Get_Open_Tag();
                shift_offset_tag_end   = itr_block->Get_Open_Tag();
                shift_tag_from         = itr_block->Get_Open_Tag();
                shift_tag_until        = itr_block->Get_Split_Tag();
            } else { // replace with second part if false
                // replace the whole IF with what's between the ELSE and the /IF
                replace_with_begin = itr_block->Get_Split_Tag().Stop();
                replace_with_end   = itr_block->Get_Close_Tag().Start();

                // delete between the IF and the ELSE
                delete_btwn_tag_begin = itr_block->Get_Open_Tag();
                delete_btwn_tag_end   = itr_block->Get_Split_Tag();

                shift_offset_tag_begin = itr_block->Get_Open_Tag();
                shift_offset_tag_end   = itr_block->Get_Split_Tag();
                shift_tag_from         = itr_block->Get_Split_Tag();
                shift_tag_until        = itr_block->Get_Close_Tag();
            }

        } else { //one part IF block - it either stays or goes
            // replacing with whatever's in the whole IF, when the statements
            // evaluates to TRUE
            replace_with_begin = itr_block->Get_Open_Tag().Stop();
            replace_with_end   = itr_block->Get_Close_Tag().Start();

            shift_offset_tag_begin = itr_block->Get_Open_Tag();
            shift_offset_tag_end   = itr_block->Get_Open_Tag();
            shift_tag_from         = itr_block->Get_Open_Tag();
            shift_tag_until        = itr_block->Get_Close_Tag();

            // if this evaluates to FALSE, define range in which all other tags
            // will be nuked, and no longer expanded by future operations
            if (!b_eval_as_true) {
                delete_btwn_tag_begin = itr_block->Get_Open_Tag();
                delete_btwn_tag_end   = itr_block->Get_Close_Tag();
            }
        }

        // length of test that will replace the IF
        replace_len = replace_with_end - replace_with_begin - 1;

        // get the actual text to replace the IF with
        str_replace_with = str_text.substr(replace_with_begin + 1,
                                           replace_len);

        // length of the IF block
        block_len = itr_block->Get_Close_Tag().Stop()
                    - itr_block->Get_Open_Tag().Start()
                    + 1;

        // delete all blocks in IF part that will go away, if that part is defined
        if (delete_btwn_tag_begin.Empty() == false) {
            delete_blocks(delete_btwn_tag_begin, delete_btwn_tag_end, r_block_map);
        }

        // shift tags inside the IF
        shift_tags(
            // subject text
            str_text,
            // the block map
            r_block_map,
            // shift size
            (shift_offset_tag_end.Stop() - shift_offset_tag_begin.Start() + 1) * -1,
            // shift range start
            shift_tag_from.Stop(),
            // shift range end
            shift_tag_until.Start()
       );

        if (itr_block->Has_Split_Tag()) {
            // shift tags in rest of document
            const ptrdiff_t i_offset = get_offset(block_len, str_replace_with);

            shift_tags(
                str_text,
                r_block_map,
                i_offset,
                shift_tag_until.Stop()
           );

            str_text.replace(
                itr_block->Get_Open_Tag().Start(),
                block_len,
                str_replace_with
           );
        } else { // process the whole IF
            if (!b_eval_as_true) {
                str_replace_with.clear();
            }

            const ptrdiff_t i_offset = get_offset(block_len, str_replace_with);

            // shift tags in rest of document
            shift_tags(
                str_text,
                r_block_map,
                i_offset,
                itr_block->Get_Close_Tag().Start()
           );

            str_text.replace(
                itr_block->Get_Open_Tag().Start(),
                block_len,
                str_replace_with
           );
        }

        itr_block->Delete();
    }

}
//----------------------------------------------------------------------------

void html_template::process_simple_vars(block_map_t & r_block_map,
                                        std::wstring & str_text,
                                        html_template::variables_t & r_variables_c) {
    block_map_t::iterator itr_block = r_block_map.begin();

    for (; itr_block != r_block_map.end(); ++itr_block) {
        if (itr_block->Get_Type() != tag_types_c[L"SIMPLE"]) continue;

        if (itr_block->Deleted())                           continue;

#ifdef DEBUG2
        wcout << L"processing variable " << itr_block->Get_Name() << endl;
#endif

        wstring str_replace_with = L"";

        // find the variable of this name
        variables_t::iterator pos_var;
        pos_var = r_variables_c.find(itr_block->Get_Name());
        cls_variable* p_var = 0;

        pos_var = r_variables_c.find(itr_block->Get_Name());

        if (pos_var != r_variables_c.end()) {
            p_var = &(pos_var->second);
            str_replace_with = p_var->Get_Val_String();
            escape_var(str_replace_with, itr_block->Get_Escape_Mode());
        }

        const size_t block_len = itr_block->Get_Open_Tag().Stop()
                                 - itr_block->Get_Open_Tag().Start() + 1;

        const ptrdiff_t i_offset = get_offset(block_len, str_replace_with);

        shift_tags(
            str_text,
            r_block_map,
            i_offset,
            itr_block->Get_Open_Tag().Stop()
       );

        str_text.replace(
            itr_block->Get_Open_Tag().Start(),
            block_len,
            str_replace_with
       );

        itr_block->Delete();
    }
}
//----------------------------------------------------------------------------

void html_template::process_loops(block_map_t & r_block_map,
                                  std::wstring & str_text,
                                  variables_t & r_variables_c) {
    block_map_t::iterator itr_block = r_block_map.begin();

    for (; itr_block != r_block_map.end(); ++itr_block) {
        if (itr_block->Get_Type() != tag_types_c[L"LOOP"]) continue;

        if (itr_block->Deleted())                         continue;

        process_loop(*itr_block, str_text, r_block_map, r_variables_c);

        itr_block->Delete();
    }
}
//----------------------------------------------------------------------------

void html_template::process_loop(const block_s & block,
                                 std::wstring & str_text,
                                 block_map_t & r_block_map,
                                 variables_t & r_variables_c) {
#ifdef DEBUG
    wcout << L"processing loop: " << block.Get_Name() << endl;
#endif

    // Get the table variable for this block.
    variables_t::iterator itr_var = r_variables_c.find(block.Get_Name());
    loop_s local_table;

    itr_var = r_variables_c.find(block.Get_Name());

    if (itr_var != r_variables_c.end()) {
        cls_variable & r_var = itr_var->second;
        local_table = r_var.Get_Val_Table();
    }

    const size_t block_start   = block.Get_Open_Tag().Start();
    const size_t block_end     = block.Get_Close_Tag().Stop();
    const size_t block_len     = block_end - block_start + 1;
    const size_t content_start = block.Get_Open_Tag().Stop() + 1;
    const size_t content_end   = block.Get_Close_Tag().Start() - 1;
    const size_t content_len   = content_end - content_start + 1;

    // get loop text content
    wstring str_row_content = str_text.substr(content_start, content_len);

    //
    // go through all variables and expand the ones within this loop.
    //

    // create a map of variables that we will re-use for each row
    block_map_t loop_block_map;

    block_map_t::iterator itr_block =
        find(r_block_map.begin(), r_block_map.end(), block);

    // move to next block inside this one
    advance(itr_block, 1);

    for (; itr_block != r_block_map.end(); ++itr_block) {
        // skip processed
        if (itr_block->Deleted()) continue;

        // get out if we are past the end of this loop
        if (*itr_block > block) break;

        block_s loop_block = *itr_block;

        // change offset relative to beginning of content
        const ptrdiff_t & i_offset = content_start * -1;
        loop_block.Shift(i_offset, block_start, block_end);

        loop_block_map.push_back(loop_block);

#ifdef DEBUG2
        wcout << L"adding " << loop_block.Get_Name() << L" to loop map" << endl;
        wcout << L"block start: " << loop_block.Get_Open_Tag().Start() << endl;
        wcout << L"block end: " << loop_block.Get_Open_Tag().Stop() << endl;
#endif
    }

    // get row iterator
    loop_s::rows_t rows = local_table.Get_Rows();
    loop_s::rows_t::const_iterator itr_row = rows.begin();

    // convenience variables for conext variable math
    const unsigned int ui_total_rows = local_table.Get_Rows().size();
    unsigned int ui_current_row      = 0;

    // our final table content
    wstring str_replace_with;

    // for each table row
    for (; itr_row != rows.end(); ++itr_row) {
        ++ui_current_row;

        // get cell iterator
        const row_s::cells_t & row_cells = itr_row->cells_c;
        row_s::cells_t::const_iterator itr_cell = row_cells.begin();

        // local variables for this scope - copy from scope one level up
        variables_t lcl_variables_c = r_variables_c;

        // add variables specific to this scope, overriding any globals
        for (; itr_cell != row_cells.end(); ++itr_cell) {
            const wstring str_name = itr_cell->first;
            const wstring str_val  = itr_cell->second.str_val;
            loop_s *p_nested_loop = 0;

            if (itr_cell->second.p_table) {
                p_nested_loop = itr_cell->second.p_table;
            }

            // delete this key of a global variable of the same name trespasses
            // private scope
            if (lcl_variables_c.find(str_name) != lcl_variables_c.end()) {
                lcl_variables_c.erase(str_name);
            }

            if (str_val.empty() && p_nested_loop != 0) {
                cls_variable var(str_name);
                var = *p_nested_loop;
                lcl_variables_c[str_name] = var;
            } else {
                lcl_variables_c[str_name] = str_val;
            }
        }

        //
        // define context vars depending on where we are in the loop
        //
        // note that we must delete the variables first to make sure they do
        // not "trickle down" from the outer loop

        // __first__
        lcl_variables_c.erase(L"__FIRST__");
        lcl_variables_c[L"__FIRST__"] = ui_current_row == 1 ? 1 : 0;

        // __last__
        lcl_variables_c.erase(L"__LAST__");
        lcl_variables_c[L"__LAST__"] = ui_current_row == ui_total_rows ? 1 : 0;

        // __count__
        lcl_variables_c.erase(L"__COUNT__");
        lcl_variables_c[L"__COUNT__"] = ui_current_row;
        lcl_variables_c.erase(L"__COUNTER__");
        lcl_variables_c[L"__COUNTER__"] = ui_current_row;

        // __total__
        lcl_variables_c.erase(L"__TOTAL__");
        lcl_variables_c[L"__TOTAL__"] = ui_total_rows;

        // __odd__
        lcl_variables_c.erase(L"__ODD__");
        lcl_variables_c[L"__ODD__"] = (ui_current_row % 2 != 0) == true ? 1 : 0;

        // __even__
        lcl_variables_c.erase(L"__EVEN__");
        lcl_variables_c[L"__EVEN__"] = (ui_current_row % 2 == 0) == true ? 1 : 0;

        // __inner__
        lcl_variables_c.erase(L"__INNER__");
        lcl_variables_c[L"__INNER__"] =
            (ui_current_row > 1 && ui_current_row < ui_total_rows) == true ? 1 : 0;

        // create a copy of row content that we can modify
        wstring str_row_content_cpy(str_row_content);

        // create a copy of block map - it will be modified as well, to keep state
        // of row processing
        block_map_t loop_block_map_cpy(loop_block_map);

        // process IF's, UNLESS's, etc
#ifdef DEBUG2
        wcout << L"processing conditionals for loop: " << block.Get_Name() << endl;
#endif
        process_conditionals(loop_block_map_cpy, str_row_content_cpy, lcl_variables_c);

        // process loops
#ifdef DEBUG2
        wcout << L"processing nested loops for loop: " << block.Get_Name() << endl;
#endif
        process_loops(loop_block_map_cpy, str_row_content_cpy, lcl_variables_c);

        // process simple variables
#ifdef DEBUG2
        wcout << L"processing vars for loop: " << block.Get_Name() << endl;
#endif
        process_simple_vars(loop_block_map_cpy, str_row_content_cpy, lcl_variables_c);

        // add processed content to loop output string
        str_replace_with.append(str_row_content_cpy);
        str_replace_with.append(L"\n");
    }

    // erase the loop and nested blocks from the map - they are not to be
    // processed by anything else
    delete_blocks(block.Get_Open_Tag(), block.Get_Close_Tag(), r_block_map);

    //
    // replace the whole block with loop output
    //

    // shift tags within this block
    shift_tags(
        str_text,
        r_block_map,
        block.Get_Open_Tag().Length() * -1,
        block.Get_Open_Tag().Stop(),
        block.Get_Close_Tag().Start()
   );

    // shift tags after this block
    const ptrdiff_t i_offset = get_offset(block_len, str_replace_with);

    shift_tags(
        str_text,
        r_block_map,
        i_offset,
        block.Get_Close_Tag().Stop()
   );

    str_text.replace(block_start, block_len, str_replace_with);
}
//----------------------------------------------------------------------------

bool html_template::evaluate(cls_variable & var) const {
    wstring val_string = var.Get_Val_String();
    const loop_s & val_loop = var.Get_Val_Table();

    // return FALSE right away if both table and string are really empty
    if (val_loop.Empty() && val_string.empty()) {
        return false;
    }

    const wstring characters = L" \t\n\r";
    size_t pos = val_string.find_first_not_of(characters);

    if (wstring::npos != pos)
        val_string = val_string.substr(pos);

    pos = val_string.find_last_not_of(characters);

    if (wstring::npos != pos) {
        val_string = val_string.substr(0, pos+1);
    } else {
        // it is still possible that 'str' contains only 'characters':
        if (wstring::npos != val_string.find_first_of(characters))
            val_string.erase();
    }

    // delete a single dot, if any (assuming this is a double)
    size_t dot_pos = val_string.find(L".");

    if (dot_pos != wstring::npos) {
        val_string.erase(dot_pos, 1);
    }

    // if there is another dot - this is NOT a number, and is TRUE
    if (val_string.find(L".") != wstring::npos) return true;

    // delete all zeros
    size_t zero_pos = val_string.find(L"0");

    while (zero_pos != wstring::npos) {
        val_string.erase(zero_pos, 1);
        zero_pos = val_string.find(L"0");
    }

    return !(val_string.empty() && val_loop.Empty());
}
//----------------------------------------------------------------------------

const ptrdiff_t html_template::get_offset(const size_t block_len,
        const std::wstring & str_replace) const {
    ptrdiff_t i_offset = 0;

    if (block_len < str_replace.length()) {
        i_offset = str_replace.length() - block_len;
    }

    if (block_len > str_replace.length()) {
        i_offset = - static_cast<ptrdiff_t>((block_len) - str_replace.length());
    }

    return i_offset;
}
//----------------------------------------------------------------------------

void html_template::delete_blocks(const tag_s & open_tag,
                                  const tag_s & close_tag,
                                  block_map_t & r_block_map) {
    block_map_t::iterator itr_block = r_block_map.begin();

    for (; itr_block != r_block_map.end(); ++itr_block) {
        if (itr_block->Get_Open_Tag().Start() > open_tag.Stop()
                && itr_block->Get_Open_Tag().Start() < close_tag.Start()) {
            itr_block->Delete();
        }
    }
}
//----------------------------------------------------------------------------

void html_template::build_line_map() {
#ifdef DEBUG3
    wcout << L"\n-> Building line map" << endl;
#endif

    line_map.clear();

    size_t pos_line_end;
    size_t pos_line_begin = 0;
    size_t ui_line        = 1;

    // go through the whole document and save line numbers, with their
    // correspnding start and end positions
    do {
        pos_line_end = str_tmpl_txt.find_first_of(L"\r\n", pos_line_begin);

        if (pos_line_end != wstring::npos) {
            line_map[ui_line] = std::make_pair(pos_line_begin, pos_line_end - 1);
#ifdef DEBUG3
            wcout << L"Line " << ui_line << L": " << pos_line_begin << L" - "
                 << pos_line_end << endl;
#endif
        }

        ++ui_line;
        pos_line_begin = pos_line_end + 1;

    } while (pos_line_end != wstring::npos);
}
//----------------------------------------------------------------------------

void html_template::build_tag_map() {
#ifdef DEBUG3
    wcout << L"\n-> Building tag map" << endl;
#endif

    build_line_map();
    tag_map.clear();

    tag_s tag;

    do {
        tag = find_tag();

        if (tag.Empty() == false)
            tag_map.insert(tag);

    } while (!tag.Empty());
}
//------------------------------------------------------------------------------

void html_template::build_block_map() {
#ifdef DEBUG3
    wcout << L"\n--> Building block map" << endl;
#endif

    build_tag_map();
    block_map.clear();

    // glue tags together into blocks
    tag_map_t::const_iterator itr_tag = tag_map.begin();

    // Iterate through tags and create tag blocks
    for (; itr_tag != tag_map.end(); ++itr_tag) {
        // skip unnamed tags
        if (itr_tag->Is_Named() == false) continue;

        // add simple tags and go to next right away
        if (itr_tag->Get_Tag_Type() == tag_types_c[L"SIMPLE"]) {
            block_s block(*itr_tag);
            block_map.push_back(block);
            continue;
        }

        // add include tags and bail out
        if (itr_tag->Get_Tag_Type() == tag_types_c[L"INCLUDE"]) {
            block_s block(*itr_tag);
            block_map.push_back(block);
            continue;
        }

        block_s block;

        // if this is an opening for a block tag
        if (itr_tag->Get_Tag_Type().b_block_tag && itr_tag->Is_Named()) {
            // create and add the new block
            block.Set_Open_Tag(*itr_tag);
#ifdef DEBUG3
            wcout << L"Adding opening tag to " << itr_tag->Get_Name() << endl;
#endif
        }

        // but wait, that's not all - search now and you can find the closing tag
        // absolutely free.

        // block termination iterator
        tag_map_t::const_iterator itr_term_tag = itr_tag;

        // move pointer forward once since we do not want to be checking the SAME
        // tag for its own terminator
        std::advance(itr_term_tag, 1);

        // counter to keep track of inner blocks we want to skip, in order to get
        // to the terminating tag
        unsigned int ui_skip_num = 0;

        for (; itr_term_tag != tag_map.end(); ++itr_term_tag) {
            // skip anyting simple
            if (itr_term_tag->Get_Tag_Type() == tag_types_c[L"SIMPLE"]) {
                continue;
            }

            // another nested block begins, we can see
            if (itr_term_tag->Is_Named()) {
                ++ui_skip_num;
                continue;
            }

            // A nested block ends. We are not interested. Next!
            if (itr_term_tag->Is_Termination() && ui_skip_num) {
                --ui_skip_num;
                continue;
            }

            // A spliiter for a nested block. Nothing to see here.
            if (itr_term_tag->Is_Splitter() && ui_skip_num) {
                continue;
            }

            // we found a splitter for this block
            if (itr_term_tag->Is_Splitter() && ui_skip_num == 0) {
#ifdef DEBUG3
                wcout << L"Adding splitter tag to " << block.Get_Name() << endl;
#endif
                block.Set_Split_Tag(*itr_term_tag);
                continue;
            }

            // and that's the way the coookie crumbles - we found the terminating
            // tag
            if (itr_term_tag->Is_Termination() && ui_skip_num == 0) {
                // check that this tag is of correct type
                if (itr_term_tag->Get_Tag_Type() != itr_tag->Get_Tag_Type()) {
                    syntax_ex error(L"Block not properly terminated");
                    error.line = get_line_from_pos(itr_tag->Start());
                    error.detail = L"'" + itr_tag->Get_Name() + L"'"
                                   + L" not terminated properly";
                    throw_exception(error);
                }

#ifdef DEBUG3
                wcout << L"Adding termination tag to " << block.Get_Name() << endl;
#endif
                block.Set_Close_Tag(*itr_term_tag);
                block_map.push_back(block);
                break;
            }
        }

        // BUT if we don't find the termination tag, it's missing.
        // MISSING!
        if (block.Get_Close_Tag().Empty()) {
            syntax_ex error(L"Block not terminated");
            error.line = get_line_from_pos(block.Get_Open_Tag().Start());
            error.detail = L"'" + block.Get_Name() + L"'"
                           + L" not terminated";
            throw_exception(error);
        }
    }
}
//----------------------------------------------------------------------------

tag_s html_template::find_tag() const {
    tag_s tag;
    size_t start_pos = 0;

    // find the last tag added
    tag_map_t::const_iterator itr_last_elem = std::max_element(
                tag_map.begin(), tag_map.end()
           );

    if (itr_last_elem != tag_map.end()) {
        start_pos = itr_last_elem->Stop() + 1;
    }

    size_t pos_var = find_no_case(str_tmpl_txt, tag_type_prefix, start_pos);

    if (pos_var == wstring::npos) {
        return tag; // this tag object is EMPTY
    }

    // find the opening bracket
    size_t pos_tag_open = str_tmpl_txt.rfind(L'<', pos_var - 1);

    // NO bracket found
    if (pos_tag_open == string::npos) {
        syntax_ex error(L"Malformed tag");
        error.line = get_line_from_pos(pos_var);
        error.detail = L"Could not locate opening bracket";
        throw_exception(error);
    }

    // find the closing bracket after the templare variable prefix
    size_t pos_tag_close = str_tmpl_txt.find(L'>', pos_var + 1);

    if (pos_tag_close == string::npos) {
        syntax_ex error(L"Malformed tag");
        error.line = get_line_from_pos(pos_var);
        error.detail = L"Could not locate closing bracket";
        throw_exception(error);
    }

    // this is our complete tag
    wstring str_compl_tag
        = str_tmpl_txt.substr(pos_tag_open, pos_tag_close - pos_tag_open + 1);

    // get a ready-to-use tag object, catching any syntax exceptions
    try {
        tag = parse_tag(str_compl_tag);
    } catch (std::exception & ex) {
        syntax_ex error(L"Malformed tag");
        const char* c = ex.what();
        size_t len = strlen(c) / sizeof(char);
        if (len > 0)
        {
            wchar_t* w = new wchar_t[len + 1];
            size_t s = mbstowcs(w, c, len);
            if (s != len)
                w[0] = 0;
            else
                w[s] = 0;
            error.detail = std::wstring(w);
            delete[] w;
        }
        error.line =  get_line_from_pos(pos_tag_open);
        throw_exception(error);
    }

    // Set other tag parameters the tag parsing logic is not required for (since
    // the parsing logic is not aware of the global document context - it just
    // deals with one tag at a time).
    tag.Set_Start(pos_tag_open);
    tag.Set_Stop(pos_tag_close);

    return tag;
}
//----------------------------------------------------------------------------

const std::wstring html_template::truncate(const std::wstring & arg) const {
    size_t pos;
    wstring str(arg);

    do {
        pos = str.find_first_of(L"\t\r\n");

        if (pos != wstring::npos) {
            str.replace(pos,1,L"");
            --pos;
        }

    } while (pos != wstring::npos);

    return str;
}
//----------------------------------------------------------------------------

const tag_s html_template::parse_tag(const std::wstring & arg) const {
    // kill any return characters and whatnot
    const wstring str = truncate(arg);

    tag_s tag; // empty for now

#ifdef DEBUG3
    wcout << L"Parsing variable: " << str << endl;
#endif

    wstring str_last_token  = L"";
    wstring str_tag_type    = L"";
    wstring str_value       = L"";
    wstring str_escape_mode = L"";
    bool b_termination_tag = false;

    for (size_t n = 1; n < str.length() - 1; ++n) {
        const wchar_t & ch = str[n];

        // skip junk seperators
        if (ch == L'\t' ||  ch == L'\r' ||  ch == L'\n' || ch == L' ' || ch == L'=') continue;

#ifdef DEBUG3
        wcout << L"(" << ch << L")" << endl;
#endif

        // account for the tag terminator
        if (ch == L'/') {
            str_last_token = ch;
            b_termination_tag = true;
            continue;
        }

        wstring str_to_look_for = L"";

        // if this is a start of some token
        if (isalpha(ch) || ch == L'_' /* for our loop context vars */) {
            str_to_look_for = L">='\"\t\r\n ";
        }

        if (ch == L'"' || ch == L'\'') {
            str_to_look_for = ch;
        }

        size_t token_stop = str.find_first_of(str_to_look_for, n + 1);

        if (token_stop == wstring::npos) {
            throw std::runtime_error("Could not parse tag");
        }

        wstring str_token = str.substr(n, token_stop - n);

        // strip leading quotes from final token
        if (str_token.size() && (str_token[0] == L'\'' || str_token[0] == L'"')) {
            str_token.erase(0, 1);
        }

        // trim spaces
        while (str_token.at(str_token.size() - 1) == L' ') {
            str_token.resize(str_token.size() - 1);
        }

        // uppercase UNLESS this is an include directive
        if (str_tag_type != tag_type_prefix + L"INCLUDE") {
            uc(str_token);
        }

#ifdef DEBUG3
        wcout << L"current token: (" << str_token << L")" << endl;
#endif

        // if this is a tag type
        if (tag_strings_c.find(str_token) != tag_strings_c.end()) {
            str_tag_type = str_token;
        }

        // if there was a previous token, pair this one up and see what we get
        if (str_last_token.size()) {
#ifdef DEBUG3
            wcout << L"last token: (" << str_last_token << L")" << endl;
#endif

            // if that was a tag type
            if (tag_strings_c.find(str_last_token) != tag_strings_c.end()) {
                str_tag_type = str_last_token;

                if (reserved_words_c.find(str_token) == reserved_words_c.end()) {
                    str_value = str_token;
#ifdef DEBUG2
                    wcout << L"setting name to: " << str_value << endl;
#endif
                }
            }

            // if the last token is an escape keyword
            if (str_last_token == L"ESCAPE" || str_last_token == L"ESC") {
                str_escape_mode = str_token;
#ifdef DEBUG2
                wcout << L"escape found: " << str_escape_mode << endl;
#endif
            }

            // if the last token is NAME
            if (str_last_token == L"NAME") {
                str_value = str_token;
#ifdef DEBUG2
                wcout << L"setting name to: " << str_value << endl;
#endif
            }

            // if the last tag token was a termination token
            if (str_last_token == L"/") {
                // this MUST be a tag type
                if (tag_strings_c.find(str_token) == tag_strings_c.end()) {
                    throw std::runtime_error("Improper tag termination");
                }

                str_tag_type = str_token;
                // bail
                n = str.size() - 1;
                continue;
            }
        }

        // save the token as the last one
        str_last_token = str_token;

        // skip to next unprocessed character
        n += (token_stop - n);
    }

    if (str_value.empty() && str_last_token != L"/" && str_tag_type != tag_type_prefix + L"ELSE") {
        str_value = str_last_token;
#ifdef DEBUG2
        wcout << L"name still empty - using last token as name: " << str_value << endl;
#endif
    }

    // simple unnamed tags like <ELSE>
    if (!b_termination_tag && (str_tag_type.size() && !str_value.size())) {
        tag.Set_Is_Splitter(true);
    }

    // we know what the tag type is as a STRING, but we have to convert it into
    // a tag_s struct
    tag_types_t::const_iterator itr_tag_type = tag_types_c.begin();
    tag_type_s tag_type;

    for (; itr_tag_type != tag_types_c.end(); ++itr_tag_type) {
        tag_type = itr_tag_type->second;

        // if this matches the string tag type that we parsed out, this tag_s structure
        // is what we want
        if (str_tag_type == (tag_type_prefix + tag_type.str_tag_type)) {
            tag.Set_Tag_Type(tag_type);
        }
    }

    // set tag name if any
    if (str_value.size()) {
        tag.Set_Name(str_value);
    }

    // set termination flag if true
    if (b_termination_tag) {
        tag.Set_Is_Termination(b_termination_tag);
    }

    // set escape mode if any
    if (str_escape_mode.size()) {
        // make sure it's a valid escape
        if (str_escape_mode == L"JS" || str_escape_mode == L"JAVASCRIPT") {
            tag.Set_Escape_Mode(ESC_JS);
        } else if (str_escape_mode == L"HTML" || str_escape_mode == L"XML") {
            tag.Set_Escape_Mode(ESC_HTML);
        } else if (str_escape_mode == L"URL") {
            tag.Set_Escape_Mode(ESC_URL);
        } else {
            throw wruntime_error(L"Escape mode '" + str_escape_mode + L"' is not supported");
        }
    }

    return tag;
}
//----------------------------------------------------------------------------

const size_t
html_template::get_line_from_pos(const size_t & ui_pos) const {
    line_map_t::const_iterator itr_line = line_map.begin();

    for (; itr_line != line_map.end(); ++itr_line) {
        const size_t ui_line = itr_line->first;
        std::pair<size_t,size_t> line_boundary = itr_line->second;

        if (ui_pos >= line_boundary.first && ui_pos <= line_boundary.second) {
            return ui_line;
        }
    }

    return 0;
}
//----------------------------------------------------------------------------

void html_template::shift_tags(const std::wstring & str_in,
                               block_map_t & r_block_map,
                               const ptrdiff_t i_offset,
                               const size_t ui_start,
                               size_t ui_end /*=wstring::npos*/) {
    // if no end position specified, select end of document
    ui_end = ui_end != wstring::npos ? ui_end
             : str_in.length() - 1;

    block_map_t::iterator itr_block = r_block_map.begin();

    for (; itr_block != r_block_map.end(); ++itr_block) {
        itr_block->Shift(i_offset, ui_start, ui_end);
    }
}
//----------------------------------------------------------------------------

void html_template::escape_var(std::wstring & arg, const en_escape_mode escape_mode) {
    if (escape_mode == ESC_NONE) return;

    if (escape_mode == ESC_HTML) {
        search_replace(arg, L"&", L"&amp;");
        search_replace(arg, L"\"", L"&quot;");
        search_replace(arg, L"'", L"&#39");
        search_replace(arg, L"<", L"&lt;");
        search_replace(arg, L">", L"&gt;");
    }

    if (escape_mode == ESC_JS) {
        search_replace(arg, L"\\", L"\\\\");
        search_replace(arg, L"'", L"\\'");
        search_replace(arg, L"\"", L"\\\"");
        search_replace(arg, L"\n", L"\\n");
        search_replace(arg, L"\r", L"\\r");
    }

    if (escape_mode == ESC_URL) {
        arg = rfc1738_encode(arg);
    }
}
//----------------------------------------------------------------------------

void html_template::throw_exception(syntax_ex & ex) const {
    // add any extra data and rethrow
    ex.template_path = str_tmpl_file_name;
    throw ex;
}
//----------------------------------------------------------------------------

void html_template::throw_exception(runtime_ex & ex) const {
    // add any extra data and rethrow
    ex.template_path = str_tmpl_file_name;
    throw ex;
}

//----------------------------------------------------------------------------
// variable class public
//----------------------------------------------------------------------------
cls_variable::cls_variable(const std::wstring & arg_var_name) {
    str_name = arg_var_name;
}
//----------------------------------------------------------------------------

cls_variable & cls_variable::operator= (const loop_s & arg_table) {
    table = arg_table;
#ifdef DEBUG2
    wcout << L"assigning table variable " << Get_Name() << endl;
#endif
#ifdef DEBUG3
    wcout << L"table size " << table.Get_Rows().size() << endl;
#endif
    return *this;
}
//----------------------------------------------------------------------------
const std::wstring cls_variable::Get_Val_String() const {
    if (table.Get_Rows().empty()) {
        return L"";
    }

    loop_s::rows_t::const_iterator itr_row = table.Get_Rows().begin();
    row_s::cells_t::const_iterator itr_cell = itr_row->cells_c.find(L"");

    // could not find the cell, so this a table with not single value, and
    // cannot be converted to string
    if (itr_cell == itr_row->cells_c.end()) {
        return L"";
    }

    return itr_cell->second.str_val;
}
//----------------------------------------------------------------------------
// LOOP public
//----------------------------------------------------------------------------
bool loop_s::Empty() const {
    /*
      An empty loop is a one that has at least one row and if the row is NOT a
      single value (has key of "")
    */
    if (rows_c.empty()) {
        return true;
    }

    // not empty if over one row - we know it's not a single value
    if (rows_c.size() > 1) {
        return false;
    }

    // if got this far, check that the single row is not a single value container
    // (remember that even single values are stored in loop containers)

    loop_s::rows_t::const_iterator itr_row = rows_c.begin();

    row_s::cells_t::const_iterator itr_cell = itr_row->cells_c.find(L"");
    return !(rows_c.size() && itr_cell == itr_row->cells_c.end());
}
//----------------------------------------------------------------------------

const loop_s & cls_variable::Get_Val_Table() const {
    return table;
}

//----------------------------------------------------------------------------
// utilities
//----------------------------------------------------------------------------
void uc(std::wstring & str) {
    std::transform(str.begin(), str.end(), str.begin(), char_toupper);
}
//-----------------------------------------------------------------------

wchar_t char_toupper(wchar_t ch) {
    return static_cast<wchar_t>(toupper(ch));
}
//-----------------------------------------------------------------------

bool bin_predicate_search_nocase(wchar_t ch1, wchar_t ch2) {
    return toupper(ch1) == toupper(ch2);
}
//-----------------------------------------------------------------------

const wstring::size_type find_no_case(const std::wstring & str_src,
                                     const std::wstring & str_find,
                                     const size_t start_pos) {
    // begin searching where?
    wstring::const_iterator begin = str_src.begin();
    std::advance(begin, start_pos);

    // the integer result
    wstring::size_type pos = wstring::npos;

    // the iterator result
    wstring::const_iterator itr = std::search(
                                     begin, str_src.end(),
                                     str_find.begin(), str_find.end(),
                                     bin_predicate_search_nocase
                                );

    if (itr != str_src.end()) {
        pos = std::distance(str_src.begin(), itr); // get the zero-based index
    }

    return pos;
}
//-----------------------------------------------------------------------

const std::wstring file_directory(const std::wstring & str_path) {
    wstring str = trim_string(str_path);

    if (str.empty()) return L"";

    wstring::size_type last_slash_pos = str.find_last_of(L"\\/");

    // if its just the file name
    if (last_slash_pos == wstring::npos) {
        return L"";
    }

    str = str_path.substr(0, last_slash_pos + 1);

    // unix-like path is going to get lost if file is in root
    if (last_slash_pos == 0) {
        str = L"/";
    }

    return str;
}
//-----------------------------------------------------------------------

std::wstring trim_string(const std::wstring & str, const std::wstring & characters /*= L" \t\n\r"*/) {
    wstring str_ret(str);
    wstring::size_type pos = str_ret.find_first_not_of(characters);

    if (wstring::npos != pos)
        str_ret = str.substr(pos);

    pos = str_ret.find_last_not_of(characters);

    if (wstring::npos != pos) {
        str_ret = str_ret.substr(0, pos+1);
    } else {
        // it is still possible that 'str' contains only 'characters':
        if (wstring::npos != str_ret.find_first_of(characters))
            str_ret.erase();
    }

    return str_ret;
}
//-----------------------------------------------------------------------

void search_replace(std::wstring & str_src, const std::wstring& str_to_find,
                    const std::wstring & str_replace) {
    if (!str_src.length())     return;

    if (!str_to_find.length()) return;

    wstring::size_type find_pos  = str_src.find(str_to_find, 0);

    while (find_pos != wstring::npos) {
        str_src.replace(find_pos, str_to_find.length(), str_replace);
        find_pos += str_replace.length();
        // go again
        find_pos = str_src.find(str_to_find, find_pos);
    }
}
//----------------------------------------------------------------------------

/*
* Encode string per RFC1738 URL encoding rules
* tnx rstaveley
*/
const std::wstring rfc1738_encode(const std::wstring & src) {
    static wchar_t hex[] = L"0123456789ABCDEF";
    std::wstring dst;

    for (size_t i = 0; i < src.size(); i++) {
        if (isalnum(src[i])) {
            dst += src[i];
        } else if (src[i] == L' ') {
            dst += L'+';
        } else {
            dst += L'%';
            dst += hex[src[i] / 16];
            dst += hex[src[i] % 16];
        }
    }

    return dst;
} // rfc1738_encode

} // end namespace
