/* Copyright (C) 2024 Aryadev Chavali

 * You may distribute and modify this code under the terms of the GPLv2
 * license.  You should have received a copy of the GPLv2 license with
 * this file.  If not, please write to: aryadev@aryadevchavali.com.

 * Created: 2024-04-14
 * Author: Aryadev Chavali
 * Description: Lexer for assembly language
 */

#include <lib/inst.h>

#include <algorithm>
#include <tuple>

#include "./lexer.hpp"

using std::string, std::string_view, std::pair, std::make_pair;

constexpr auto VALID_SYMBOL = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUV"
                              "WXYZ0123456789-_.:()%#$",
               VALID_DIGIT  = "0123456789";

bool is_char_in_s(char c, const char *s)
{
  return string_view(s).find(c) != string::npos;
}

bool initial_match(string_view src, string_view match)
{
  return (src.size() > match.size() && src.substr(0, match.size()) == match);
}

pair<token_t, lerr_t> tokenise_symbol(string_view &source, size_t &column)
{
  auto end = source.find_first_not_of(VALID_SYMBOL);
  if (end == string::npos)
    end = source.size() - 1;
  string sym{source.substr(0, end)};
  source.remove_prefix(end);
  std::transform(sym.begin(), sym.end(), sym.begin(), ::toupper);

  token_t t{};

  if (initial_match(sym, "%CONST"))
  {
    t = token_t(token_type_t::PP_CONST, sym.substr(6));
  }
  else if (sym == "%USE")
  {
    t.type = token_type_t::PP_USE;
  }
  else if (sym == "%END")
  {
    t.type = token_type_t::PP_END;
  }
  else if (sym[0] == '%')
  {
    return make_pair(t, lerr_t::INVALID_PREPROCESSOR_DIRECTIVE);
  }
  else if (sym.size() > 1 && sym[0] == '$')
  {
    t = token_t(token_type_t::PP_REFERENCE, sym.substr(1));
  }
  else if (sym == "NOOP")
  {
    t.type = token_type_t::NOOP;
  }
  else if (sym == "HALT")
  {
    t.type = token_type_t::HALT;
  }
  else if (initial_match(sym, "PUSH.REG."))
  {
    t = token_t(token_type_t::PUSH_REG, sym.substr(9));
  }
  else if (initial_match(sym, "PUSH."))
  {
    t = token_t(token_type_t::PUSH, sym.substr(5));
  }
  else if (initial_match(sym, "POP."))
  {
    t = token_t(token_type_t::POP, sym.substr(4));
  }
  else if (initial_match(sym, "MOV."))
  {
    t = token_t(token_type_t::MOV, sym.substr(4));
  }
  else if (initial_match(sym, "DUP."))
  {
    t = token_t(token_type_t::DUP, sym.substr(4));
  }
  else if (initial_match(sym, "MALLOC.STACK."))
  {
    t = token_t(token_type_t::MALLOC_STACK, sym.substr(13));
  }
  else if (initial_match(sym, "MALLOC."))
  {
    t = token_t(token_type_t::MALLOC, sym.substr(7));
  }
  else if (initial_match(sym, "MSET.STACK."))
  {
    t = token_t(token_type_t::MSET_STACK, sym.substr(11));
  }
  else if (initial_match(sym, "MSET."))
  {
    t = token_t(token_type_t::MSET, sym.substr(5));
  }
  else if (initial_match(sym, "MGET.STACK."))
  {
    t = token_t(token_type_t::MGET_STACK, sym.substr(11));
  }
  else if (initial_match(sym, "MGET."))
  {
    t = token_t(token_type_t::MGET, sym.substr(5));
  }
  else if (sym == "MDELETE")
  {
    t.type = token_type_t::MDELETE;
  }
  else if (sym == "MSIZE")
  {
    t.type = token_type_t::MSIZE;
  }
  else if (initial_match(sym, "NOT."))
  {
    t = token_t(token_type_t::NOT, sym.substr(4));
  }
  else if (initial_match(sym, "OR."))
  {
    t = token_t(token_type_t::OR, sym.substr(3));
  }
  else if (initial_match(sym, "AND."))
  {
    t = token_t(token_type_t::AND, sym.substr(4));
  }
  else if (initial_match(sym, "XOR."))
  {
    t = token_t(token_type_t::XOR, sym.substr(4));
  }
  else if (initial_match(sym, "EQ."))
  {
    t = token_t(token_type_t::EQ, sym.substr(3));
  }
  else if (initial_match(sym, "LTE."))
  {
    t = token_t(token_type_t::LTE, sym.substr(4));
  }
  else if (initial_match(sym, "LT."))
  {
    t = token_t(token_type_t::LT, sym.substr(3));
  }
  else if (initial_match(sym, "GTE."))
  {
    t = token_t(token_type_t::GTE, sym.substr(4));
  }
  else if (initial_match(sym, "GT."))
  {
    t = token_t(token_type_t::GT, sym.substr(3));
  }
  else if (initial_match(sym, "SUB."))
  {
    t = token_t(token_type_t::SUB, sym.substr(4));
  }
  else if (initial_match(sym, "PLUS."))
  {
    t = token_t(token_type_t::PLUS, sym.substr(5));
  }
  else if (initial_match(sym, "MULT."))
  {
    t = token_t(token_type_t::MULT, sym.substr(5));
  }
  else if (initial_match(sym, "PRINT."))
  {
    t = token_t(token_type_t::PRINT, sym.substr(6));
  }
  else if (sym == "JUMP.ABS")
  {
    t.type = token_type_t::JUMP_ABS;
  }
  else if (sym == "JUMP.STACK")
  {
    t.type = token_type_t::JUMP_STACK;
  }
  else if (initial_match(sym, "JUMP.IF."))
  {
    t = token_t(token_type_t::JUMP_IF, sym.substr(8));
  }
  else if (sym == "CALL.STACK")
  {
    t.type = token_type_t::CALL_STACK;
  }
  else if (sym == "CALL")
  {
    t.type = token_type_t::CALL;
  }
  else if (sym == "RET")
  {
    t.type = token_type_t::RET;
  }
  else if (sym == "GLOBAL")
  {
    t.type = token_type_t::GLOBAL;
  }
  else
  {
    t.type = token_type_t::SYMBOL;
  }

  if (t.content == "")
    t.content = sym;
  t.column = column;
  column += sym.size();
  return make_pair(t, lerr_t::OK);
}

token_t tokenise_literal_number(string_view &source, size_t &column)
{
  bool is_negative = false;
  if (source[0] == '-')
  {
    is_negative = true;
    source.remove_prefix(1);
  }

  auto end = source.find_first_not_of(VALID_DIGIT);
  if (end == string::npos)
    end = source.size() - 1;
  string digits{source.substr(0, end)};
  source.remove_prefix(end);

  token_t t{token_type_t::LITERAL_NUMBER, (is_negative ? "-" : "") + digits,
            column};

  column += digits.size() + (is_negative ? 1 : 0);

  return t;
}
