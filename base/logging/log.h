/**
 * Copyright (c) 2023.07.10
 * Author: MengXii
 */

#pragma once

#include <glog/logging.h>

#include "fmt/format.h"

namespace base::logging {

#define LLOG(module, lvl, ...) \
  LOG(lvl) << "[" #module "] " << fmt::format(__VA_ARGS__)

}  // namespace base::logging