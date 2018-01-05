/*************************************************
 *        class: zsdatab::intern::context_common
 *      library: zsdatable
 *      package: zsdatab
 *      version: 0.2.8
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

#include <stdexcept>
#include "zsdatable.hpp"

using namespace std;

namespace zsdatab {
  namespace intern {
    // select
    context_common& context_common::clear() noexcept {
      _buffer.clear();
      return *this;
    }

    context_common& context_common::filter(const string& field, const string& value, const bool whole, const bool neg) {
      return filter(get_field_nr(field), value, whole, neg);
    }

    context_common& context_common::set_field(const string& field, const string& value) {
      return set_field(get_field_nr(field), value);
    }

    context_common& context_common::append_part(const string& field, const string& value) {
      return append_part(get_field_nr(field), value);
    }

    context_common& context_common::remove_part(const string& field, const string& value) {
      return remove_part(get_field_nr(field), value);
    }

    context_common& context_common::replace_part(const string& field, const string& from, const string& to) {
      return replace_part(get_field_nr(field), from, to);
    }

    // report
    vector<string> context_common::get_column_data(const string &colname, const bool _uniq) const {
      try {
        return get_column_data(get_field_nr(colname), _uniq);
      } catch(...) {
        return {};
      }
    }

    auto context_common::get_metadata() const noexcept -> const metadata& {
      return get_const_table().get_metadata();
    }

    auto context_common::get_field_nr(const string &colname) const -> size_t {
      return get_metadata().get_field_nr(colname);
    }

    auto context_common::data() const noexcept -> const buffer_t& {
      return _buffer;
    }
  }
}
