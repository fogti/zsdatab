/*************************************************
 *      library: zsdatable
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

#ifndef ZSDATABLE_HPP
# define ZSDATABLE_HPP 1
# include <istream>
# include <ostream>
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

  // metadata class
  class metadata final {
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

    void swap(metadata &o) noexcept;

    bool good() const noexcept;
    bool empty() const noexcept;

    auto get_field_count() const -> size_t;
    auto get_field_nr(const std::string &colname) const -> size_t;
    auto get_field_name(const size_t n) const -> std::string;

    auto deserialize(const std::string &line) const -> std::vector<std::string>;
    auto serialize(std::vector<std::string> line) const -> std::string;
  };

  bool operator==(const metadata &a, const metadata &b);
  bool operator!=(const metadata &a, const metadata &b);

  std::ostream& operator<<(std::ostream& stream, const metadata& meta);
  std::istream& operator>>(std::istream& stream, metadata& meta);

  class table;

  // buffer_interface class:
  //  common interface for table, context and const_context
  class buffer_interface {
   public:
    virtual ~buffer_interface() noexcept = default;

    virtual bool good() const noexcept = 0;
    virtual bool empty() const noexcept = 0;
    virtual auto get_metadata() const noexcept -> const metadata& = 0;
    virtual auto get_data() const noexcept -> const buffer_t& = 0;
    virtual auto get_const_table() const noexcept -> const table& = 0;
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
    auto get_data() const noexcept -> const buffer_t&;
    auto get_const_table() const noexcept -> const table&;
    void update_data(const buffer_t &n);
  };

  std::ostream& operator<<(std::ostream& stream, const table& tab);
  std::istream& operator>>(std::istream& stream, table& tab);

  namespace intern {
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

      void pull();

      void clear() noexcept;
      bool good() const noexcept;
      bool empty() const noexcept;
      void append(std::vector<std::string> line);

      void sort();
      void uniq();
      void negate();
      bool select(const std::string& field, const std::string& value, const bool whole = true);

      // change
      bool set_field(const std::string& field, const std::string& value);
      bool append_part(const std::string& field, const std::string& value);
      bool remove_part(const std::string& field, const std::string& value);
      bool replace_part(const std::string& field, const std::string& from, const std::string& to);

      // report
      auto get_field(const std::string &colname) const -> std::vector<std::string>;

      // const getters
      auto get_metadata() const noexcept -> const metadata&;
      auto get_data() const noexcept -> const buffer_t&;
      auto get_field_nr(const std::string &colname) const -> size_t;

      // main delegation and abstraction
      virtual auto get_const_table() const noexcept -> const table& = 0;

     protected:
      buffer_t _buffer;

      void op_table_compat_chk(const table& a, const table& b) const;

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

      virtual auto get_const_table() const noexcept -> const table& final
        { return _table; }

     protected:
      T &_table;
    };

    template<class T, class V>
    context_base<T, V> operator+(const context_base<T, V> &a, const context_base<T, V> &b) {
      context_base<T, V> ret = a;
      return ret += b;
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
}
#endif
