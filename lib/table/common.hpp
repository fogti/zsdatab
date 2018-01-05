/*************************************************
 *       header: zsdatab::intern::*table*_common
 *      library: zsdatable
 *      package: zsdatab
 *      version: 0.2.8
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
#ifndef ZSDATAB_TABLE_COMMON_HPP
# define ZSDATAB_TABLE_COMMON_HPP 1
# include "zsdatable.hpp"
namespace zsdatab {
  namespace intern {
    class table_impl_common : public table_interface {
     public:
      table_impl_common() = default;
      explicit table_impl_common(const metadata &o);
      table_impl_common(const metadata &m, const buffer_t &n);
      virtual ~table_impl_common() noexcept = default;

      auto get_metadata() const noexcept -> const metadata&;
      auto data() const noexcept -> const buffer_t&;
      void data(const buffer_t &n);

     protected:
      metadata _meta;
      buffer_t _data;
    };

    class permanent_table_common : public table_impl_common {
     public:
      permanent_table_common();
      permanent_table_common(const std::string &name);
      ~permanent_table_common();

      bool good() const noexcept;
      using table_impl_common::data;
      void data(const buffer_t &n);
      auto clone() const -> std::shared_ptr<table_interface>;

     protected:
      bool _valid, _modified;
      std::string _path;
    };
  }
}
#endif /* ZSDATAB_TABLE_COMMON_HPP */
