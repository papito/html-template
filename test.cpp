/***********************************************************************
 * Copyright (C) 2009  Andrei Taranchenko
 **********************************************************************/

/*
 * Copyright (C) 2014 James Higley

   Changed from std:string to std:wstring
*/


#ifndef test_c
#define test_c
#include "test.h"
#endif

#ifdef WIN
#include <conio.h>
#endif

using namespace std;
using namespace tmpl;

html_template templ;
tests_t tests_c;

// convert a file.tmpl into a file.txt
wstring get_reference_file(const wstring& str_file_name) {
    size_t dot_pos = str_file_name.rfind(L'.', str_file_name.length());
    wstring str_ret = L"";

    if ( dot_pos != wstring::npos && dot_pos + 1 < str_file_name.length() - 1 ) {
        str_ret = str_file_name;
        str_ret.replace(dot_pos + 1, str_file_name.length() - dot_pos, L"txt");
    }

    return str_ret;
}
//----------------------------------------------------------------------------

void test(const tests_t & tests_c, const wstring& str_test_name) {
    wcout << str_test_name << L" ";

    for (unsigned int n=0; n < tests_c.size(); n++) {
        const test_s & t = tests_c[n];

        wstring str_template_output = L"";

        try {
            templ.Set_Template_File( t.str_file );
            str_template_output = templ.Process();
        } catch (tmpl::syntax_ex & ex) {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cv;
            if (t.str_error_to_catch == cv.from_bytes(ex.what())) {
                wcout << L".";
                continue;
            } else { // NOT the type of error we expect, rethrow
                throw ex;
            }
        }


        // if there is an error to catch - we didn't catch it
        if ( t.str_error_to_catch.length() ) {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cv;
            throw(string("Uncaught error: '") + cv.to_bytes(t.str_error_to_catch).c_str() + "'");
        }

        const wstring str_ref_file_path = get_reference_file( t.str_file );

        std::wifstream in_stream;
        in_stream.open(str_ref_file_path.c_str());

        if( !in_stream.is_open() ) {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cv;
            throw(string("Could not open reference test file '")
                  + cv.to_bytes(str_ref_file_path).c_str()
                  + "'. All tests aborted");
        }

        std::wstringstream oss;
        oss << in_stream.rdbuf();
        wstring str_reference_output = oss.str();

        in_stream.close();

        const bool b_match = (str_template_output == str_reference_output);

        if (b_match == true) {
            wcout << L".";
        } else {
            wcout << L" FAIL: '" << t.str_file << L"'" << endl;
            wcout << L"Output left in 'fail.txt'" << endl;
            wofstream out_stream;
            out_stream.open(L"fail.txt", std::ios::out);
            out_stream.write(str_template_output.c_str(),
                             str_template_output.length());
            out_stream.close();

#ifdef WIN
            wcout << L"\nPress any key to QUIT";
            _getch();
#endif

            exit(1);
        }
    }

    wcout << L" Pass" << endl;
}
//----------------------------------------------------------------------------

void run() {
    wcout << L"[ Testing ]" << endl;
    tests_t simple;
    simple.push_back( test_s(L"t/simple.tmpl") );
    simple.push_back( test_s(L"t/no_name.tmpl") );
    simple.push_back( test_s(L"t/simple2.tmpl") );
    simple.push_back( test_s(L"t/malformed_tag1.tmpl") );
    simple.push_back( test_s(L"t/malformed_tag3.tmpl") );
    simple.push_back( test_s(L"t/malformed_tag4.tmpl") );
    simple.push_back( test_s(L"t/malformed_tag5.tmpl") );
    simple.push_back( test_s(L"t/name_var.tmpl") );
    test(simple, L"Substitution");

    tests_t cond;
    cond.push_back( test_s(L"t/if.tmpl") );
    cond.push_back( test_s(L"t/if2.tmpl") );
    cond.push_back( test_s(L"t/if3.tmpl") );
    cond.push_back( test_s(L"t/if4.tmpl") );
    cond.push_back( test_s(L"t/if_else.tmpl") );
    cond.push_back( test_s(L"t/if_else2.tmpl") );
    cond.push_back( test_s(L"t/if_else3.tmpl") );
    cond.push_back( test_s(L"t/if_and_vars.tmpl") );
    cond.push_back( test_s(L"t/unless.tmpl") );
    cond.push_back( test_s(L"t/unless_else.tmpl") );
    test(cond, L"Conditional");

    tests_t loops;
    loops.push_back( test_s(L"t/loop.tmpl") );
    loops.push_back( test_s(L"t/loop_globals.tmpl") );
    loops.push_back( test_s(L"t/empty_loop.tmpl") );
    loops.push_back (test_s(L"t/loop_with_if.tmpl") );
    loops.push_back (test_s(L"t/nested_loop.tmpl") );
    loops.push_back (test_s(L"t/context_vars.tmpl") );
    test(loops, L"Loop");

    tests_t include;
    include.push_back( test_s(L"t/includer.tmpl") );
    include.push_back( test_s(L"t/includer2.tmpl") );
    test(include, L"Includes");

    tests_t encodings;
    encodings.push_back( test_s(L"t/escape.tmpl") );
    test(encodings, L"Encodings");

    tests_t exceptions;
    exceptions.push_back( test_s(L"t/overlapping_ifs.tmpl", L"Block not properly terminated") );
    exceptions.push_back( test_s(L"t/open_loop.tmpl", L"Block not terminated") );
    exceptions.push_back( test_s(L"t/malformed_tag2.tmpl", L"Malformed tag") );
    test(exceptions, L"Exceptions");

    tests_t all;
    all.push_back( test_s(L"t/all.tmpl") );
    test(all, L"Combined");
}
//----------------------------------------------------------------------------

void create(const wstring& str_tmpl_file = L"") {
    // if we are trying to create a single reference file, overwrite the global
    // test list
    if ( str_tmpl_file.length() ) {
        tests_c.clear();
        tests_c.push_back( test_s(str_tmpl_file) );
    }

    wcout << L"\nCreating " << tests_c.size() << L" reference tests...\n" << endl;

    for (unsigned int n=0; n < tests_c.size(); n++) {
        test_s & r_test = tests_c[n];
        templ.Set_Template_File( r_test.str_file );

        const wstring str_out_file = get_reference_file( r_test.str_file );

        wofstream out_stream;
        out_stream.open(str_out_file.c_str(), std::ios::out);

        const wstring str_reference_output = templ.Process();

        out_stream.write(str_reference_output.c_str(),
                         str_reference_output.length());
        out_stream.close();
        wcout << r_test.str_file << endl;
    }
}
//----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    try {
        tmpl::row_t row;
        tmpl::loop_t table;
        tmpl::row_t row2;
        tmpl::loop_t table2;
        tmpl::row_t row3;
        tmpl::loop_t table3;

        tmpl::loop_t empty_table;

        row2(L"table_2_column_1") = L"TABLE 2 ROW 1 COLUMN 1";
        row2(L"table_2_column_2") = L"TABLE 2 ROW 1 COLUMN 2";
        row2(L"table_2_column_3") = L"I GOT THE WHOLE WOLRD IN MY HAND... I GOT THE WHOLE WIDE WORLD, IN MY HAND";
        table2 += row2;
        row2(L"table_2_column_1") = L"TABLE 2 ROW 2 COLUMN 1";
        row2(L"table_2_column_2") = L"TABLE 2 ROW 2 COLUMN 2";
        row2(L"table_2_column_3") = L"I GOT THE WHOLE WOLRD IN MY HAND... I GOT THE WHOLE WIDE WORLD, IN MY HAND";
        table2 += row2;

        row3(L"table_3_column_1") = L"TABLE 3 ROW 1 COLUMN 1";
        table3 += row3;

        row(L"table_1_column_1") = L"TABLE 1 ROW 1 COLUMN 1";
        row(L"table_1_column_2") = L"TABLE 1 ROW 1 COLUMN 2";
        row(L"table_1_column_3") = L"I GOT THE WHOLE WOLRD IN MY HAND... I GOT THE WHOLE WIDE WORLD, IN MY HAND";
        row(L"LOOP2") = table2;
        row(L"LOOP3") = table3;
        table += row;
        row(L"table_1_column_1") = L"TABLE 1 ROW 2 COLUMN 1";
        row(L"table_1_column_2") = L"TABLE 1 ROW 2 COLUMN 2";
        row(L"table_1_column_3") = L"I GOT THE WHOLE WOLRD IN MY HAND... I GOT THE WHOLE WIDE WORLD, IN MY HAND";
        row(L"LOOP2") = table2;
        row(L"LOOP3") = table3;
        table += row;
        row(L"table_1_column_1") = L"TABLE 1 ROW 3 COLUMN 1";
        row(L"table_1_column_2") = L"TABLE 1 ROW 3 COLUMN 2";
        row(L"table_1_column_3") = L"I GOT THE WHOLE WOLRD IN MY HAND... I GOT THE WHOLE WIDE WORLD, IN MY HAND";
        row(L"LOOP2") = table2;
        row(L"LOOP3") = table3;
        table += row;

        templ(L"VAR")     = L"VARIABLE 1";
        templ(L"VAR2")    = L"VERY VERY VERY LONG VARIABLE 2";
        templ(L"CHAR")    = L"V";
        templ(L"INT")     = 123456789;
        templ(L"NEG_INT") = -123456789;
        templ(L"VAR_DBL") = 1234.5678;
        templ(L"EMPTY1")  = 0;
        templ(L"EMPTY2")  = L"";
        templ(L"EMPTY3")  = L"  ";
        templ(L"EMPTY4")  = L"0000 ";
        templ(L"NEMPTY1") = L" 000.00";
        templ(L"LOOP1")   = table;
        templ(L"EMPTY_LOOP") = empty_table;
        templ(L"TEXT")       = L"Cheney was of military age and a supporter \
of the Vietnam War but he did not serve in the war, applying for and \
receiving five draft deferments. On May 19, 1965, Cheney was classified \
as 1-A , \"available for service\" by the Selective Service. On October 26, \
1965 the Selective Service lifted the constraints on drafting childless \
married men. Cheney and his wife then had a child after which he applied \
for and received, a reclassification of 3-A, gaining him a final draft \
deferment.\n\nIn an interview with George C. Wilson that appeared in the \
April 5, 1989 issue of The Washington Post, when asked about his deferments \
the future Defense Secretary said, \"I had other priorities in the '60s than \
military service.\"" ;
        templ(L"RESULT_SET") = L"";
        templ(L"SEARCH_PARAM") = L"Jessica Simpson";
        templ(L"SEARCH_PARAM2") = L"get a life";
        templ(L"NAME") = L"NAME";
        templ(L"ENCODED") = L"Tom & Jerry is \"funny\"";
        templ(L"ENCODED2") = L"\\ ' \" \n";
        templ(L"ENCODED3") = L"Dolce & Gabbana";

        // for testing global var overrides
        templ(L"table_1_column_1") = L"I_MUST_BE_OVERWRITTEN_IN_LOOP";

        if (argc == 2 && strcmp(argv[1], "create") == 0) {
            create();
        }
        // if creating a single test
        else if (argc == 3 && strcmp(argv[1], "create") == 0) {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cv;
            wstring str_tmpl_file = cv.from_bytes(argv[2]);
            create( str_tmpl_file );
        }
        // if running a single test
        else if (argc == 3 && strcmp(argv[1], "run") == 0) {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cv;
            wstring str_tmpl_file = cv.from_bytes(argv[2]);
            tests_t single_test;
            single_test.push_back( test_s(str_tmpl_file) );
            test(single_test, str_tmpl_file);
        } else {
            run();
        }
    } catch (tmpl::runtime_ex & ex) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cv;
        wcout << L"\nCGI TEMPLATE ERROR: " << cv.from_bytes(ex.what()) << L"\n";
        wcout << L"CGI TEMPLATE FILE: " << ex.template_path << L"\n";
    } catch (tmpl::syntax_ex & ex) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> cv;
        wcout << L"\nSYNTAX ERROR: " << cv.from_bytes(ex.what()) << L"\n";
        wcout << L"LINE: " << ex.line << L"\n";
        wcout << L"DETAIL: " << ex.detail << L"\n";
        wcout << L"PATH: " << ex.template_path << L"\n";
    } catch (std::exception & ex) {
        cout << "\nEXCEPTION: " << ex.what() << "\n";
    } catch (string & str) {
        cout << "\nERROR: " << str << "\n";
    }

#ifdef WIN
    wcout << L"\nPress any key to QUIT";
    _getch();
#endif

    return 0;
}

