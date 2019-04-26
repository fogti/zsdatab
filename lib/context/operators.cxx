/*************************************************
 *        class: zsdatab::intern::context_common (operators)
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

#include <algorithm>
#include <stdexcept>
#include "zsdatable.hpp"

using namespace std;

namespace zsdatab {
  namespace intern {
    context_common& context_common::operator=(const context_common &o) {
      return *this = static_cast<const buffer_interface&>(o);
    }

    context_common& context_common::operator=(const buffer_interface &o) {
      if(this != &o) {
        op_table_compat_chk(get_const_table(), o.get_const_table());
        _buffer = o.data();
      }
      return *this;
    }

    context_common& context_common::operator=(context_common &&o) {
      op_table_compat_chk(get_const_table(), o.get_const_table());
      swap(_buffer, o._buffer);
      return *this;
    }

    context_common& context_common::operator+=(const buffer_interface &o) {
      if(this != &o) {
        op_table_compat_chk(get_const_table(), o.get_const_table());
        _buffer.reserve(_buffer.size() + o.data().size());
        _buffer.insert(_buffer.end(), o.data().begin(), o.data().end());
      } else {
        _buffer.reserve(_buffer.size() << 1);
        copy_n(_buffer.begin(), _buffer.size(), back_inserter(_buffer)); // assuming no overflow
      }
      return *this;
    }

    context_common& context_common::operator+=(const row_t &line) {
      if(line.size() != get_metadata().get_field_count())
        throw length_error(__PRETTY_FUNCTION__);
      _buffer.push_back(line);
      return *this;
    }

    auto context_common::column(const std::string field) -> fixcol_proxy {
      return {*this, field};
    }

    auto context_common::column(const std::string field) const -> const_fixcol_proxy {
      return {*this, field};
    }

    bool operator==(const context_common &a, const context_common &b) noexcept {
      return (&a.get_metadata() == &b.get_metadata()) && (a.data() == b.data());
    }

    ostream& operator<<(ostream& stream, const context_common &ctx) {
      if(!stream) return stream;
      auto &m = ctx.get_metadata();
      for(auto &l : ctx.data())
        stream << m.serialize(l) << '\n';
      return stream;
    }

    istream& operator>>(istream& stream, context_common& ctx) {
      if(!stream) return stream;
      const auto m = ctx.get_metadata();
      string tmp;
      while(getline(stream, tmp))
        ctx += m.deserialize(tmp);
      return stream;
    }
  }
}
