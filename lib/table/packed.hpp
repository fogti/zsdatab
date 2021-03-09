/*************************************************
 *  header: zsdatab::intern::packed_table_common
 * library: zsdatable
 * package: zsdatab
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *************************************************/
#pragma once
#include "common.hpp"
#include <iostream>
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

        }
        if(_valid) {
          table tmpt(_meta);
          in >> tmpt;
          _data = std::move(tmpt).data_move_out();
        }
      }

      ~packed_table_common() noexcept {
        if(good() && _modified && !_path.empty()) {
#define FETPF "libzsdatable.so: ERROR: zsdatab::packed_table_common::~packed_table_common() (write) failed: "
          try {
            Tostream out(_path.c_str());
            if(!out)
              std::cerr << FETPF << "table open failed\n";
            else
              out << _meta << make_table_ref(_meta, data());
          } catch(const std::length_error &e) {
            std::cerr << FETPF << "corrupt table data\n"
                "  failure detected in: " << e.what() << '\n';
          } catch(const std::exception &e) {
            std::cerr << FETPF << "unknown error\n"
                "  failure detected in: " << e.what() << '\n';
          } catch(...) {
            std::cerr << FETPF << "unknown error - untraceable\n";
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
