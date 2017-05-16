/*************************************************
 *        class: zsdatab::metadata
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
#include <sstream>
#include "zsdatable.hpp"

using namespace std;

static auto deserialize_line(const string &line, char my_sep) -> vector<string> {
  vector<string> ret;
  string col;
  istringstream ss(line);
  while(getline(ss, col, my_sep)) ret.push_back(col);
  return ret;
}

static auto serialize_line(const vector<string> &cols, char my_sep) -> string {
  string ret;
  bool fi = true;

  for(auto i : cols) {
    if(!fi) ret += my_sep;
    ret += i;
    fi = false;
  }

  return ret;
}

namespace zsdatab {
  class metadata::impl final {
   public:
    vector<string> cols;
    char sep;
    bool valid;

    impl(): sep(' '), valid(false) { }
    ~impl() noexcept = default;

    void swap(impl &o) noexcept {
      std::swap(this->cols, o.cols);
      std::swap(this->sep, o.sep);
      std::swap(this->valid, o.valid);
    }
  };

  metadata::metadata():
    _d(new metadata::impl()) { }

  metadata::metadata(const metadata &o):
    _d(new metadata::impl(*o._d)) { }

  metadata::metadata(metadata &&o) = default;

  metadata::~metadata() noexcept = default;

  metadata& metadata::operator=(const metadata &o) {
    metadata(o).swap(*this);
    return *this;
  }

  void metadata::swap(metadata &o) noexcept {
    std::swap(this->_d, o._d);
  }

  bool metadata::good() const noexcept {
    return _d->valid;
  }

  bool metadata::empty() const noexcept {
    return _d->cols.empty();
  }

  auto metadata::get_field_count() const -> size_t {
    return _d->cols.size();
  }

  auto metadata::get_field_nr(const string &colname) const -> size_t {
    const vector<string> &cols = _d->cols;
    const auto it = find(cols.begin(), cols.end(), colname);
    if(it == cols.end())
      throw out_of_range("zsdatab::metadata::get_field_nr");
    return static_cast<size_t>(distance(cols.begin(), it));
  }

  auto metadata::get_field_name(const size_t n) const -> string {
    return _d->cols.at(n);
  }

  auto metadata::deserialize(const string &line) const -> vector<string> {
    auto ret = deserialize_line(line, _d->sep);
    ret.resize(_d->cols.size());
    return ret;
  }

  auto metadata::serialize(const vector<string> &line) const -> string {
    if(line.size() != _d->cols.size())
      throw length_error(__PRETTY_FUNCTION__);
    return serialize_line(line, _d->sep);
  }

  bool operator==(const metadata &a, const metadata &b) {
    const auto &ad = *a._d;
    const auto &bd = *b._d;
    return (ad.valid == bd.valid) && (ad.sep == bd.sep) && (ad.cols == bd.cols);
  }

  auto operator<<(ostream& stream, const metadata::impl& meta) -> ostream& {
    stream << meta.sep << serialize_line(meta.cols, meta.sep) << '\n';
    return stream;
  }

  auto operator>>(istream& stream, metadata::impl& meta) -> istream& {
    string tmp;

    stream.get(meta.sep);
    const bool old_layout = (stream.get() == '\n');
    if(!stream) return stream;

    if(!old_layout) stream.unget();
    getline(stream, tmp);

    meta.cols = deserialize_line(tmp, old_layout ? ' ' : meta.sep);
    return stream;
  }

  auto operator<<(ostream& stream, const metadata& meta) -> ostream& {
    stream << *meta._d;
    return stream;
  }

  auto operator>>(istream& stream, metadata& meta) -> istream& {
    stream >> *meta._d;
    return stream;
  }
}
