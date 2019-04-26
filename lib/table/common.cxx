/*************************************************
 *        class: zsdatab::intern::permanent_table_common
 *      library: zsdatable
 *      package: zsdatab
 **************| *********************************
 *       author: Erik Kai Alain Zscheile
 *        email: erik.zscheile.ytrizja@gmail.com
 **************| *********************************
 * organisation: Ytrizja
 *     org unit: Zscheile IT
 *     location: Chemnitz, Saxony
 *************************************************
 *
 * Copyright (c) 2019 Erik Kai Alain Zscheile
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

#include "table/common.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <sched.h>
#include <unistd.h>

#include <iostream>

using namespace std;

namespace zsdatab {
  namespace intern {
    permanent_table_common::permanent_table_common()
      : _valid(false), _modified(false) { }

    permanent_table_common::permanent_table_common(const string &name)
      : permanent_table_common()
    {
      _path = name + ".#";

      {
        char tmp[65];
        gethostname(tmp, 64);
        tmp[64] = 0;
        _path += tmp;
      }

      _path += '.' + to_string(getpid());

      struct stat st;
      bool fi = true;
      while(
                !::link(name.c_str(), _path.c_str())
             && (
                    stat(name.c_str(), &st) == -1
                 || st.st_nlink != 2
                )
           ) {
        ::unlink(_path.c_str());
        if(fi)
          cerr << "libzsdatable.so: WARNING: waiting for table lock on '" << name << "' ";
        fi = false;
        cerr << '.';
        sched_yield();
      }
      if(!fi) cerr << '\n';
    }

    permanent_table_common::~permanent_table_common() {
      if(!_path.empty())
        ::unlink(_path.c_str());
    }

    void permanent_table_common::data(const buffer_t &n) {
      _modified = true;
      _data = n;
    }

    auto permanent_table_common::clone() const -> std::shared_ptr<table_interface> {
      throw table_clone_error(__PRETTY_FUNCTION__);
    }

    bool table_ref_common::good() const noexcept {
      // check for matching column count
      return (_data.empty() || get_metadata().get_field_count() == _data.front().size());
    }

    void table_ref_common::data(const buffer_t &n) {
      throw logic_error(__PRETTY_FUNCTION__);
    }

    auto table_ref::clone() const -> std::shared_ptr<table_interface> {
      return make_shared<table_ref>(_meta, _data);
    }

    auto table_data_ref::clone() const -> std::shared_ptr<table_interface> {
      return make_shared<table_data_ref>(_meta, _data);
    }
  }
}
