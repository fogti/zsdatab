/*************************************************
 *        class: zsdatab::intern::context_common
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

#include "zsdatable.hpp"
#include <algorithm>
#include <stdexcept>

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

    context_common& context_common::set_field(const size_t field, const string& value) {
      get_fixcol_proxy(field).set(value);
      return *this;
    }

    context_common& context_common::set_field(const string& field, const string& value) {
      return set_field(get_field_nr(field), value);
    }

    context_common& context_common::append_part(const size_t field, const string& value) {
      get_fixcol_proxy(field).append(value);
      return *this;
    }

    context_common& context_common::append_part(const string& field, const string& value) {
      return append_part(get_field_nr(field), value);
    }

    context_common& context_common::remove_part(const size_t field, const string& value) {
      get_fixcol_proxy(field).remove(value);
      return *this;
    }

    context_common& context_common::remove_part(const string& field, const string& value) {
      return remove_part(get_field_nr(field), value);
    }

    context_common& context_common::replace_part(const size_t field, const string& from, const string& to) {
      get_fixcol_proxy(field).replace(from, to);
      return *this;
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

    auto context_common::get_field_nr(const string &colname) const -> size_t {
      return get_metadata().get_field_nr(colname);
    }

    vector<string> context_common::get_column_data(const size_t field, const bool _uniq) const {
      return const_fixcol_proxy(*this, field).get(_uniq);
    }

    fixcol_proxy context_common::get_fixcol_proxy(const size_t field) {
      return {*this, field};
    }

    // operators
    static void op_table_compat_chk(const buffer_interface& a, const buffer_interface& b) {
      if(a.get_metadata() != b.get_metadata())
        throw invalid_argument(__PRETTY_FUNCTION__);
    }

    context_common& context_common::operator=(const context_common &o) {
      return *this = static_cast<const buffer_interface&>(o);
    }

    context_common& context_common::operator=(const buffer_interface &o) {
      if(this != &o) {
        op_table_compat_chk(*this, o);
        _buffer = o.data();
      }
      return *this;
    }

    context_common& context_common::operator=(context_common &&o) {
      op_table_compat_chk(*this, o);
      swap(_buffer, o._buffer);
      return *this;
    }

    context_common& context_common::operator+=(const buffer_interface &o) {
      if(this != &o) {
        op_table_compat_chk(*this, o);
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
      if(&a == &b) return true;
      const auto &am = a.get_metadata(), &bm = b.get_metadata();
      return (&am == &bm || am.get_cols() == bm.get_cols())
          && (a.data() == b.data());
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

  // dunno where to put this one
  const_context::const_context(const context &o)
    : context_base<const table>(o._table, o._buffer) { }
}
