# zsdatab

## License

This library is licensed under `LGPL-2.1-or-later` (SPDX identifier).

## USAGE zsdatable

```
USAGE: zsdatable TABLE CMD ARGS...

Commands:

  mk  SEPERATOR COLUMNS...  make table
  mmk METAFILE              make table from metafile
  rm                        delete table
  chmod MODE                chmod table
  mv  DESTINATION           move table to DESTINATION
```

## USAGE zsdatab-entry

```
USAGE: zsdatab-entry TABLE [CMD ARGS... ]...

Commands:
  select FIELD VALUE                  select all entries that match VALUE (deprecated)
  xsel whole|part|LIKE|= FIELD VALUE  select all entries that match VALUE (whole field or partial)
  neg                                 negate buffer
  get FIELD                           get field FIELD and exit

  ch FIELD NEWVALUE                   change FIELD to NEWVALUE
  rmpart FIELD SUBSTRING              remove FIELD-part SUBSTRING
  appart FIELD SUBSTRING              append FIELD-part SUBSTRING

  new COLUMNS...                      create new entry
  rm                                  remove selected entries (= negate push)
  rmexcept                            remove everything except selected entries (= push)

Other Commands:
  quit                                quit without printing buffer
  push                                update table file

Commands can be joined
```

## USAGE libzsdatab

NOTE: all the following statements assume that you installed the libzsdatab library
and included the library header using:

```cpp
#include <zsdatable.hpp>
```

### metadata

```cpp
char sep = ':'; // The column separator
zsdatab::metadata md(sep);

// get the column names
const auto &cols = md.get_cols();

// get the field count
const size_t fcnt = md.get_field_count();

// detect if there is a column with a specific name
bool has_c = md.has_field("n");

// get the column id from name
// this function may throw an out_of_range exception if the name isn't found
size_t cid = md.get_field_nr("n");

// and vice-versa
auto cname = md.get_field_name(cid);

// rename a column
bool success = md.rename_field("n", "m");

// get the separator
sep = md.separator();

// set the separator
md.separator(sep);
```

### table

```cpp
zsdatab::table tab("amtab");
// or for in-memory table
zsdatab::table tab(metadata, initdata);

// is table empty
bool em = tab.empty();
// is table open or has valid metadata
bool gd = tab.good();

// get metadata
const auto &md = tab.get_metadata();

// get data
auto dat = tab.data();
```

### packed/gzipped table

NOTE: to use a gzipped packed table, simply replace ```packed``` with ```gzipped```.

```cpp
// setup the metadata for the table
zsdatab::metadata md(':');
md += { "a", "b", "c" };

// create packed table
zsdatab::create_packed_table("mood_packed", md);

// get a reference to the table
zsdatab::table tab = zsdatab::make_packed_table("mood_packed");

// work with the table
```

### context

```cpp
// assuming tab is a zsdatab::table
zsdatab::context ctx(tab);

// clear context buffer
ctx.clear();

// invert the buffer (put anything in ctx that is in tab but not in ctx currently)
ctx.negate();

// put current tab buffer into ctx buffer
ctx.pull();

// sort the buffer
ctx.sort();

// uniq the buffer
ctx.uniq();

// filter out anything that doesn't match
// the third param is the whole flag:
//  whole = true: match the whole value
//  whole = false: match the value partial (only a part of the field col must match)
ctx.filter("a", "match value", true);

// set a field
ctx.set_field("a", "new value");

// append to a field
ctx.append_part("a", "appended");

// remove from a field
ctx.remove_part("a", "appended");

// search-replace in a whole column
ctx.replace_part("a", "from", "to");

// put the context data into the table
ctx.push();
```

### fixcol proxy

```zsdatab::intern::fixcol_proxy``` is a column proxy to edit a column as a whole
(as replacement to the context set_field, append_part, remove_part, replace_part methods).

```cpp
// assuming ctx is a zsdatab::context
auto xcol = ctx.column("a");

// set all fields in this column
xcol.set("new value");

// append to all ...
xcol.append("appended");

// remove from all ...
xcol.remove("appended");

// search-replace
xcol.replace("from", "to");

// get all contents
std::vector<std::string> coldat = xcol.get();
```
