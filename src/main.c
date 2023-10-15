/* Copyright (C) 2023 Aryadev Chavali

 * You may distribute and modify this code under the terms of the
 * GPLv2 license.  You should have received a copy of the GPLv2
 * license with this file.  If not, please write to:
 * aryadev@aryadevchavali.com.

 * Created: 2023-10-15
 * Author: Aryadev Chavali
 * Description: Entrypoint to program
 */

#include <stdio.h>
#include <string.h>

#include "./runtime.h"

int main(void)
{
  byte stack_data[256];
  vm_t vm = {0};
  vm_load_stack(&vm, stack_data, ARR_SIZE(stack_data));
  return 0;
}
