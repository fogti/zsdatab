/**********************************************
 *   class: zsdatab::table
 * library: zsdatable
 * package: zsdatab
 * SPDX-License-Identifier: LGPL-2.1-or-later
 **********************************************/

#include "common.hpp"
#include <fstream>
#include <iostream> // cerr

using namespace std;

namespace zsdatab {
  // concrete table implementations
  namespace intern {
    class permanent_table final : public permanent_table_common {
     public:
      explicit permanent_table(const string &name)
        : permanent_table_common(name)
      {
        {
          ifstream in((name + ".meta").c_str());
          if(in) {
            in >> _meta;
            _valid = !_meta.empty();
          }
        }
        if(!_valid) return;

        {
          ifstream in(_path.c_str());
          if(!in) _valid = false;
          else {
            // use an in-memory table to get input, copy to _data
            table tmpt(_meta);
            in >> tmpt;
            _data = move(tmpt).data_move_out();
          }
        }
      }

      ~permanent_table() noexcept {
        if(good() && _modified && !_path.empty()) {
#define FETPF "libzsdatable.so: ERROR: zsdatab::intern::permanent_table::~permanent_table() (write) failed: "
          try {
            ofstream out(_path.c_str());
            if(!out)
              cerr << FETPF << "table open failed\n";
            else
              out << make_table_ref(_meta, data());
          } catch(const length_error &e) {
            cerr << FETPF << "corrupt table data\n"
                    "  failure detected in: " << e.what() << '\n';
          } catch(const exception &e) {
            cerr << FETPF << "unknown error\n"
                    "  failure detected in: " << e.what() << '\n';
          } catch(...) {
            cerr << FETPF << "unknown error - untraceable\n";
          }
#undef FETPF
        }
      }
    };

    struct in_memory_table final : public table_impl_common {
      using table_impl_common::table_impl_common;

      bool good() const noexcept {
        return _meta.good();
      }

      auto clone() const -> std::shared_ptr<table_interface> {
        return make_shared<in_memory_table>(_meta, _data);
      }
    };
  }

  // for permanent tables
  table::table(const string &name)
    : _t(make_shared<intern::permanent_table>(name)) { }

  // for in-memory tables
  table::table(metadata meta)
    : _t(make_shared<intern::in_memory_table>(move(meta))) { }

  table::table(metadata meta, buffer_t n)
    : _t(make_shared<intern::in_memory_table>(move(meta), move(n))) { }

  void table::data(const buffer_t &n) {
    // copy on write
    if(n != _t->data()) {
      if(!experimental::get_underlying(_t).unique())
        _t = _t->clone();
      _t->data(n);
    }
  }

  auto table::clone() const -> std::shared_ptr<table_interface> {
    return _t->clone();
  }
}
