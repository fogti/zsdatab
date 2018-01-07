/*************************************************
 *        class: zsdatab::intern::context_common
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
#include <stdexcept>
#include "zsdatable.hpp"

using namespace std;

namespace zsdatab {
  namespace intern {
    void op_table_compat_chk(const table& a, const table& b) {
      if(a.get_metadata() != b.get_metadata())
        throw invalid_argument(__PRETTY_FUNCTION__);
    }

    context_common::context_common(const buffer_interface &bif)
      : _buffer(bif.data()) { }

    context_common::context_common(const buffer_t &o)
      : _buffer(o) { }

    context_common::context_common(buffer_t &&o)
      : _buffer(std::move(o)) { }

    context_common& context_common::pull() {
      _buffer = get_const_table().data();
      return *this;
    }

    // select
    context_common& context_common::sort() {
      const size_t colcnt = get_metadata().get_field_count();
      std::sort(_buffer.begin(), _buffer.end(),
        [colcnt](const row_t &a, const row_t &b) noexcept {
          for(size_t i = 0; i < colcnt; ++i) {
            if(a[i] < b[i]) return true;
            else if(a[i] > b[i]) break;
          }
          return false;
        }
      );

      return *this;
    }

    context_common& context_common::uniq() {
      if(!empty()) {
        sort();
        _buffer.erase(
          std::unique(_buffer.begin(), _buffer.end()),
          _buffer.end()
        );
      }

      return *this;
    }

    context_common& context_common::negate() {
      if(empty())
        pull();
      else if(_buffer == get_const_table().data())
        clear();
      else {
        const buffer_t oldbuf = _buffer;
        pull();

        using namespace std;
        _buffer.erase(
          remove_if(_buffer.begin(), _buffer.end(), [&oldbuf](const row_t &arg) noexcept {
            return find(oldbuf.begin(), oldbuf.end(), arg) != oldbuf.end(); // assuming no overflow
          }),
          _buffer.end());
      }

      return *this;
    }

    context_common& context_common::filter(const size_t field, const string& value, const bool whole, const bool neg) {
      if(empty()) return *this;

      using namespace std;
      _buffer.erase(
        remove_if(_buffer.begin(), _buffer.end(),
          [field, &value, whole, neg](const row_t &s) noexcept {
            return neg != ((s[field].find(value) == string::npos) || (whole && s[field] != value)); // assuming no overflow
          }
        ),
        _buffer.end());

      return *this;
    }
  }
}
