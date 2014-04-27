/***********************************************************************
* Copyright (C) 2009  Andrei Taranchenko
**********************************************************************/

/*
* Copyright (C) 2014 James Higley

Changed from std:string to std:wstring
*/


#ifndef test_h
#define test_h

#include "html_template.h"
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <codecvt>

//! test case structure
struct test_s {
//! template file
    std::wstring str_file;
//! error to catch, if any
    std::wstring str_error_to_catch;

//! ctructor
    test_s() {}
//! ctructor with template file name
    test_s(const std::wstring & arg_file) {
        str_file = arg_file;
    }
//! ctructor with template file name and exception to catch, to test syntax
// error catching
    test_s(const std::wstring & arg_file, const std::wstring & str_error) {
        str_file = arg_file;
        str_error_to_catch = str_error;
    }
};

//! type of container for tests
typedef std::vector <test_s> tests_t;

#endif
