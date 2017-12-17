/*************************************************
 *       header: zsdatab::intern::packed_table_common
 *      library: zsdatable
 *      package: zsdatab
 *      version: 0.2.6
 **************| *********************************
 *       author: Erik Kai Alain Zscheile
 *        email: erik.zscheile.ytrizja@gmail.com
 **************| *********************************
 * organisation: Ytrizja
 *     org unit: Zscheile IT
 *     location: Chemnitz, Saxony
 *************************************************
 *
 * Copyright (c) 2017 Erik Kai Alain Zscheile
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
#ifndef ZSDATAB_TABLE_PACKED_HPP
# define ZSDATAB_TABLE_PACKED_HPP 1
# include <iostream>
# include "table/common.hpp"
namespace zsdatab {
  namespace intern {
    template<class Tistream, class Tostream>
    struct packed_table_common final : public permanent_table_common {
      explicit packed_table_common(const std::string &name): permanent_table_common(name) {
        Tistream in(_path.c_str());
        if(!in) _valid = false;
        else {
          in >> _meta;
          _valid = !_meta.empty();

          table tmpt(_meta);
          in >> tmpt;
          data(tmpt.data());
        }
      }

      ~packed_table_common() noexcept {
        if(good() && _modified && !_path.empty()) {
#define FETPF "libzsdatable.so: ERROR: zsdatab::packed_table_common::~packed_table_common() (write) failed "
          try {
            Tostream out(_path.c_str());
            if(!out)
              std::cerr << FETPF << "(table open failed)\n";
            else
              out << _meta << table(_meta, data());
          } catch(const std::length_error &e) {
            std::cerr << FETPF << "(corrupt table data)\n"
                "  failure detected in: " << e.what() << '\n';
          } catch(const std::exception &e) {
            std::cerr << FETPF << "(unknown error)\n"
                "  failure detected in: " << e.what() << '\n';
          } catch(...) {
            std::cerr << FETPF << "(unknown error - untraceable)\n";
          }
#undef FETPF
        }
      }
    };

    template<class Tostream>
    bool create_packed_table_common(const std::string &_path, const metadata &_meta) {
      try {
        Tostream out(_path.c_str());
        if(out.good()) out << _meta;
        return out.good();
      } catch(...) {
        return false;
      }
    }

    template<class Tistream, class Tostream>
    table make_packed_table_common(const std::string &_path) {
      return table(std::make_shared<intern::packed_table_common<Tistream, Tostream>>(_path));
    }
  }
}
#endif /* ZSDATAB_TABLE_PACKED_HPP */
