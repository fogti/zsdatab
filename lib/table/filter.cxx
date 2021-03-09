/**********************************************
 *   class: zsdatab::table::filter
 * library: zsdatable
 * package: zsdatab
 * SPDX-License-Identifier: LGPL-2.1-or-later
 **********************************************/

#include "zsdatable.hpp"

#define ZSDA_PAR
#include <config.h>
#include "pool.hpp"

#ifdef HAVE_CXXH_EXECUTION
# include <algorithm>
# include <iterator>
#endif

using namespace std;

namespace zsdatab {
  static buffer_t buffer_filter(const buffer_t &buf, const size_t field, const string& value, const bool whole, const bool neg) {
    // IMPORTANT NOTE: breaks if chklambda is declared static
    const auto chklambda = [field, &value, whole, neg](const row_t &i) noexcept {
      const auto &s = i[field];
      return neg == ((s.find(value) == string::npos) || (whole && s != value));
    };

    if(buf.empty()) return {};

#ifndef HAVE_CXXH_EXECUTION
    vector<future<bool>> futs;
    futs.reserve(buf.size());

    for(const auto &i : buf)
      futs.emplace_back(intern::threadpool.enqueue(chklambda, i));
#endif

    buffer_t ret;
    ret.reserve(buf.size());

#ifdef HAVE_CXXH_EXECUTION
    copy_if(ZSDAM_PAR buf.begin(), buf.end(), back_inserter(ret), chklambda);
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
