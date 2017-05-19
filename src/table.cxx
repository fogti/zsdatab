/*************************************************
 *        class: zsdatab::table
 *      library: zsdatable
 *      package: zsdatab
 *      version: 0.1.5
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

#include "zsdatable.hpp"

using namespace std;

namespace zsdatab {
  class table::impl final {
   public:
    std::string path;

    bool _valid;
    bool _modified;
    metadata _meta;
    buffer_t _data;
    std::string _lock_path;

    impl(): _valid(false), _modified(false) { }

    impl(const impl &o) = default;
    impl(impl &&o) = default;
    ~impl() noexcept = default;
  };

  // for permanent tables
  table::table(const string &name):
    _d(new table::impl())
  {
    string &path = _d->path = name;

    bool &_valid = _d->_valid;
    metadata &_meta = _d->_meta;
    string &_lock_path = _d->_lock_path;

    {
      ifstream in((path + ".meta").c_str());
      if(!in) {
        _valid = false;
      } else {
        in >> _meta;
        _valid = !_meta.empty();
      }
    }
    if(_valid) {
      string host;
      {
        char tmp[65];
        gethostname(tmp, 64);
        tmp[64] = 0;
        host = tmp;
      }

      _lock_path = path + ".#" + host + '.' + to_string(getpid());

      while(1) {
        if(!::link(path.c_str(), _lock_path.c_str())) {
          struct stat st;
          if(stat(path.c_str(), &st) == -1 || st.st_nlink != 2) {
            ::unlink(_lock_path.c_str());
            sleep(1);
          } else {
            break;
          }
        }
      }
    }
  }

  // for in-memory tables
  table::table(const metadata &meta):
    _d(new table::impl())
  {
    _d->_valid = meta.good();
    _d->_meta = meta;
  }

  table::table(const table &o):
    _d(new table::impl(*o._d)) { }

  table::table(table &&o) = default;

  table::~table() noexcept {
    write();
    if(!_d->_lock_path.empty())
      ::unlink(_d->_lock_path.c_str());
  }

  void table::swap(table &o) noexcept {
    std::swap(_d, o._d);
  }

  bool table::good() const noexcept {
    return _d->_valid;
  }

  bool table::empty() const noexcept {
    return _d->_data.empty();
  }

  bool table::read() {
    if(!good() || _d->path.empty()) return false;
    ifstream in(_d->path.c_str());
    if(!in) return false;
    in >> *this;
    return true;
  }

  bool table::write() noexcept {
    static const string fetpf = "libzsdatable.so: ERROR: zsdatab::table::write() failed ";

    // we must do carefully error handling here because
    // the destructor can't release the lock on the table if we don't do this

    if(good() && _d->_modified && !_d->path.empty()) {
      try {
        ofstream out(_d->path.c_str());
        if(!out) {
          cerr << fetpf << "(table open failed)\n";
          return false;
        }
        out << *this;
      } catch(const length_error &e) {
        cerr << fetpf << "(corrupt table data)\n"
                "  failure detected in: " << e.what() << '\n';
        return false;
      } catch(const exception &e) {
        cerr << fetpf << "(unknown error)\n"
                "  failure detected in: " << e.what() << '\n';
        return false;
      } catch(...) {
        cerr << fetpf << "(unknown error - untraceable)\n";
        return false;
      }
      _d->_modified = false;
    }
    return true;
  }

  auto table::get_metadata() const noexcept -> const metadata& {
    return _d->_meta;
  }

  auto table::get_data() const noexcept -> const buffer_t& {
    return _d->_data;
  }

  auto table::get_const_table() const noexcept -> const table& {
    return *this;
  }

  void table::update_data(const buffer_t &n) {
    if(_d->_data != n) {
      _d->_modified = true;
      _d->_data = n;
      // we assume that users don't push very often
      _d->_data.shrink_to_fit();
    }
  }
}
