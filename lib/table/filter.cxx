/*************************************************
 *        class: zsdatab::table::filter
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

#include <ThreadPool.h>
#include <zsdatable.hpp>

#define ZSDA_PAR
#include <config.h>

#ifdef HAVE_CXXH_EXECUTION
# include <algorithm>
# include <iterator>
#endif

using namespace std;

namespace zsdatab {
#ifndef HAVE_CXXH_EXECUTION
  namespace intern {
    extern ThreadPool threadpool;
  }
#endif

  static buffer_t buffer_filter(const buffer_t &buf, const size_t field, const string& value, const bool whole, const bool neg) {
    static const auto chklambda = [&value, whole, neg](const auto &s) noexcept {
      return neg == ((s.find(value) == string::npos) || (whole && s != value));
    };

    if(buf.empty()) return {};

#ifndef HAVE_CXXH_EXECUTION
    vector<future<bool>> futs;
    futs.reserve(buf.size());

    for(const auto &i : buf)
      futs.emplace_back(intern::threadpool.enqueue(chklambda, i[field]));
#endif

    buffer_t ret;
    ret.reserve(buf.size());

#ifdef HAVE_CXXH_EXECUTION
    copy_if(ZSDAM_PAR buf.begin(), buf.end(), back_inserter(ret), [chklambda, field](const auto &s) noexcept {
      return chklambda(s[field]);
    });
#else
    size_t n = 0;
    for(auto &s : futs) {
      if(s.get()) ret.emplace_back(buf.at(n));
      ++n;
    }
#endif

    ret.shrink_to_fit();
    return ret;
  }

  auto table::filter(const size_t field, const std::string& value, const bool whole, const bool neg) -> context {
    return {*this, buffer_filter(data(), field, value, whole, neg)};
  }

  auto table::filter(const size_t field, const std::string& value, const bool whole, const bool neg) const -> const_context {
    return {*this, buffer_filter(data(), field, value, whole, neg)};
  }

  auto table::filter(const std::string& field, const std::string& value, const bool whole, const bool neg) -> context {
    return filter(get_metadata().get_field_nr(field), value, whole, neg);
  }

  auto table::filter(const std::string& field, const std::string& value, const bool whole, const bool neg) const -> const_context {
    return filter(get_metadata().get_field_nr(field), value, whole, neg);
  }
}
