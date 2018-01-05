/*************************************************
 *      library: zsdatable
 *      package: zsdatab
 *      version: 0.2.8
 **************| ********************************
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

#ifndef ZSDATABLE_HPP
# define ZSDATABLE_HPP 1
# include <istream>
# include <ostream>
# include <string>
# include <memory>
# include <unordered_map>
# include <vector>
# include <utility>

# include <experimental/propagate_const>

namespace zsdatab {
  // typedefs
  typedef std::vector<std::vector<std::string>> buffer_t;

  namespace intern {
    // type for "pointers to implementation"
    template<class T>
    using pimpl = std::experimental::propagate_const<std::unique_ptr<T>>;

    template<class T>
    struct swapable {
      virtual void swap(T &o) noexcept = 0;
    };

    namespace ta {
      // base class for transaction parts
      struct action;
    }
  }

  // metadata class
  class metadata final : public intern::swapable<metadata> {
    struct impl;
    intern::pimpl<impl> _d;

    friend auto operator<<(std::ostream &stream, const metadata::impl &meta) -> std::ostream&;
    friend auto operator>>(std::istream &stream, metadata::impl &meta) -> std::istream&;

    friend bool operator==(const metadata &a, const metadata &b);
    friend auto operator<<(std::ostream &stream, const metadata &meta) -> std::ostream&;
    friend auto operator>>(std::istream &stream, metadata &meta) -> std::istream&;

   public:
    metadata();
    metadata(const char sep);
    metadata(const metadata &o);
    metadata(metadata &&o) = default;

    ~metadata() noexcept;

    auto operator=(const metadata &o) -> metadata&;
    auto operator+=(const std::vector<std::string> &o) -> metadata&;

    void swap(metadata &o) noexcept;

    bool good() const noexcept;
    bool empty() const noexcept;

    auto get_cols() const noexcept -> const std::vector<std::string>&;
    auto get_field_count() const -> size_t;
    bool has_field(const std::string &colname) const noexcept;
    auto get_field_nr(const std::string &colname) const -> size_t;
    auto get_field_name(const size_t n) const -> std::string;
    bool rename_field(const std::string &from, const std::string &to);

    // simple setters and getters
    void separator(const char sep) noexcept;
    char separator() const noexcept;

    auto deserialize(const std::string &line) const -> std::vector<std::string>;
    auto serialize(const std::vector<std::string> &line) const -> std::string;
  };

  bool operator==(const metadata &a, const metadata &b);
  bool operator!=(const metadata &a, const metadata &b);

  std::ostream& operator<<(std::ostream &stream, const metadata &meta);
  std::istream& operator>>(std::istream &stream, metadata &meta);

  class table;

  // buffer_interface class:
  //  common interface for table, context and const_context
  //  provides an interface to a buffer and the associated metadata
  struct buffer_interface {
    virtual ~buffer_interface() noexcept = default;

    virtual auto get_metadata() const noexcept -> const metadata& = 0;
    virtual auto get_const_table() const noexcept -> const table& = 0;

    virtual auto data() const noexcept -> const buffer_t& = 0;
    bool empty() const noexcept;
  };

  struct table_clone_error : public std::runtime_error {
    using runtime_error::runtime_error;
  };

  // table_interface class:
  //  common interface for tables
  struct table_interface {
    virtual ~table_interface() noexcept = default;

    virtual bool good() const noexcept = 0;

    virtual auto get_metadata() const noexcept -> const metadata& = 0;
    virtual auto data() const noexcept -> const buffer_t& = 0;
    virtual void data(const buffer_t &n) = 0;

    virtual auto clone() const -> std::shared_ptr<table_interface> = 0;
  };

  class const_context;
  class context;

  // table (delegating) class
  class table final : public buffer_interface, public table_interface, public intern::swapable<table> {
    std::shared_ptr<table_interface> _t;

   public:
    // for permanent tables
    table(const std::string &_path);

    // for in-memory tables
    table(const metadata &_meta);
    table(const metadata &_meta, const buffer_t &n);

    table(std::shared_ptr<table_interface> &&o);
    table(const table &o) = default;
    table(table &&o) = default;
    virtual ~table() noexcept = default;

    void swap(table &o) noexcept;

    bool good() const noexcept;
    auto get_metadata() const noexcept -> const metadata&;
    auto get_const_table() const noexcept -> const table&;

    auto data() const noexcept -> const buffer_t&;
    void data(const buffer_t &n);

    auto clone() const -> std::shared_ptr<table_interface>;

    auto filter(const size_t field, const std::string& value, const bool whole = true, const bool neg = false) -> context;
    auto filter(const std::string& field, const std::string& value, const bool whole = true, const bool neg = false) -> context;
    auto filter(const size_t field, const std::string& value, const bool whole = true, const bool neg = false) const -> const_context;
    auto filter(const std::string& field, const std::string& value, const bool whole = true, const bool neg = false) const -> const_context;
  };

  std::ostream& operator<<(std::ostream& stream, const table& tab);
  std::istream& operator>>(std::istream& stream, table& tab);

  // for permanent tables, metadata and main data in one file
  bool create_packed_table(const std::string &_path, const metadata &_meta);
  table make_packed_table(const std::string &_path);

  // for permanent tables, gzipped and packed
  bool create_gzipped_table(const std::string &_path, const metadata &_meta);
  table make_gzipped_table(const std::string &_path);

  namespace intern {
    // an internal checker for compatibility of tables
    // throws if not compatible
    void op_table_compat_chk(const table& a, const table& b);

    // abstract base class for contexts
    class context_common : public buffer_interface {
     public:
      context_common(const buffer_interface &bif);
      context_common(const buffer_t &o);
      context_common(buffer_t &&o);
      context_common(const context_common &ctx) = default;
      context_common(context_common &&ctx) noexcept = default;
      virtual ~context_common() noexcept = default;

      auto operator=(const context_common &o) -> context_common&;
      auto operator=(const buffer_interface &o) -> context_common&;
      auto operator=(context_common &&o) -> context_common&;
      auto operator+=(const buffer_interface &o) -> context_common&;
      auto operator+=(const std::vector<std::string> &line) -> context_common&;

      context_common& pull();

      // select
      context_common& clear() noexcept;
      context_common& sort();
      context_common& uniq();
      context_common& negate();
      context_common& filter(const size_t field, const std::string& value, const bool whole = true, const bool neg = false);
      context_common& filter(const std::string& field, const std::string& value, const bool whole = true, const bool neg = false);

      // change
      context_common& set_field(const size_t field, const std::string& value);
      context_common& append_part(const size_t field, const std::string& value);
      context_common& remove_part(const size_t field, const std::string& value);
      context_common& replace_part(const size_t field, const std::string& from, const std::string& to);

      context_common& set_field(const std::string& field, const std::string& value);
      context_common& append_part(const std::string& field, const std::string& value);
      context_common& remove_part(const std::string& field, const std::string& value);
      context_common& replace_part(const std::string& field, const std::string& from, const std::string& to);

      // report
      auto get_column_data(const size_t colnr, const bool _uniq = false) const -> std::vector<std::string>;
      auto get_column_data(const std::string &colname, const bool _uniq = false) const -> std::vector<std::string>;

      auto get_metadata() const noexcept -> const metadata&;
      auto get_field_nr(const std::string &colname) const -> size_t;

      auto data() const noexcept -> const buffer_t&;

      // main delegation and abstraction
      virtual auto get_const_table() const noexcept -> const table& = 0;

     protected:
      buffer_t _buffer;
    };

    bool operator==(const context_common &a, const context_common &b) noexcept;
    bool operator!=(const context_common &a, const context_common &b) noexcept;
    std::ostream& operator<<(std::ostream& stream, const context_common &ctx);
    std::istream& operator>>(std::istream& stream, context_common& ctx);

    // minimal non-abstract template base class for contexts
    template<class T>
    class context_base : public context_common {
     public:
      explicit context_base(T &tab): context_common(tab), _table(tab) { }

      context_base(T &tab, const buffer_t &o)
        : context_common(o), _table(tab) { }

      context_base(T &tab, buffer_t &&o)
        : context_common(std::move(o)), _table(tab) { }

      context_base(const T &&tab) = delete;
      context_base(const T &&tab, const buffer_t &&o) = delete;

      auto get_const_table() const noexcept -> const table& final
        { return _table; }

      // delegators
      template<class B>
      auto operator=(const B &o) -> context_base& {
        context_common::operator=(o);
        return *this;
      }

      template<class B>
      auto operator+=(const B &o) -> context_base& {
        context_common::operator+=(o);
        return *this;
      }

     protected:
      T &_table;
    };

    template<class Ta, class Tb>
    context_base<Ta> operator+(const context_base<Ta> &a, const context_base<Tb> &b) {
      return context_base<Ta>(a) += b;
    }

    template<class T>
    context_base<T> operator+(const context_base<const T> &a, const context_base<T> &b) {
      return (context_base<T>(b) = a) += b;
    }
  }

  class const_context final : public intern::context_base<const table> {
   public:
    using context_base<const table>::context_base;

    const_context(const context &o);
    const_context(const table &&tab) = delete;
    const_context(const buffer_interface &&o) = delete;
    const_context(const const_context &o) = default;
    const_context(const_context &&o) noexcept = default;
  };

  class context final : public intern::context_base<table> {
    friend class const_context;

   public:
    using context_base<table>::context_base;
    context(const context &o) = default;
    context(context &&o) noexcept = default;

    // transfer
    void push();

    // rm = negate push
    // rmexcept = push
  };

  class transaction final {
   public:
    transaction(const metadata &m);
    transaction(const transaction &o);
    transaction(transaction &&o) = default;

    void apply(intern::context_common &ctx) const;

    transaction& operator+=(const std::vector<std::string> &line);

    // select
    transaction& clear();
    transaction& sort();
    transaction& uniq();
    transaction& negate();
    transaction& filter(const std::string& field, const std::string& value, const bool whole = true);

    // change
    transaction& set_field(const std::string& field, const std::string& value);
    transaction& append_part(const std::string& field, const std::string& value);
    transaction& remove_part(const std::string& field, const std::string& value);
    transaction& replace_part(const std::string& field, const std::string& from, const std::string& to);

   private:
    const metadata _meta;
    std::vector<std::shared_ptr<intern::ta::action>> _actions;
  };

  /* inner_join - join to buffers into a table via inner join (common subset)
   * @return : table : composed table
   *         - buffer : composed buffer
   *
   * @param sep : char : (metadata) column separator
   *
   * @param a, b : buffer_interface : buffers to join
   *             - metadata.cols (equal names are assumed equivalent and will be joined)
   */
  table inner_join(const char sep, const buffer_interface &a, const buffer_interface &b);

  // table_map_fields - map field names (mappings: {from, to}) (e.g. for an following join)
  table table_map_fields(const buffer_interface &in, const std::unordered_map<std::string, std::string>& mappings);

  template<class T>
  void swap(intern::swapable<T> &a, intern::swapable<T> &b) { a.swap(b); }
}
#endif
