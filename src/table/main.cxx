/*************************************************
 *        class: zsdatab::table
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <utility>

#include "table/common.hpp"

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
            data(tmpt.data());
          }
        }
      }

      ~permanent_table() noexcept {
        if(good() && _modified && !_path.empty()) {
#define FETPF "libzsdatable.so: ERROR: zsdatab::intern::permanent_table::~permanent_table() (write) failed "
          try {
            ofstream out(_path.c_str());
            if(!out)
              cerr << FETPF << "(table open failed)\n";
            else
              out << table(_meta, data());
          } catch(const length_error &e) {
            cerr << FETPF << "(corrupt table data)\n"
                    "  failure detected in: " << e.what() << '\n';
          } catch(const exception &e) {
            cerr << FETPF << "(unknown error)\n"
                    "  failure detected in: " << e.what() << '\n';
          } catch(...) {
            cerr << FETPF << "(unknown error - untraceable)\n";
          }
#undef FETPF
        }
      }
    };

    class in_memory_table final : public table_interface {
      const metadata _meta;
      buffer_t _data;

     public:
      explicit in_memory_table(const metadata &o):
        _meta(o) { }

      in_memory_table(const metadata &o, const buffer_t &n):
        _meta(o), _data(n) { }

      bool good() const noexcept {
        return _meta.good();
      }

      auto get_metadata() const noexcept -> const metadata& {
        return _meta;
      }

      auto data() const noexcept -> const buffer_t& {
        return _data;
      }

      void data(const buffer_t &n) {
        _data = n;
      }

      auto clone() const -> std::shared_ptr<table_interface> {
        return make_shared<in_memory_table>(_meta, _data);
      }
    };

    static buffer_t buffer_filter(const buffer_t &buf, const size_t field, const std::string& value, const bool whole, const bool neg) {
      if(buf.empty()) return buf;

      buffer_t ret;

      for(auto &&s : buf) {
        // assuming no overflow
        if(neg == ((s[field].find(value) == string::npos) || (whole && s[field] != value)))
          ret.emplace_back(std::move(s));
      }

      return ret;
    }
  }

  // for permanent tables
  table::table(const string &name)
    : _t(new intern::permanent_table(name)) { }

  // for in-memory tables
  table::table(const metadata &meta)
    : _t(new intern::in_memory_table(meta)) { }

  table::table(const metadata &meta, const buffer_t &n)
    : _t(new intern::in_memory_table(meta, n)) { }

  table::table(shared_ptr<table_interface> &&o)
    : _t(std::move(o)) { }

  void table::swap(table &o) noexcept {
    std::swap(_t, o._t);
  }

  bool table::good() const noexcept {
    return _t->good();
  }

  bool table::empty() const noexcept {
    return _t->data().empty();
  }

  auto table::get_metadata() const noexcept -> const metadata& {
    return _t->get_metadata();
  }

  auto table::get_const_table() const noexcept -> const table& {
    return *this;
  }

  auto table::data() const noexcept -> const buffer_t& {
    return _t->data();
  }

  void table::data(const buffer_t &n) {
    // copy on write
    if(n != _t->data()) {
      if(!_t.unique()) _t = _t->clone();
      _t->data(n);
    }
  }

  auto table::clone() const -> std::shared_ptr<table_interface> {
    return _t->clone();
  }

  auto table::filter(const size_t field, const std::string& value, const bool whole, const bool neg) -> context {
    return context(*this, intern::buffer_filter(data(), field, value, whole, neg));
  }

  auto table::filter(const std::string& field, const std::string& value, const bool whole, const bool neg) -> context {
    return filter(get_metadata().get_field_nr(field), value, whole, neg);
  }

  auto table::filter(const size_t field, const std::string& value, const bool whole, const bool neg) const -> const_context {
    return const_context(*this, intern::buffer_filter(data(), field, value, whole, neg));
  }

  auto table::filter(const std::string& field, const std::string& value, const bool whole, const bool neg) const -> const_context {
    return filter(get_metadata().get_field_nr(field), value, whole, neg);
  }
}
