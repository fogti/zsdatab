/**********************************************
 *    part: table operators
 * library: zsdatable
 * package: zsdatab
 * SPDX-License-Identifier: LGPL-2.1-or-later
 **********************************************/

#include "zsdatable.hpp"
#include <sstream>

using namespace std;

namespace zsdatab {
  ostream& operator<<(ostream &stream, const table &tab) {
    return (stream << const_context(tab));
  }

  istream& operator>>(istream &stream, table &tab) {
    context ctx(tab, {});
    stream >> ctx;
    ctx.push();
    return stream;
  }
}
