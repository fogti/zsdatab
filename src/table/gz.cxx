/*************************************************
 *         part: extra table implementations (gzip tables)
 *      library: zsdatable
 *      package: zsdatab
 *      version: 0.2.1
 **************| *********************************
 *       author: Erik Kai Alain Zscheile
 *        email: erik.zscheile.ytrizja@gmail.com
 **************| *********************************
 * organisation: Ytrizja
 *     org unit: Zscheile IT
 *     location: Chemnitz, Saxony
 *************************************************
 *
 * Copyright (c) 2016 Erik Kai Alain Zscheile
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
 *************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <iostream>

#include "table/common.hpp"
#include "3rdparty/gzstream/gzstream.h"

namespace zsdatab {
  using namespace std;

  // concrete table implementations
  namespace intern {
    using namespace zsdatab_3rdparty;

    class gzipped_table : public permanent_table_common {
     public:
      explicit gzipped_table(const string &name): permanent_table_common(name) {
        igzstream in(_path.c_str());
        if(!in) _valid = false;
        else {
          in >> _meta;
          _valid = !_meta.empty();

          table tmpt(_meta);
          in >> tmpt;
          data(tmpt.data());
        }
      }

      ~gzipped_table() noexcept {
        static const string fetpf = "libzsdatable.so: ERROR: zsdatab::gzipped_table::~gzipped_table() (write) failed ";

        if(good() && _modified && !_path.empty()) {
          try {
            ogzstream out(_path.c_str());
            if(!out)
              cerr << fetpf << "(table open failed)\n";
            else
              out << _meta << table(_meta, data());
          } catch(const length_error &e) {
            cerr << fetpf << "(corrupt table data)\n"
                    "  failure detected in: " << e.what() << '\n';
          } catch(const exception &e) {
            cerr << fetpf << "(unknown error)\n"
                    "  failure detected in: " << e.what() << '\n';
          } catch(...) {
            cerr << fetpf << "(unknown error - untraceable)\n";
          }
        }
      }
    };
  }

  table make_gzipped_table(const std::string &_path) {
    return table(make_shared<intern::gzipped_table>(_path));
  }
}
