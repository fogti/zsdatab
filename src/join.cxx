/*************************************************
 *         part: table joining
 *      library: zsdatable
 *      package: zsdatab
 *      version: 0.2.6
 **************| *********************************
 *       author: Erik Kai Alain Zscheile
 *        email: erik.zscheile.ytrizja@gmail.com
 **************| *********************************
 * organisation: Ytrizja
 *     org unit: Zscheile IT
 *     location: Chemnitz, Saxony
 *************************************************
 *
 * Copyright (c) 2017 Erik Kai Alain Zscheile
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

zsdatab::table zsdatab::inner_join(char sep, const buffer_interface &a, const buffer_interface &b) {
  struct column_join {
    size_t a, b, c;
  };

  enum column_join_from_where {
    FROM_COMMON, FROM_A, FROM_B
  };

  const metadata &ma = a.get_metadata();
  const metadata &mb = b.get_metadata();
  metadata mt(sep);

  using namespace std;
  // compute column names
  unordered_map<string, column_join> merge_cols;
  unordered_map<string, column_join_from_where> join_cols;
  {
    vector<string> cols = ma.get_cols();
    for(auto &&i : mb.get_cols()) {
      if(find(cols.begin(), cols.end(), i) != cols.end())
        merge_cols[i] = { 0, 0, 0 };
      else
        cols.push_back(i);
    }
    mt += cols;

    for(auto &&i : cols) {
#define CJFW(V) column_join_from_where::FROM_##V
      join_cols[i] = (
        (merge_cols.find(i) != merge_cols.end())
        ? CJFW(COMMON)
        : (ma.has_field(i) ? CJFW(A) : CJFW(B))
      );
#undef CJFW
    }
  }

  for(auto &i : merge_cols) {
    auto &fi = i.first;
    auto &se = i.second;
    se.a = ma.get_field_nr(fi);
    se.b = mb.get_field_nr(fi);
    se.c = mt.get_field_nr(fi);
  }

  // compute table
  vector<vector<string>> table_data;

  for(auto &&x : a.data()) {
    for(auto &&y : b.data()) {
      vector<string> line(mt.get_field_count());
      bool match = true;
      for(auto &&col : merge_cols) {
        const string value = x[col.second.a];
        if(value == y[col.second.b]) {
          line[col.second.c] = value;
        } else {
          match = false;
          break;
        }
      }
      if(!match) continue;

      for(size_t i = 0; i < line.size(); ++i) {
        auto &l = line[i];
        if(!l.empty()) continue;

        const auto colname = mt.get_field_name(i);
        switch(join_cols[colname]) {
          case column_join_from_where::FROM_A:
            l = x[ma.get_field_nr(colname)];
            break;

          case column_join_from_where::FROM_B:
            l = y[mb.get_field_nr(colname)];
            break;

          default: break;
        }
      }
      table_data.push_back(line);
    }
  }

  return table(mt, table_data);
}
