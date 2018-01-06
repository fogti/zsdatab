/*************************************************
 *         part: table joining
 *      library: zsdatable
 *      package: zsdatab
 *      version: 0.2.9
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

#include <algorithm>
#include "zsdatable.hpp"

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
            l = y[se.b];

          default: match = (l == y[se.a]);
        }

        if(!match) break;
      }
      if(match) table_data.emplace_back(move(line));
    }

  return table(mt, table_data);
}
