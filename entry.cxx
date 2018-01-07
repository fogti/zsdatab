/*************************************************
 *      program: zsdatab-entry
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

#include <ctype.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <deque>

#include "zsdatable.hpp"

using namespace std;

static string my_tolower(string instr) {
  transform(instr.begin(), instr.end(), instr.begin(), ::tolower);
  return instr;
}

static unsigned int xsel_gmatcht(const string &mat) {
  if(mat == "whole" || mat == "=") return 1;
  if(mat == "part"  || mat == "LIKE") return 2;
  return 0;
}

int main(int argc, char *argv[]) {
  if(argc < 2) {
    cerr << "USAGE: zsdatab-entry TABLE [CMD ARGS... ]...\n"
            "\n"
            "Commands:\n"
            "  select FIELD VALUE                  select all entries that match VALUE (deprecated)\n"
            "  xsel whole|part|LIKE|= FIELD VALUE  select all entries that match VALUE (whole field or partial)\n"
            "  neg                                 negate buffer\n"
            "  get FIELD                           get field FIELD and exit\n"
            "\n"
            "  ch FIELD NEWVALUE                   change FIELD to NEWVALUE\n"
            "  rmpart FIELD SUBSTRING              remove FIELD-part SUBSTRING\n"
            "  appart FIELD SUBSTRING              append FIELD-part SUBSTRING\n"
            "\n"
            "  new COLUMNS...                      create new entry\n"
            "  rm                                  remove selected entries (= negate push)\n"
            "  rmexcept                            remove everything except selected entries (= push)\n"
            "\n"
            "Other Commands:\n"
            "  quit                                quit without printing buffer\n"
            "  push                                update table file\n"
            "\n"
            "Commands can be joined\n\n"
            "zsdatab v0.2.8 by Erik Zscheile <erik.zscheile.ytrizja@gmail.com>\n"
            "released under X11-License\n";
    return 1;
  } else if(argc == 2) {
    string tmp;
    ifstream in(argv[1]);
    if(!in) {
      cerr << "zsdatab-entry: ERROR: " << argv[1] << ": file not found\n";
      return 1;
    }
    while(getline(in, tmp)) cout << tmp << "\n";
    return 0;
  }

  zsdatab::table my_table(argv[1]);
  if(!my_table.good()) {
    cerr << "zsdatab-entry: ERROR: " << argv[1] << ": file not found / read failed\n";
    return 1;
  }
  const size_t colcnt = my_table.get_metadata().get_field_count();

  zsdatab::context my_ctx(my_table);
  deque<string> commands(argv + 2, argv + argc);

  string cmd, field;

  // parse commands
  try {
    while(!commands.empty()) {
      cmd = my_tolower(commands.front());
      commands.pop_front();
      string selector;

      // check args
      {
        bool args_ok = true;
        if(cmd == "ch" || cmd == "select" || cmd == "appart" || cmd == "rmpart") {
          if(commands.size() < 2 || commands.front().empty()) args_ok = false;
          else field = commands[0];
          commands.pop_front();
        } else if(cmd == "xsel") {
          if(commands.size() < 3 || commands[0].empty() || commands[1].empty()) args_ok = false;
          else if(!xsel_gmatcht(commands[0])) args_ok = false;
          else field = commands[1];
          selector = commands[0];
          commands.pop_front();
          commands.pop_front();
        } else if(cmd == "new") {
          if(commands.size() < colcnt) args_ok = false;
        } else if(cmd == "get") {
          if(commands.empty() || commands.front().empty()) args_ok = false;
          else field = commands[0];
          commands.pop_front();
        } else if(cmd == "rm" || cmd == "rmexcept" || cmd == "neg" || cmd == "quit" || cmd == "push") {
          // do nothing
        } else {
          cerr << "zsdatab-entry: ERROR: unknown command '" << cmd << "'\n";
          return 1;
        }

        if(!args_ok) {
          cerr << "zsdatab-entry: ERROR: command " << cmd << ": invalid args\n";
          return 1;
        }
      }

      // command execution
      if(cmd == "ch") {
        zsdatab::context tmp_ctx = my_ctx;
        tmp_ctx.negate();

        my_ctx.set_field(field, commands[0]);
        commands.pop_front();

        tmp_ctx += my_ctx;
        tmp_ctx.push();
      } else if(cmd == "appart") {
        zsdatab::context tmp_ctx = my_ctx;
        tmp_ctx.negate();

        my_ctx.append_part(field, commands[0]);
        commands.pop_front();

        tmp_ctx += my_ctx;
        tmp_ctx.push();
      } else if(cmd == "rmpart") {
        zsdatab::context tmp_ctx = my_ctx;
        tmp_ctx.negate();

        my_ctx.remove_part(field, commands[0]);
        commands.pop_front();

        tmp_ctx += my_ctx;
        tmp_ctx.push();
      } else if(cmd == "select") {
        my_ctx.filter(field, commands[0]);
        commands.pop_front();
      } else if(cmd == "xsel") {
        my_ctx.filter(field, commands[0], xsel_gmatcht(selector) == 1);
        commands.pop_front();
      } else if(cmd == "new") {
        const auto cbi = commands.begin();
        const auto cei = cbi + colcnt;
        const vector<string> line(cbi, cei);
        commands.erase(cbi, cei);

        my_ctx.pull();
        my_ctx += line;
        my_ctx.push();
      } else if(cmd == "get") {
        for(auto &&l : my_ctx.get_column_data(field))
          cout << l << '\n';
        return 0;
      } else if(cmd == "rm") {
        my_ctx.negate();
        my_ctx.push();
      } else if(cmd == "rmexcept")
        my_ctx.push();
      else if(cmd == "neg")
        my_ctx.negate();
      else if(cmd == "push")
        my_ctx.push();
      else if(cmd == "quit")
        return 0;
    }
  } catch(const out_of_range&e) {
    cerr << "zsdatab-entry: ERROR: command " << cmd << ": unknown fieldname '" << field << "'\n";
    return 1;
  }

  // print buffer
  cout << my_ctx;
  return 0;
}
