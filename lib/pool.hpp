/**********************************************
 *  object: zsdatab::intern::threadpool
 * library: zsdatable
 * package: zsdatab
 * SPDX-License-Identifier: LGPL-2.1-or-later
 **********************************************/

#include <config.h>

#ifndef HAVE_CXXH_EXECUTION
#include "3rdparty/ThreadPool/ThreadPool.hpp"

namespace zsdatab {
  namespace intern {
    extern ThreadPool threadpool;
  }
}
#endif
