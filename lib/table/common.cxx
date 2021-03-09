/***************************************************
 *   class: zsdatab::intern::permanent_table_common
 * library: zsdatable
 * package: zsdatab
 * SPDX-License-Identifier: LGPL-2.1-or-later
 ***************************************************/

#include "table/common.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <thread>

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
        this_thread::yield();
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

    auto table_ref_common::data_move_out() && -> buffer_t&& {
      throw logic_error(__PRETTY_FUNCTION__);
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
