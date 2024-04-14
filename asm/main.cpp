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
}
