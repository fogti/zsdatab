/*************************************************
 *      program: zsdatab-entry
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

#include <ctype.h>
#include <stdlib.h>
//#include <unistd.h>
#include <iostream>
#include <fstream>

#include <deque>

#include "zsdatable.hpp"

using namespace std;

__attribute__((noreturn)) static void zerror(const string &s) {
  cerr << "zsdatab-entry: ERROR: " << s << "\n";
  exit(1);
}

static void zwarn(const string &s) {
  clog << "zsdatab-entry: WARNING: " << s << "\n";
}

__attribute__((noreturn)) static void fieldname_err(const string &cmd, const string &field) {
  zerror("command " + cmd + ": unknown fieldname '" + field + "'");
}

static string my_tolower(string instr) {
  transform(instr.begin(), instr.end(), instr.begin(), ::tolower);
  return instr;
}

static unsigned int xsel_gmatcht(const string &matcht) {
  if(matcht == "whole" || matcht == "=") return 1;
  if(matcht == "part"  || matcht == "LIKE") return 2;
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
            "zsdatab v0.0.9 by Erik Zscheile <erik.zscheile.ytrizja@gmail.com>\n"
            "released under X11-License\n";
    return 1;
  } else if(argc == 2) {
    string tmp;
    ifstream in(argv[1]);
    if(!in) zerror(string(argv[1]) + ": file not found");
    while(getline(in, tmp)) cout << tmp << "\n";
    return 0;
  }

  zsdatab::table my_table(argv[1]);
  if(!my_table.good()) zerror(string(argv[1]) + ": metadata read failed");
  if(!my_table.read()) zerror(string(argv[1]) + ": file not found");
  const size_t colcnt = my_table.get_metadata().get_field_count();

  zsdatab::context my_ctx(my_table);
  deque<string> commands(argv + 2, argv + argc);

  // parse commands
  while(!commands.empty()) {
    const string cmd = my_tolower(commands.front());
    commands.pop_front();

    // check args
    {
      bool args_ok = true;
      if(cmd == "ch" || cmd == "select" || cmd == "appart" || cmd == "rmpart") {
        if(commands.size() < 2 || commands.front().empty()) args_ok = false;
      } else if(cmd == "xsel") {
        if(commands.size() < 3 || commands[0].empty() || commands[1].empty()) args_ok = false;
        else if(!xsel_gmatcht(commands[0])) args_ok = false;
      } else if(cmd == "new") {
        if(commands.size() < colcnt) args_ok = false;
      } else if(cmd == "get") {
        if(commands.empty() || commands.front().empty()) args_ok = false;
      } else if(cmd == "rm" || cmd == "rmexcept" || cmd == "neg" || cmd == "quit" || cmd == "push") {
        // do nothing
      } else zerror("unknown command '" + cmd + "'");

      if(!args_ok) zerror("command " + cmd + ": invalid args");
    }

    // command execution
    if(cmd == "ch") {
      zsdatab::context tmp_ctx = my_ctx;
      tmp_ctx.negate();

      if(!my_ctx.set_field(commands[0], commands[1]))
        fieldname_err(cmd, commands[0]);

      commands.pop_front();
      commands.pop_front();

      tmp_ctx += my_ctx;
      tmp_ctx.push();
    } else if(cmd == "appart") {
      zsdatab::context tmp_ctx = my_ctx;
      tmp_ctx.negate();

      if(!my_ctx.append_part(commands[0], commands[1]))
        fieldname_err(cmd, commands[0]);

      commands.pop_front();
      commands.pop_front();

      tmp_ctx += my_ctx;
      tmp_ctx.push();
    } else if(cmd == "rmpart") {
      zsdatab::context tmp_ctx = my_ctx;
      tmp_ctx.negate();

      if(!my_ctx.remove_part(commands[0], commands[1]))
        fieldname_err(cmd, commands[0]);

      commands.pop_front();
      commands.pop_front();

      zsdatab::context tmp2_ctx(my_table);
      tmp2_ctx = my_ctx + tmp_ctx;
      tmp2_ctx.push();

      //tmp_ctx += my_ctx;
      //tmp_ctx.push();
    } else if(cmd == "select") {
      if(!my_ctx.select(commands[0], commands[1])) {
        // keep the old behavoir
        if(!my_ctx.empty())
          fieldname_err(cmd, commands[0]);
      }
      commands.pop_front();
      commands.pop_front();
    } else if(cmd == "xsel") {
      if(!my_ctx.select(commands[1], commands[2], xsel_gmatcht(commands[0]) == 1)) {
        // keep the old behavoir
        if(!my_ctx.empty())
          fieldname_err(cmd, commands[1]);
      }
      commands.pop_front();
      commands.pop_front();
      commands.pop_front();
    } else if(cmd == "new") {
      vector<string> line;
      line.reserve(colcnt);
      for(size_t fieldn = 0; fieldn < colcnt; ++fieldn) {
        line.push_back(commands[0]);
        commands.pop_front();
      }

      zsdatab::context crctx(my_table);
      crctx.append(line);
      crctx.push();
      my_ctx.pull();
    } else if(cmd == "get") {
      for(auto &&l : my_ctx.get_field(commands[0]))
        cout << l << '\n';
      return 0;
    } else if(cmd == "rm") {
      my_ctx.negate();
      my_ctx.push();
    } else if(cmd == "rmexcept") {
      my_ctx.push();
    } else if(cmd == "neg") {
      my_ctx.negate();
    } else if(cmd == "push") {
      my_ctx.push();
    } else if(cmd == "quit") return 0;
  }

  // print buffer
  cout << my_ctx;
  return 0;
}
