/* Copyright (C) 2024 Aryadev Chavali

 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License Version 2 for
 * more details.

 * You should have received a copy of the GNU General Public License Version 2
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.

 * Created: 2024-04-28
 * Author: Aryadev Chavali
 * Description:
 */

#include "test-base.h"
#include "test-darr.h"
#include "test-inst.h"

int main(int argc, char *argv[])
{
  if (argc > 1)
  {
    int a = atoi(argv[1]);
    switch (a)
    {
    case 1:
      RUN_TEST_SUITE(test_lib_base);
      break;
    case 2:
      RUN_TEST_SUITE(test_lib_darr);
      break;
    case 3:
      RUN_TEST_SUITE(test_lib_inst);
      break;
    default:
      goto all;
    }
  }
  else
  {
  all:
    RUN_TEST_SUITE(test_lib_base);
    RUN_TEST_SUITE(test_lib_darr);
    RUN_TEST_SUITE(test_lib_inst);
  }
  return 0;
}
