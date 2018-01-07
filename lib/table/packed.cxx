/***********************************************************
 *         part: extra table implementations (packed tables)
 *      library: zsdatable
 *      package: zsdatab
 *      version: 0.2.8
 **************| *******************************************
 *       author: Erik Kai Alain Zscheile
 *        email: erik.zscheile.ytrizja@gmail.com
 **************| *******************************************
 * organisation: Ytrizja
 *     org unit: Zscheile IT
 *     location: Chemnitz, Saxony
 ***********************************************************
 *
 * Copyright (c) 2018 Erik Kai Alain Zscheile
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ***********************************************************/

#include <fstream>
#include <gzstream.h>
#include "table/packed.hpp"

namespace zsdatab {
  using namespace std;
  using namespace intern;

  bool create_packed_table(const std::string &_path, const metadata &_meta) {
    return create_packed_table_common<ofstream>(_path, _meta);
  }

  table make_packed_table(const std::string &_path) {
    return make_packed_table_common<ifstream, ofstream>(_path);
  }

  using namespace GZSTREAM_NAMESPACE;

  bool create_gzipped_table(const string &_path, const metadata &_meta) {
    return create_packed_table_common<ogzstream>(_path, _meta);
  }

  table make_gzipped_table(const string &_path) {
    return make_packed_table_common<igzstream, ogzstream>(_path);
  }
}
