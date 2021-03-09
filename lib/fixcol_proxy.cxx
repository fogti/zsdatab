/**********************************************
 *   class: zsdatab::intern::*fixcol_proxy*
 * library: zsdatable
 * package: zsdatab
 * SPDX-License-Identifier: LGPL-2.1-or-later
 **********************************************/

#include "zsdatable.hpp"
#define ZSDA_PAR
#include <config.h>

#include <algorithm>

using namespace std;

namespace zsdatab {
  namespace intern {
    fixcol_proxy_common::fixcol_proxy_common(const buffer_interface &uplink, const string &field)
      : _nr(uplink.get_metadata().get_field_nr(field)) { }

    auto fixcol_proxy_common::get(const bool _uniq) const -> vector<string> {
      vector<string> ret;
      ret.reserve(_underlying_data().size());
      for(const auto &i : _underlying_data())
        ret.emplace_back(i[_nr]);

      if(_uniq && !ret.empty()) {
        auto ie = ret.end();
        sort(ZSDAM_PAR ret.begin(), ie);
        ret.erase(unique(ZSDAM_PAR ret.begin(), ie), ie);
      }

      return ret;
    }

    fixcol_proxy::fixcol_proxy(context_common &uplink, const size_t nr)
      : fixcol_proxy_common(nr), _uplink(uplink) { }

    fixcol_proxy::fixcol_proxy(context_common &uplink, const string &field)
      : fixcol_proxy_common(uplink, field), _uplink(uplink) { }

    auto fixcol_proxy::_underlying_data() const -> const buffer_t&
      { return _uplink.data(); }

    const_fixcol_proxy::const_fixcol_proxy(const buffer_interface &uplink, const size_t nr)
      : fixcol_proxy_common(nr), _uplink(uplink) { }

    const_fixcol_proxy::const_fixcol_proxy(const buffer_interface &uplink, const string &field)
      : fixcol_proxy_common(uplink, field), _uplink(uplink) { }

    const_fixcol_proxy::const_fixcol_proxy(const fixcol_proxy &o)
      : const_fixcol_proxy(o._uplink, o._nr) { }

    // change

    fixcol_proxy& fixcol_proxy::set(const string &value) {
      for(auto &l : _uplink._buffer) l[_nr] = value;
      return *this;
    }

    fixcol_proxy& fixcol_proxy::append(const string &value) {
      for(auto &l : _uplink._buffer) l[_nr] += value;
      return *this;
    }

    fixcol_proxy& fixcol_proxy::remove(const string &value) {
      for(auto &l : _uplink._buffer) {
        const auto pos = l[_nr].find(value);
        if(pos != string::npos) l[_nr].erase(pos, value.length());
      }
      return *this;
    }

    fixcol_proxy& fixcol_proxy::replace(const string& from, const string& to) {
      if(from.empty()) return *this;

      for_each(ZSDAC_PAR _uplink._buffer.begin(), _uplink._buffer.end(), [this, &from, &to](auto &l) noexcept {
        size_t sp = 0;
        while((sp = l[_nr].find(from, sp)) != string::npos) {
          l[_nr].replace(sp, from.length(), to);
          sp += to.length();
        }
      });
      return *this;
    }
  }
}
