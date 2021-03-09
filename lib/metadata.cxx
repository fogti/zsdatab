/**********************************************
 *   class: zsdatab::metadata
 * library: zsdatable
 * package: zsdatab
 * SPDX-License-Identifier: LGPL-2.1-or-later
 **********************************************/

#include "zsdatable.hpp"
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <sstream>

using namespace std;

namespace zsdatab {
  [[gnu::hot]]
  static auto deserialize_line(const string &line, const char my_sep) {
    row_t ret;
    string col;
    istringstream ss(line);
    while(getline(ss, col, my_sep)) {
      bool escape = false;
      string col2;
      for(auto c : col) {
        if(escape) {
          escape = false;
          switch(c) {
            case '-': c = 0; break;
            case 'd': c = my_sep; break;
            case 'n': c = '\n'; break;
          }
          if(c) col2 += c;
        } else {
          // default mode
          if(c == '\\') escape = true;
          else col2 += c;
        }
      }
      ret.emplace_back(move(col2));
    }
    return ret;
  }

  [[gnu::hot]]
  static auto serialize_line(const row_t &cols, const char my_sep) {
    string ret;
    bool fi = true;

    for(const auto &i : cols) {
      if(!fi) ret += my_sep;
      fi = false;

      if(i.empty()) {
        ret += "\\-";
        continue;
      }

      for(auto c : i)
        switch(c) {
          case '\\': ret += "\\\\"; break;
          case '\n': ret += "\\n"; break;
          default:
            if(c == my_sep) ret += "\\d";
            else ret += c;
        }
    }

    return ret;
  }

  class metadata::impl final {
   public:
    row_t cols;
    char sep;

    impl(): sep(' ') { }

    void swap(impl &o) noexcept {
      std::swap(this->cols, o.cols);
      std::swap(this->sep, o.sep);
    }
  };

  metadata::metadata()
    : _d(make_unique<metadata::impl>()) { }

  metadata::metadata(const char sep, row_t cols)
    : metadata()
    { _d->sep = sep; _d->cols = move(cols); }

  metadata::metadata(const metadata &o)
    : _d(make_unique<metadata::impl>(*o._d)) { }

  metadata::metadata(metadata &&o)
    : _d(move(o._d)) { }

  metadata::~metadata() noexcept = default;

  metadata& metadata::operator=(const metadata &o) {
    metadata(o).swap(*this);
    return *this;
  }

  metadata& metadata::operator+=(const row_t &o) {
    _d->cols.insert(_d->cols.end(), o.begin(), o.end());
    return *this;
  }

  metadata& metadata::operator+=(row_t &&o) {
    auto &cls = _d->cols;
    cls.reserve(cls.size() + o.size());
    cls.insert(cls.end(), make_move_iterator(o.begin()), make_move_iterator(o.end()));
    return *this;
  }

  void metadata::swap(metadata &o) noexcept {
    std::swap(this->_d, o._d);
  }

  auto metadata::get_cols() const noexcept -> const row_t& {
    return _d->cols;
  }

  bool metadata::has_field(const string &colname) const noexcept {
    const auto &cols = get_cols();
    return find(cols.begin(), cols.end(), colname) != cols.end();
  }

  [[gnu::hot]]
  auto metadata::get_field_nr(const string &colname) const -> size_t {
    const auto &cols = get_cols();
    const auto it = find(cols.begin(), cols.end(), colname);
    if(it == cols.end())
      throw out_of_range(__PRETTY_FUNCTION__);
    return static_cast<size_t>(distance(cols.begin(), it));
  }

  bool metadata::rename_field(const string &from, const string &to) {
    auto &cols = _d->cols;
    auto it = find(cols.begin(), cols.end(), from);
    const bool ret = (it != cols.end());
    if(ret) *it = to;
    return ret;
  }

  void metadata::separator(const char sep) noexcept {
    _d->sep = sep;
  }

  char metadata::separator() const noexcept {
    return _d->sep;
  }

  auto metadata::deserialize(const string &line) const -> row_t {
    auto ret = deserialize_line(line, _d->sep);
    ret.resize(_d->cols.size());
    return ret;
  }

  auto metadata::serialize(const row_t &line) const -> string {
    if(line.size() != _d->cols.size())
      throw length_error(__PRETTY_FUNCTION__);
    return serialize_line(line, _d->sep);
  }

  auto operator<<(ostream &stream, const metadata::impl &meta) -> ostream& {
    stream << meta.sep << serialize_line(meta.cols, meta.sep) << '\n';
    return stream;
  }

  auto operator>>(istream &stream, metadata::impl &meta) -> istream& {
    stream.get(meta.sep);
    const bool old_layout = (stream.get() == '\n');
    if(!stream) return stream;

    string tmp;
    if(!old_layout) stream.unget();
    getline(stream, tmp);

    meta.cols = deserialize_line(tmp, old_layout ? ' ' : meta.sep);
    return stream;
  }

  auto operator<<(ostream &stream, const metadata &meta) -> ostream& {
    return (stream << *meta._d);
  }

  auto operator>>(istream &stream, metadata &meta) -> istream& {
    return (stream >> *meta._d);
  }

  bool operator==(const metadata &a, const metadata &b) {
    return a.get_cols().size() == b.get_cols().size();
  }
}
