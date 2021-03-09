/**********************************************
 *   class: zsdatab::intern::context_common
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
    context_common& context_common::pull() {
      _buffer = get_const_table().data();
      return *this;
    }

    // select
    context_common& context_common::sort() {
      const size_t colcnt = get_metadata().get_field_count();
      std::sort(ZSDAM_PAR _buffer.begin(), _buffer.end(),
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
          std::unique(ZSDAM_PAR _buffer.begin(), _buffer.end()),
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

        _buffer.erase(
          remove_if(ZSDAM_PAR _buffer.begin(), _buffer.end(), [&oldbuf](const row_t &arg) noexcept {
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
        remove_if(ZSDAM_PAR _buffer.begin(), _buffer.end(),
          [field, &value, whole, neg](const row_t &s) noexcept {
            return neg != ((s[field].find(value) == string::npos) || (whole && s[field] != value)); // assuming no overflow
          }
        ),
        _buffer.end());

      return *this;
    }
  }
}
