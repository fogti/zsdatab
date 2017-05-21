/*************************************************
 *      library: zsdatable
 *      package: zsdatab
 *      version: 0.1.6
 **************| ********************************
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

#ifndef ZSDATABLE_HPP
# define ZSDATABLE_HPP 1
# include <istream>
# include <ostream>
# include <unordered_map>
# include <vector>
# include <string>

# include <type_traits>
# include <algorithm>
# include <utility>
# include <memory>

# include <experimental/propagate_const>

namespace zsdatab {
  // typedefs
  typedef std::vector<std::vector<std::string>> buffer_t;

  namespace intern {
    // type for "pointers to implementation"
    template<class T>
    using pimpl = std::experimental::propagate_const<std::unique_ptr<T>>;
  }

  class container_interface {
   public:
    virtual ~container_interface() noexcept = default;

    virtual bool good() const noexcept = 0;
    virtual bool empty() const noexcept = 0;
  };

  // metadata class
  class metadata final : public container_interface {
    struct impl;
    intern::pimpl<impl> _d;

    friend auto operator<<(std::ostream& stream, const metadata::impl& meta) -> std::ostream&;
    friend auto operator>>(std::istream& stream, metadata::impl& meta) -> std::istream&;

    friend bool operator==(const metadata &a, const metadata &b);
    friend auto operator<<(std::ostream& stream, const metadata& meta) -> std::ostream&;
    friend auto operator>>(std::istream& stream, metadata& meta) -> std::istream&;

   public:
    metadata();
    metadata(const metadata &o);
    metadata(metadata &&o);

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
    void separator(char sep) noexcept;
    char separator() const noexcept;

    auto deserialize(const std::string &line) const -> std::vector<std::string>;
    auto serialize(const std::vector<std::string> &line) const -> std::string;
  };

  bool operator==(const metadata &a, const metadata &b);
  bool operator!=(const metadata &a, const metadata &b);

  std::ostream& operator<<(std::ostream& stream, const metadata& meta);
  std::istream& operator>>(std::istream& stream, metadata& meta);

  class table;

  // buffer_interface class:
  //  common interface for table, context and const_context
  class buffer_interface : public container_interface {
   public:
    virtual ~buffer_interface() noexcept = default;

    virtual auto get_metadata() const noexcept -> const metadata& = 0;
    virtual auto get_const_table() const noexcept -> const table& = 0;

    virtual auto data() const noexcept -> const buffer_t& = 0;
  };

  // table class
  class table final : public buffer_interface {
    struct impl;
    intern::pimpl<impl> _d;

   public:
    // for permanent tables
    table(const std::string &_path);

    // for in-memory tables
    table(const metadata &_meta);

    table(const table &o);
    table(table &&o);

    virtual ~table() noexcept;

    void swap(table &o) noexcept;

    bool good() const noexcept;
    bool empty() const noexcept;

    bool read();
    bool write() noexcept;

    auto get_metadata() const noexcept -> const metadata&;
    auto get_const_table() const noexcept -> const table&;

    auto data() const noexcept -> const buffer_t&;
    void data(const buffer_t &n);
  };

  std::ostream& operator<<(std::ostream& stream, const table& tab);
  std::istream& operator>>(std::istream& stream, table& tab);

  namespace intern {
    // an internal checker for compatibility of tables
    // throws if not compatible
    void op_table_compat_chk(const table& a, const table& b);

    // abstract base class for contexts
    class context_common : public buffer_interface {
     public:
      context_common(const buffer_interface &bif);
      context_common(const context_common &ctx) = default;
      context_common(context_common &&ctx) noexcept = default;
      virtual ~context_common() noexcept = default;

      auto operator=(const context_common &o) -> context_common&;
      auto operator=(const buffer_interface &o) -> context_common&;
      auto operator=(context_common &&o) -> context_common&;
      auto operator+=(const context_common &o) -> context_common&;
      auto operator+=(const buffer_interface &o) -> context_common&;
      auto operator+=(const std::vector<std::string> &line) -> context_common&;

      context_common& pull();

      bool good() const noexcept;
      bool empty() const noexcept;

      // select
      context_common& clear() noexcept;
      context_common& sort();
      context_common& uniq();
      context_common& negate();
      context_common& filter(const size_t field, const std::string& value, const bool whole = true);
      context_common& filter(const std::string& field, const std::string& value, const bool whole = true);

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
      auto get_column_data(const size_t colnr, bool _uniq = false) const -> std::vector<std::string>;
      auto get_column_data(const std::string &colname, bool _uniq = false) const -> std::vector<std::string>;

      auto get_metadata() const noexcept -> const metadata&;
      auto get_field_nr(const std::string &colname) const -> size_t;

      auto data() const noexcept -> const buffer_t&;

      // main delegation and abstraction
      virtual auto get_const_table() const noexcept -> const table& = 0;

     protected:
      buffer_t _buffer;

     private:
      friend std::ostream& operator<<(std::ostream& stream, const context_common &ctx);
      friend std::istream& operator>>(std::istream& stream, context_common& ctx);
    };

    bool operator==(const context_common &a, const context_common &b) noexcept;
    bool operator!=(const context_common &a, const context_common &b) noexcept;
    std::ostream& operator<<(std::ostream& stream, const context_common &ctx);
    std::istream& operator>>(std::istream& stream, context_common& ctx);

    // minimal non-abstract template base class for contexts
    template<class T, class = typename std::enable_if<std::is_base_of<buffer_interface, T>::value>::type>
    class context_base : public context_common {
     public:
      explicit context_base(T &tab): context_common(tab), _table(tab) { }
      context_base(const T &&tab) = delete;
      virtual ~context_base() noexcept = default;

      virtual auto get_const_table() const noexcept -> const table& final
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

    template<class T>
    context_base<T> operator+(const context_base<T> &a, const context_base<T> &b) {
      return context_base<T>(a) += b;
    }
  }

  class context;

  class const_context final : public intern::context_base<const table> {
   public:
    explicit const_context(const table& tab);
    const_context(const buffer_interface &o);

    const_context(const table&& tab) = delete;
    const_context(const buffer_interface &&o) = delete;
    const_context(const const_context &o) = default;
    const_context(const_context &&o) noexcept = default;
    virtual ~const_context() noexcept = default;
  };

  class context final : public intern::context_base<table> {
    friend class const_context;

   public:
    explicit context(table& tab);
    context(const context &o) = default;
    context(context &&o) noexcept = default;
    virtual ~context() noexcept = default;

    // transfer
    void push();

    // rm = negate push
    // rmexcept = push
  };

  namespace intern {
    namespace ta {
      struct action;
    }
  }

  class transaction final {
   public:
    transaction(const metadata &m);
    transaction(const transaction &o);
    transaction(transaction&& o) = default;

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
   *         - buffer : ccomposed buffer
   *
   * @param sep : char : (metadata) column separator
   *
   * @param a, b : buffer_interface : buffers to join
   *             - metadata.cols (equal names are assumed equivalent and will be joined)
   */
  table inner_join(char sep, const buffer_interface &a, const buffer_interface &b);

  // table_map_fields - map field names (mappings: {from, to}) (e.g. for an following join)
  table table_map_fields(const buffer_interface &in, const std::unordered_map<std::string, std::string>& mappings);
}
#endif
