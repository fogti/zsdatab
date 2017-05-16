/*************************************************
 *        class: zsdatab::intern::context_common
 *      library: zsdatable
 *      package: zsdatab
 *      version: 0.1.0
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

#include <stdexcept>
#include "zsdatable.hpp"

using namespace std;

namespace zsdatab {
  namespace intern {
    void op_table_compat_chk(const table& a, const table& b) {
      if(&a.get_metadata() != &b.get_metadata())
        throw invalid_argument("zsdatab::intern::op_table_compat_chk");
    }

    context_common::context_common(const buffer_interface &bif)
      : _buffer(bif.get_data()) { }

    context_common& context_common::operator=(const context_common &o) {
      return *this = static_cast<const buffer_interface&>(o);
    }

    context_common& context_common::operator=(const buffer_interface &o) {
      if(this != &o) {
        op_table_compat_chk(get_const_table(), o.get_const_table());
        _buffer = o.get_data();
      }
      return *this;
    }

    context_common& context_common::operator=(context_common &&o) {
      op_table_compat_chk(get_const_table(), o.get_const_table());
      swap(_buffer, o._buffer);
      return *this;
    }

    context_common& context_common::operator+=(const context_common &o) {
      return *this += static_cast<const buffer_interface&>(o);
    }

    context_common& context_common::operator+=(const buffer_interface &o) {
      if(this != &o) {
        op_table_compat_chk(get_const_table(), o.get_const_table());
        _buffer.reserve(_buffer.size() + o.get_data().size());
        _buffer.insert(_buffer.end(), o.get_data().begin(), o.get_data().end());
      } else {
        _buffer.reserve(_buffer.size() << 1);
        copy_n(_buffer.begin(), _buffer.size(), back_inserter(_buffer)); // assuming no overflow
      }
      return *this;
    }

    context_common& context_common::operator+=(vector<string> line) {
      if(line.size() != get_metadata().get_field_count())
        throw length_error(__PRETTY_FUNCTION__);
      _buffer.push_back(line);
      return *this;
    }

    context_common& context_common::pull() {
      _buffer = get_const_table().get_data();
      return *this;
    }


    bool context_common::good() const noexcept {
      return true;
    }

    bool context_common::empty() const noexcept {
      return _buffer.empty();
    }

    // select
    context_common& context_common::clear() noexcept {
      _buffer.clear();
      return *this;
    }

    context_common& context_common::sort() {
      std::sort(_buffer.begin(), _buffer.end(),
        [](const vector<string> &a, const vector<string> &b) -> bool {
          for(size_t i = 0; i < a.size(); ++i) {
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
      else if(_buffer == get_const_table().get_data())
        clear();
      else {
        const buffer_t oldbuf = _buffer;
        pull();

        using namespace std;
        _buffer.erase(
          remove_if(_buffer.begin(), _buffer.end(), [oldbuf](const vector<string> &arg) -> bool {
            return find(oldbuf.begin(), oldbuf.end(), arg) != oldbuf.end(); // assuming no overflow
          }),
          _buffer.end());
      }

      return *this;
    }

    context_common& context_common::filter(const string& field, const string& value, const bool whole) {
      if(empty()) return *this;

      size_t fieldn = get_field_nr(field);

      using namespace std;
      _buffer.erase(
        remove_if(_buffer.begin(), _buffer.end(),
          [value, whole, fieldn](const vector<string> &s) noexcept -> bool {
            return (s[fieldn].find(value) == string::npos) || (whole && s[fieldn] != value); // assuming no overflow
          }
        ),
        _buffer.end());

      return *this;
    }

    context_common& context_common::set_field(const string& field, const string& value) {
      size_t fieldn = get_field_nr(field);

      for(auto &l : _buffer)
        l[fieldn] = value;

      return *this;
    }

    context_common& context_common::append_part(const string& field, const string& value) {
      size_t fieldn = get_field_nr(field);

      for(auto &l : _buffer)
        l[fieldn] += value;

      return *this;
    }

    context_common& context_common::remove_part(const string& field, const string& value) {
      size_t fieldn = get_field_nr(field);

      for(auto &l : _buffer) {
        const string::size_type pos = l[fieldn].find(value);
        if(pos != string::npos) l[fieldn].erase(pos, value.length());
      }

      return *this;
    }

    context_common& context_common::replace_part(const string& field, const string& from, const string& to) {
      if(from.empty()) return *this;

      size_t fieldn = get_field_nr(field);

      for(auto &l : _buffer) {
        size_t start_pos = 0;
        while((start_pos = l[fieldn].find(from, start_pos)) != string::npos) {
          l[fieldn].replace(start_pos, from.length(), to);
          start_pos += to.length();
        }
      }

      return *this;
    }

    // report
    vector<string> context_common::get_column_data(const string &colname, bool _uniq) const {
      vector<string> ret;
      size_t fieldn;
      try {
        fieldn = get_field_nr(colname);
      } catch(...) {
        return ret;
      }

      for(auto &&i : _buffer)
        ret.push_back(i[fieldn]);

      if(!ret.empty() && _uniq) {
        std::sort(ret.begin(), ret.end());
        ret.erase(
          std::unique(ret.begin(), ret.end()),
          ret.end()
        );
      }

      return ret;
    }

    auto context_common::get_metadata() const noexcept -> const metadata& {
      return get_const_table().get_metadata();
    }

    auto context_common::get_data() const noexcept -> const buffer_t& {
      return _buffer;
    }

    auto context_common::get_field_nr(const string &colname) const -> size_t {
      return get_metadata().get_field_nr(colname);
    }

    bool operator==(const context_common &a, const context_common &b) noexcept {
      return (&a.get_metadata() == &b.get_metadata()) && (a.get_data() == b.get_data());
    }

    bool operator!=(const context_common &a, const context_common &b) noexcept {
      return !(a == b);
    }

    ostream& operator<<(ostream& stream, const context_common &ctx) {
      if(!stream) return stream;
      for(auto &&l : ctx._buffer)
        stream << ctx.get_metadata().serialize(l) << '\n';
      return stream;
    }

    istream& operator>>(istream& stream, context_common& ctx) {
      if(!stream) return stream;
      string tmp;
      while(getline(stream, tmp))
        ctx._buffer.push_back(ctx.get_metadata().deserialize(tmp));
      return stream;
    }
  }
}
