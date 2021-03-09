/**********************************************
 *  object: zsdatab::intern::threadpool
 * library: zsdatable
 * package: zsdatab
 * SPDX-License-Identifier: LGPL-2.1-or-later
 **********************************************/

#include "pool.hpp"

#ifndef HAVE_CXXH_EXECUTION
namespace zsdatab {
  namespace intern {
    ThreadPool threadpool(std::thread::hardware_concurrency());
  }
}
#endif
