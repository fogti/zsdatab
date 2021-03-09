/**********************************************
 *    part: table joining
 * library: zsdatable
 * package: zsdatab
 * SPDX-License-Identifier: LGPL-2.1-or-later
 **********************************************/

#include "zsdatable.hpp"
#include <algorithm>

zsdatab::table zsdatab::inner_join(const char sep, const buffer_interface &a, const buffer_interface &b) {
  enum column_join_from_where {
    FROM_COMMON, FROM_A, FROM_B
  };

  struct column_join {
    column_join_from_where src;
    size_t a, b;
  };

  const metadata &ma = a.get_metadata();
  const metadata &mb = b.get_metadata();
  metadata mt(sep);

  using namespace std;
  // compute column names
  unordered_map<string, column_join> join_cols;
  {
    const auto &mac = ma.get_cols();
    vector<string> cols = mac, merge_cols;
    for(auto &&i : mb.get_cols()) {
      ((find(mac.begin(), mac.end(), i) != mac.end())
        ? &merge_cols : &cols)->emplace_back(move(i));
    }
    mt += cols;

    for(const auto &i : cols) {
      auto &se = join_cols[i];
      bool na = true, nb = true;
#define CJFW(V) column_join_from_where::FROM_##V
      if(find(merge_cols.begin(), merge_cols.end(), i) != merge_cols.end()) {
        se.src = CJFW(COMMON);
      } else if(ma.has_field(i)) {
        se.src = CJFW(A);
        nb = false;
      } else {
        se.src = CJFW(B);
        na = false;
      }
#undef CJFW
      se.a = na ? ma.get_field_nr(i) : 0;
      se.b = nb ? mb.get_field_nr(i) : 0;
    }
  }

  // compute table
  vector<vector<string>> table_data;

  for(const auto &x : a.data())
    for(const auto &y : b.data()) {
      vector<string> line(mt.get_field_count());
      bool match = true;

      for(size_t i = 0; i < line.size(); ++i) {
        auto &l = line[i];
        const auto &se = join_cols[mt.get_field_name(i)];

        switch(se.src) {
          case column_join_from_where::FROM_A:
            l = x[se.a]; break;

          case column_join_from_where::FROM_B:
            l = y[se.b]; break;

          default:
            l = x[se.a];
            match = (l == y[se.b]);
            break;
        }

        if(!match) break;
      }
      if(match) table_data.emplace_back(move(line));
    }

  return table(move(mt), move(table_data));
}
