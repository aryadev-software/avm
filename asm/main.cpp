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

#include "./lexer.hpp"

using std::pair, std::string, std::string_view, std::vector;

std::optional<string> read_file(const char *filename)
{
  FILE *fp = fopen(filename, "rb");
  if (fp)
  {
    string contents;
    fseek(fp, 0, SEEK_END);
    contents.resize(ftell(fp));
    rewind(fp);
    fread(&contents[0], 1, contents.size(), fp);
    fclose(fp);

    return contents;
  }
  else
    return std::nullopt;
}

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
  usage(argv[1], stdout);
  return 0;
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
  vector<token_t> tokens;
  lerr_t lerr;

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
    for (auto token : tokens)
    {
      std::cout << "\t" << token << std::endl;
    }
  }

end:
  return ret;
}
