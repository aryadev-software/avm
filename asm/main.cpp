/* Copyright (C) 2024 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2024-04-14
 * Author: Aryadev Chavali
 * Description: Entrypoint for assembly program
 */

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <lib/inst.h>

#include "./base.hpp"
#include "./lexer.hpp"
#include "./preprocesser.hpp"

using std::pair, std::string, std::string_view, std::vector;

void usage(const char *program_name, FILE *fp)
{
  fprintf(fp,
          "Usage: %s FILE OUT-FILE\n"
          "\tFILE: Source code to compile\n"
          "\tOUT-FILE: Name of file to store bytecode\n",
          program_name);
}

int main(int argc, const char *argv[])
{
  if (argc == 1 || argc > 3)
  {
    usage(argv[0], stderr);
    return -1;
  }
  int ret               = 0;
  const char *file_name = argv[1];
  auto file_source      = read_file(file_name);

  string source_str;
  string_view original;
  string_view src;
  vector<token_t *> tokens, preprocessed_tokens;
  lerr_t lerr;
  pp_err_t pp_err;

  // Highest scoped variable cut off point

  if (file_source.has_value())
    source_str = file_source.value();
  else
  {
    std::cerr << "ERROR: file `" << file_name << "` does not exist!"
              << std::endl;
    ret = -1;
    goto end;
  }
  original = string_view{source_str};
  src      = string_view{source_str};
  lerr     = tokenise_buffer(src, tokens);

  if (lerr != lerr_t::OK)
  {
    size_t col = 0, line = 1;
    // TODO: Fix this UB (probably a change in API)
    auto diff = src.begin() - original.begin();
    for (auto i = 0; i < diff; ++i)
    {
      if (source_str[i] == '\n')
      {
        col = 0;
        ++line;
      }
      else
        ++col;
    }
    std::cerr << file_name << ":" << line << ":" << col << ":"
              << lerr_as_cstr(lerr) << std::endl;
    ret = 255 - static_cast<int>(lerr);
    goto end;
  }
  else
  {
    std::cout << "LEXER: \n"
                 "-------------------------------------------------------------"
                 "-------------------\n";
    for (auto token : tokens)
      std::cout << "\t" << *token << std::endl;
    std::cout << "-------------------------------------------------------------"
                 "-------------------\n";
  }

  // preprocessing
  std::tie(preprocessed_tokens, pp_err) = preprocesser(tokens);
  if (pp_err.type != pp_err_type_t::OK)
  {
    std::cerr << file_name << ":" << pp_err.reference->line << ":"
              << pp_err.reference->column << ":" << pp_err << std::endl;
    ret = 255 - static_cast<int>(pp_err.type);
    goto end;
  }
  else
  {
    std::cout << "PREPROCESSER: \n"
                 "-------------------------------------------------------------"
                 "-------------------\n";
    for (auto token : preprocessed_tokens)
      std::cout << "\t" << *token << std::endl;
    std::cout << "-------------------------------------------------------------"
                 "-------------------\n";
  }

end:
  for (auto token : tokens)
    delete token;
  for (auto token : preprocessed_tokens)
    delete token;

  return ret;
}
