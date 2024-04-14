/* Copyright (C) 2024 Aryadev Chavali

 * You may distribute and modify this code under the terms of the GPLv2
 * license.  You should have received a copy of the GPLv2 license with
 * this file.  If not, please write to: aryadev@aryadevchavali.com.

 * Created: 2024-04-14
 * Author: Aryadev Chavali
 * Description: Preprocessor which occurs after lexing before parsing.
 */

#ifndef PREPROCESSER_HPP
#define PREPROCESSER_HPP

#include <tuple>

#include "./lexer.hpp"

enum pp_err_t
{
  OK = 0,
  EXPECTED_NAME,
  EXPECTED_STRING,
  EXPECTED_END,
  FILE_NONEXISTENT,
  FILE_PARSE_ERROR,
  UNKNOWN_NAME,
};

std::pair<std::vector<token_t>, pp_err_t>
preprocesser(const std::vector<token_t> &);

std::pair<std::vector<token_t>, pp_err_t>
preprocess_macro_blocks(const std::vector<token_t> &);

std::pair<std::vector<token_t>, pp_err_t>
preprocess_use_blocks(const std::vector<token_t> &);

#endif
