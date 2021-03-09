/**********************************************
 *    part: table mapping
 * library: zsdatable
 * package: zsdatab
 * SPDX-License-Identifier: LGPL-2.1-or-later
 **********************************************/

#include "zsdatable.hpp"
#include "table/common.hpp"

namespace zsdatab {
  using namespace std;

  table table_map_fields(const buffer_interface &in, unordered_map<string, string> mappings) {
    const auto &mo = in.get_metadata();
    row_t tmp;
    tmp.reserve(mo.get_field_count());

    for(const auto &i : mo.get_cols()) {
      const auto it = mappings.find(i);
      tmp.emplace_back(it != mappings.end() ? string(move(it->second)) : string(i));
    }

    return intern::make_table_data_ref(metadata(mo.separator(), move(tmp)), in.data());
  }
}
