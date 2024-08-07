/* Copyright (C) 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License Version 2 for
 * more details.

 * You should have received a copy of the GNU General Public License Version 2
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

 * Created: 2024-04-25
 * Author: Aryadev Chavali
 * Description: Virtual machine data structures and some helpers
 */

#ifndef STRUCT_H
#define STRUCT_H

#include <lib/darr.h>
#include <lib/heap.h>
#include <lib/inst.h>

struct Registers
{
  byte_t *bytes;
  size_t size;
};

struct Stack
{
  byte_t *data;
  size_t ptr, max;
};

struct Program
{
  prog_t data;
  word_t ptr;
};

struct CallStack
{
  word_t *address_pointers;
  size_t ptr, max;
};

#define VM_NTH_REGISTER(REGISTERS, N)     (((word_t *)((REGISTERS).bytes))[N])
#define VM_REGISTERS_AVAILABLE(REGISTERS) (((REGISTERS).size) / WORD_SIZE)

typedef struct
{
  struct Registers registers;
  struct Stack stack;
  heap_t heap;

  struct CallStack call_stack;
  struct Program program;
} vm_t;

// Start and stop
void vm_load_stack(vm_t *, byte_t *, size_t);
void vm_load_registers(vm_t *, byte_t *, size_t);
void vm_load_heap(vm_t *, heap_t);
void vm_load_program(vm_t *, prog_t);
void vm_load_call_stack(vm_t *, word_t *, size_t);
void vm_stop(vm_t *);

// Printing the VM
#define VM_PRINT_PROGRAM_EXCERPT 5
void vm_print_registers(vm_t *, FILE *);
void vm_print_stack(vm_t *, FILE *);
void vm_print_program(vm_t *, FILE *);
void vm_print_heap(vm_t *, FILE *);
void vm_print_call_stack(vm_t *, FILE *);
void vm_print_all(vm_t *, FILE *);

#endif
