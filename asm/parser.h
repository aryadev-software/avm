/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-24
 * Author: Aryadev Chavali
 * Description: Parser for assembly language
 */

#ifndef PARSER_H
#define PARSER_H

#include "./lexer.h"

#include <lib/inst.h>

typedef enum
{
  PERR_OK = 0,
  PERR_INTEGER_OVERFLOW,
  PERR_NOT_A_NUMBER,
  PERR_EXPECTED_UTYPE,
  PERR_EXPECTED_TYPE,
  PERR_EXPECTED_SYMBOL,
  PERR_EXPECTED_OPERAND,
  PERR_UNKNOWN_OPERATOR,
} perr_t;

const char *perr_as_cstr(perr_t);

perr_t parse_next_inst(token_stream_t *, inst_t *);
perr_t parse_stream(token_stream_t *, inst_t **, size_t *);

#endif
