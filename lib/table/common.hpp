/*************************************************
 *       header: zsdatab::intern::*table*_common
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
 * Copyright (c) 2019 Erik Kai Alain Zscheile
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
#pragma once
#include "zsdatable.hpp"
namespace zsdatab {
  namespace intern {
    class table_impl_common : public table_interface {
     public:
      table_impl_common() = default;
      explicit table_impl_common(metadata o)
        : _meta(std::move(o)) { }
      table_impl_common(metadata m, buffer_t n)
        : _meta(std::move(m)), _data(std::move(n)) { }
      virtual ~table_impl_common() noexcept = default;

      auto get_metadata() const noexcept -> const metadata& final
        { return _meta; }
      auto get_const_table() const noexcept -> const table_interface& final
        { return *this; }
      auto data() const noexcept -> const buffer_t& final
        { return _data; }
      auto data_move_out() && -> buffer_t&& final
        { return std::move(_data); }
      void data(const buffer_t &n)
        { _data = n; }

     protected:
      metadata _meta;
      buffer_t _data;
    };

    class permanent_table_common : public table_impl_common {
     public:
      permanent_table_common();
      permanent_table_common(const std::string &name);
      ~permanent_table_common();

      bool good() const noexcept final
        { return _valid; }

      using table_impl_common::data;
      void data(const buffer_t &n) final;
      auto clone() const -> std::shared_ptr<table_interface> final;

     protected:
      bool _valid, _modified;
      std::string _path;
    };

    class table_ref_common : public table_interface {
     public:
      table_ref_common(const buffer_t &n): _data(n) { }
      virtual ~table_ref_common() noexcept = default;

      bool good() const noexcept final;

      auto get_const_table() const noexcept -> const table_interface& final
        { return *this; }
      auto data() const noexcept -> const buffer_t& final
        { return _data; }

      // this function will always throw a logic_error
      void data(const buffer_t &n) final;
      auto data_move_out() && -> buffer_t&& final;

     protected:
      const buffer_t &_data;
    };

    class table_ref final : public table_ref_common {
      const metadata &_meta;

     public:
      table_ref(const metadata &m, const buffer_t &n)
        : table_ref_common(n), _meta(m) { }

      auto get_metadata() const noexcept -> const metadata&
        { return _meta; }

      auto clone() const -> std::shared_ptr<table_interface>;
    };

    static inline table make_table_ref(const metadata &m, const buffer_t &n) {
      return table(std::make_shared<table_ref>(m, n));
    }

    class table_data_ref final : public table_ref_common {
      metadata _meta;

     public:
      table_data_ref(metadata m, const buffer_t &n)
        : table_ref_common(n), _meta(std::move(m)) { }

      virtual ~table_data_ref() noexcept = default;

      auto get_metadata() const noexcept -> const metadata&
        { return _meta; }

      auto clone() const -> std::shared_ptr<table_interface>;
    };

    static inline table make_table_data_ref(metadata m, const buffer_t &n) {
      return table(std::make_shared<table_data_ref>(std::move(m), n));
    }
  }
}
