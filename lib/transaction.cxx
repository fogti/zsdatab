/*************************************************
 *        class: zsdatab::transaction
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

#include "zsdatable.hpp"

using namespace std;

namespace zsdatab {
  namespace intern {
    namespace ta {
      enum action_name {
        NONE,        APPEND,
        CLEAR, SORT, UNIQ, NEGATE,
        FILTER,      SET_FIELD,
        APPEND_PART, REMOVE_PART,
        REPLACE_PART
      };

      struct action {
        virtual ~action() = default;
        virtual void apply(context_common &ctx) const = 0;
        virtual action_name get_name() const noexcept = 0;
      };

      struct akv : action {
        string value;
        size_t field;
      };

      struct append final : action {
        void apply(context_common &ctx) const { ctx += line;                       }
        action_name get_name() const noexcept { return action_name::APPEND;        }
        row_t line;
      };

      struct clear final : action {
        void apply(context_common &ctx) const { ctx.clear();                       }
        action_name get_name() const noexcept { return action_name::CLEAR;         }
      };

      struct sort final : action {
        void apply(context_common &ctx) const { ctx.sort();                        }
        action_name get_name() const noexcept { return action_name::SORT;          }
      };

      struct uniq final : action {
        void apply(context_common &ctx) const { ctx.uniq();                        }
        action_name get_name() const noexcept { return action_name::UNIQ;          }
      };

      struct negate final : action {
        void apply(context_common &ctx) const { ctx.negate();                      }
        action_name get_name() const noexcept { return action_name::NEGATE;        }
      };

      struct filter final : akv {
        void apply(context_common &ctx) const { ctx.filter(field, value, whole);   }
        action_name get_name() const noexcept { return action_name::FILTER;        }
        bool whole;
      };

      struct set_field final : akv {
        void apply(context_common &ctx) const { ctx.set_field(field, value);       }
        action_name get_name() const noexcept { return action_name::SET_FIELD;     }
      };

      struct append_part final : akv {
        void apply(context_common &ctx) const { ctx.append_part(field, value);     }
        action_name get_name() const noexcept { return action_name::APPEND_PART;   }
      };

      struct remove_part final : akv {
        void apply(context_common &ctx) const { ctx.remove_part(field, value);     }
        action_name get_name() const noexcept { return action_name::REMOVE_PART;   }
      };

      struct replace_part final : action {
        void apply(context_common &ctx) const { ctx.replace_part(field, from, to); }
        action_name get_name() const noexcept { return action_name::REPLACE_PART;  }

        string from, to;
        size_t field;
      };

      typedef vector<shared_ptr<action>> actions_t;
    }
  }

  transaction::transaction(const metadata &m): _meta(m) { }
  transaction::transaction(const transaction &o) = default;

  void transaction::apply(intern::context_common &ctx) const {
    if(_meta != ctx.get_metadata())
      throw invalid_argument(__PRETTY_FUNCTION__);

    for(auto &&i : _actions)
      i->apply(ctx);
  }

  // actions

  transaction& transaction::operator+=(const row_t &line) {
    if(line.size() != _meta.get_field_count())
      throw length_error(__PRETTY_FUNCTION__);

    auto p = new intern::ta::append;

    try {
      p->line = line;
      _actions.emplace_back(p);
      return *this;
    } catch(...) {
      delete p;
      throw;
    }
  }

  transaction& transaction::clear() {
    _actions.clear();
    _actions.emplace_back(new intern::ta::clear);
    _actions.shrink_to_fit();
    return *this;
  }

  using intern::ta::action_name;

  static action_name ta__get_lasta(const intern::ta::actions_t &actions) {
    if(actions.empty()) return action_name::NONE;
    return actions.back()->get_name();
  }

  transaction& transaction::sort() {
    switch(ta__get_lasta(_actions)) {
      case action_name::CLEAR:
      case action_name::SORT:
      case action_name::UNIQ:
        break;

      default:
        _actions.emplace_back(new intern::ta::sort);
    }
    return *this;
  }

  transaction& transaction::uniq() {
    switch(ta__get_lasta(_actions)) {
      case action_name::CLEAR:
      case action_name::UNIQ:
        break;

      case action_name::SORT:
        _actions.pop_back();

      default:
        _actions.emplace_back(new intern::ta::uniq);
    }
    return *this;
  }

  transaction& transaction::negate() {
    switch(ta__get_lasta(_actions)) {
      case action_name::NEGATE:
        _actions.pop_back();
        break;

      case action_name::SORT:
      case action_name::UNIQ:
        _actions.pop_back();

      default:
        _actions.emplace_back(new intern::ta::negate);
    }
    return *this;
  }

  transaction& transaction::filter(const string& field, const string& value, const bool whole) {
    auto p = make_shared<intern::ta::filter>();
    p->field = _meta.get_field_nr(field);
    p->value = value;
    p->whole = whole;

    switch(ta__get_lasta(_actions)) {
      case action_name::CLEAR:
        break;

      case action_name::SORT:
        _actions.back() = p;
        sort();
        break;

      case action_name::UNIQ:
        _actions.back() = p;
        uniq();
        break;

      default:
        _actions.emplace_back(p);
    }

    return *this;
  }

  transaction& transaction::set_field(const string& field, const string& value) {
    if(ta__get_lasta(_actions) == action_name::CLEAR) return *this;
    const size_t fnr = _meta.get_field_nr(field);
    auto p = new intern::ta::set_field;
    p->field = fnr;
    try {
      p->value = value;
    } catch(...) {
      delete p;
      throw;
    }

    if(ta__get_lasta(_actions) == action_name::SET_FIELD) {
      const auto old = dynamic_cast<const intern::ta::set_field *>(_actions.back().get());
      if(old && old->field == fnr)
        _actions.pop_back();
    }
    _actions.emplace_back(p);
    return *this;
  }

  transaction& transaction::append_part(const string& field, const string& value) {
    if(ta__get_lasta(_actions) == action_name::CLEAR) return *this;
    const size_t fnr = _meta.get_field_nr(field);
    auto p = new intern::ta::append_part;
    p->field = fnr;
    try {
      p->value = value;
    } catch(...) {
      delete p;
      throw;
    }

    if(ta__get_lasta(_actions) == action_name::APPEND_PART) {
      const auto old = dynamic_cast<const intern::ta::append_part *>(_actions.back().get());
      if(old && old->field == fnr && old->value != value) {
        p->value = old->value + value;
        _actions.pop_back();
      }
    }
    _actions.emplace_back(p);
    return *this;
  }

  transaction& transaction::remove_part(const string& field, const string& value) {
    if(ta__get_lasta(_actions) == action_name::CLEAR) return *this;
    const size_t fnr = _meta.get_field_nr(field);
    auto p = new intern::ta::append_part;
    p->field = fnr;
    try {
      p->value = value;
    } catch(...) {
      delete p;
      throw;
    }

    switch(ta__get_lasta(_actions)) {
      case action_name::APPEND_PART:
        {
          const auto old = dynamic_cast<const intern::ta::append_part *>(_actions.back().get());
          if(old && old->field == fnr && old->value == value)
            _actions.pop_back();
        }
        break;

      case action_name::REMOVE_PART:
        {
          const auto old = dynamic_cast<const intern::ta::remove_part *>(_actions.back().get());
          if(old && old->field == fnr && old->value == value)
            _actions.pop_back();
        }
        break;

      default:
        break;
    }
    _actions.emplace_back(p);
    return *this;
  }

  transaction& transaction::replace_part(const string& field, const string& from, const string& to) {
    if(ta__get_lasta(_actions) == action_name::CLEAR) return *this;
    const size_t fnr = _meta.get_field_nr(field);
    auto p = new intern::ta::replace_part;
    p->field = fnr;

    try {
      p->from = from;
      p->to = to;
      _actions.emplace_back(p);
      return *this;
    } catch(...) {
      delete p;
      throw;
    }
  }
}
