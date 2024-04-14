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
               VALID_DIGIT = "0123456789", VALID_HEX = "0123456789abcdefABCDEF";

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

token_t tokenise_literal_hex(string_view &source, size_t &column)
{
  // Remove x char from source
  source.remove_prefix(1);
  auto end = source.find_first_not_of(VALID_HEX);
  if (end == string::npos)
    end = source.size() - 1;
  string digits{source.substr(0, end)};
  source.remove_prefix(end);

  token_t t = {token_type_t::LITERAL_NUMBER, "0x" + digits, column};

  column += digits.size() + 1;
  return t;
}

pair<token_t, lerr_t> tokenise_literal_char(string_view &source, size_t &column)
{
  token_t t{};
  if (source.size() < 3)
    return make_pair(t, lerr_t::INVALID_CHAR_LITERAL);
  else if (source[1] == '\\')
  {
    // Escape sequence
    char escape = '\0';
    if (source.size() < 4 || source[3] != '\'')
      return make_pair(t, lerr_t::INVALID_CHAR_LITERAL_ESCAPE_SEQUENCE);
    switch (source[2])
    {
    case 'n':
      escape = '\n';
      break;
    case 't':
      escape = '\t';
      break;
    case 'r':
      escape = '\r';
      break;
    case '\\':
      escape = '\\';
      break;
    default:
      column += 2;
      return make_pair(t, lerr_t::INVALID_CHAR_LITERAL_ESCAPE_SEQUENCE);
      break;
    }
    t = token_t{token_type_t::LITERAL_CHAR, std::to_string(escape), column};
    column += 4;
    source.remove_prefix(4);
  }
  else
  {
    t = token_t(token_type_t::LITERAL_CHAR, std::to_string(source[1]));
    column += 3;
    source.remove_prefix(3);
  }
  return make_pair(t, lerr_t::OK);
}

token_t tokenise_literal_string(string_view &source, size_t &column, size_t end)
{
  source.remove_prefix(1);
  token_t token{token_type_t::LITERAL_STRING, string(source.substr(1, end - 1)),
                column};
  source.remove_prefix(end);
  column += end + 1;
  return token;
}

lerr_t tokenise_buffer(string_view source, std::vector<token_t> &tokens)
{
  size_t column = 0, line = 1;
  while (source.size() > 0)
  {
    bool is_token = true;
    char first    = source[0];
    token_t t{};
    if (isspace(first) || first == '\0')
    {
      size_t i;
      for (i = 0;
           i < source.size() && (isspace(source[i]) || source[i] == '\0'); ++i)
      {
        ++column;
        if (source[i] == '\n')
        {
          column = 0;
          ++line;
        }
      }
      ++column;
      source.remove_prefix(i);
      is_token = false;
    }
    else if (first == ';')
    {
      size_t i;
      for (i = 0; i < source.size() && source[i] != '\n'; ++i)
        continue;
      column = 0;
      ++line;
      source.remove_prefix(i + 1);
      is_token = false;
    }
    else if (first == '*')
    {
      t = token_t(token_type_t::STAR, "", column);
      source.remove_prefix(1);
    }
    else if (first == '\"')
    {
      auto end = source.find('\"', 1);
      if (end == string::npos)
        return lerr_t::INVALID_STRING_LITERAL;
      t = tokenise_literal_string(source, column, end);
    }
    else if (first == '\'')
    {
      lerr_t lerr;
      std::tie(t, lerr) = tokenise_literal_char(source, column);
      if (lerr != lerr_t::OK)
        return lerr;
    }
    else if (isdigit(first) ||
             (source.size() > 1 && first == '-' && isdigit(source[1])))
    {
      auto end = source.find_first_not_of(VALID_DIGIT, first == '-' ? 1 : 0);
      if (end == string::npos)
        end = source.size() - 1;
      else if (end != string::npos && !(isspace(source[end])))
        return lerr_t::INVALID_NUMBER_LITERAL;
      t = tokenise_literal_number(source, column);
    }
    else if (first == 'x' && source.size() > 1 &&
             is_char_in_s(source[1], VALID_HEX))
    {
      auto end = source.find_first_not_of(VALID_HEX);
      if (end == string::npos)
        end = source.size() - 1;
      else if (end != string::npos && !(isspace(source[end])))
        return lerr_t::INVALID_NUMBER_LITERAL;
      t = tokenise_literal_hex(source, column);
    }
    else if (is_char_in_s(first, VALID_SYMBOL))
    {
      lerr_t lerr;
      std::tie(t, lerr) = tokenise_symbol(source, column);
      if (lerr != lerr_t::OK)
        return lerr;
    }
    if (is_token)
    {
      t.line = line;
      tokens.push_back(t);
    }
  }
  return lerr_t::OK;
}
