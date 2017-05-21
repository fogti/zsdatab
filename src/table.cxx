/*************************************************
 *        class: zsdatab::table
 *      library: zsdatable
 *      package: zsdatab
 *      version: 0.1.6
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
    bool valid;
    bool modified;
    metadata meta;
    buffer_t data;
    std::string path;

    impl(): valid(false), modified(false) { }

    impl(const impl &o) = default;
    impl(impl &&o) = default;
    ~impl() noexcept = default;
  };

  // for permanent tables
  table::table(const string &name):
    _d(new table::impl())
  {
    bool &valid = _d->valid;
    metadata &meta = _d->meta;
    string &path = _d->path;

    {
      ifstream in((name + ".meta").c_str());
      if(!in) {
        valid = false;
      } else {
        in >> meta;
        valid = !meta.empty();
      }
    }
    if(valid) {
      string host;
      {
        char tmp[65];
        gethostname(tmp, 64);
        tmp[64] = 0;
        host = tmp;
      }

      path = name + ".#" + host + '.' + to_string(getpid());

      while(1) {
        if(!::link(name.c_str(), path.c_str())) {
          struct stat st;
          if(stat(name.c_str(), &st) == -1 || st.st_nlink != 2) {
            ::unlink(path.c_str());
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
    _d->valid = meta.good();
    _d->meta = meta;
  }

  table::table(const table &o):
    _d(new table::impl(*o._d)) { }

  table::table(table &&o) = default;

  table::~table() noexcept {
    write();
    if(!_d->path.empty())
      ::unlink(_d->path.c_str());
  }

  void table::swap(table &o) noexcept {
    std::swap(_d, o._d);
  }

  bool table::good() const noexcept {
    return _d->valid;
  }

  bool table::empty() const noexcept {
    return _d->data.empty();
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

    if(good() && _d->modified && !_d->path.empty()) {
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
      _d->modified = false;
    }
    return true;
  }

  auto table::get_metadata() const noexcept -> const metadata& {
    return _d->meta;
  }

  auto table::get_const_table() const noexcept -> const table& {
    return *this;
  }

  auto table::data() const noexcept -> const buffer_t& {
    return _d->data;
  }

  void table::data(const buffer_t &n) {
    if(_d->data != n) {
      _d->modified = true;
      _d->data = n;
    }
  }
}
