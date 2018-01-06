/*************************************************
 *        class: zsdatab::intern::*fixcol_proxy*
 *      library: zsdatable
 *      package: zsdatab
 *      version: 0.2.9
 **************| *********************************
 *       author: Erik Kai Alain Zscheile
 *        email: erik.zscheile.ytrizja@gmail.com
 **************| *********************************
 * organisation: Ytrizja
 *     org unit: Zscheile IT
 *     location: Chemnitz, Saxony
 *************************************************
 *
 * Copyright (c) 2018 Erik Kai Alain Zscheile
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

#include <algorithm>
#include "zsdatable.hpp"

using namespace std;

namespace zsdatab {
namespace intern {
  fixcol_proxy_common::fixcol_proxy_common(const size_t nr)
    : _nr(nr) { }

  auto fixcol_proxy_common::get(const bool _uniq) const -> vector<string> {
    vector<string> ret;
    ret.reserve(_underlying_data().size());
    for(const auto &i : _underlying_data())
      ret.emplace_back(i[_nr]);

    if(_uniq && !ret.empty()) {
      auto ie = ret.end();
      sort(ret.begin(), ie);
      ret.erase(unique(ret.begin(), ie), ie);
    }

    return ret;
  }

  fixcol_proxy::fixcol_proxy(context_common &uplink, const size_t nr)
    : fixcol_proxy_common(_nr), _uplink(uplink) { }

  fixcol_proxy::fixcol_proxy(context_common &uplink, const string field)
    : fixcol_proxy(uplink, uplink.get_metadata().get_field_nr(field)) { }

  auto fixcol_proxy::_underlying_data() const -> const buffer_t& {
    return _uplink.data();
  }

  const_fixcol_proxy::const_fixcol_proxy(const buffer_interface &uplink, const size_t nr)
    : fixcol_proxy_common(_nr), _uplink(uplink) { }

  const_fixcol_proxy::const_fixcol_proxy(const buffer_interface &uplink, const string field)
    : const_fixcol_proxy(uplink, uplink.get_metadata().get_field_nr(field)) { }

  const_fixcol_proxy::const_fixcol_proxy(const fixcol_proxy &o)
    : const_fixcol_proxy(o._uplink, o._nr) { }

  auto const_fixcol_proxy::_underlying_data() const -> const buffer_t& {
    return _uplink.data();
  }

  // change

  void fixcol_proxy::set(const string &value) {
    for(auto &l : _uplink._buffer) l[_nr] = value;
  }

  void fixcol_proxy::append(const string &value) {
    for(auto &l : _uplink._buffer) l[_nr] += value;
  }

  void fixcol_proxy::remove(const string &value) {
    for(auto &l : _uplink._buffer) {
      const auto pos = l[_nr].find(value);
      if(pos != string::npos) l[_nr].erase(pos, value.length());
    }
  }

  void fixcol_proxy::replace(const string& from, const string& to) {
    if(from.empty()) return;
    for(auto &l : _uplink._buffer) {
      size_t sp = 0;
      while((sp = l[_nr].find(from, sp)) != string::npos) {
        l[_nr].replace(sp, from.length(), to);
        sp += to.length();
      }
    }
  }
}
}
