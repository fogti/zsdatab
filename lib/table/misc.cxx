/**********************************************
 *    part: table operators
 *    part: extra table implementations (packed tables)
 * library: zsdatable
 * package: zsdatab
 * SPDX-License-Identifier: LGPL-2.1-or-later
 **********************************************/

#include "zsdatable.hpp"
#include "packed.hpp"
#include <3rdparty/gzstream/gzstream.h>
#include <fstream>
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

  using namespace intern;

  bool create_packed_table(const std::string &_path, const metadata &_meta) {
    return create_packed_table_common<ofstream>(_path, _meta);
  }

  table make_packed_table(const std::string &_path) {
    return make_packed_table_common<ifstream, ofstream>(_path);
  }

  using namespace zsdatab_3rdparty;

  bool create_gzipped_table(const string &_path, const metadata &_meta) {
    return create_packed_table_common<ogzstream>(_path, _meta);
  }

  table make_gzipped_table(const string &_path) {
    return make_packed_table_common<igzstream, ogzstream>(_path);
  }
}
