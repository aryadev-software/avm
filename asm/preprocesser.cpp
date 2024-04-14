/* Copyright (C) 2024 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2024-04-14
 * Author: Aryadev Chavali
 * Description: Preprocessor which occurs after lexing before parsing.
 */

#include "./preprocesser.hpp"
#include "./base.hpp"

#include <algorithm>
#include <unordered_map>

using std::pair, std::vector, std::make_pair, std::string, std::string_view;

#define ERR(E) std::make_pair(tokens, (E))
#define VAL(E) std::make_pair(E, pp_err_t{pp_err_type_t::OK})

pair<vector<token_t *>, pp_err_t>
preprocess_use_blocks(vector<token_t *> tokens)
{
  vector<token_t *> new_tokens;
  for (size_t i = 0; i < tokens.size(); ++i)
  {
    token_t *t = tokens[i];
    if (t->type == token_type_t::PP_USE)
    {
      if (i + 1 >= tokens.size() ||
          tokens[i + 1]->type != token_type_t::LITERAL_STRING)
      {
        new_tokens.clear();
        return ERR(pp_err_t(pp_err_type_t::EXPECTED_STRING, t));
      }

      token_t *name = tokens[i + 1];
      auto source   = read_file(name->content.c_str());
      if (!source)
      {
        new_tokens.clear();
        return ERR(pp_err_t(pp_err_type_t::FILE_NONEXISTENT, name));
      }

      std::vector<token_t *> ftokens;
      lerr_t lerr = tokenise_buffer(source.value(), ftokens);
      if (lerr != lerr_t::OK)
      {
        new_tokens.clear();
        return ERR(pp_err_t(pp_err_type_t::FILE_PARSE_ERROR, name, lerr));
      }

      new_tokens.insert(new_tokens.end(), ftokens.begin(), ftokens.end());

      i += 2;
    }
    else
      new_tokens.push_back(new token_t{*t});
  }
  return VAL(new_tokens);
}

pp_err_t::pp_err_t(pp_err_type_t e)
    : reference{nullptr}, type{e}, lerr{lerr_t::OK}
{}

pp_err_t::pp_err_t(pp_err_type_t err, const token_t *ref)
    : reference{ref}, type{err}
{}

pp_err_t::pp_err_t(pp_err_type_t err, const token_t *ref, lerr_t lerr)
    : reference{ref}, type{err}, lerr{lerr}
{}
