#ifndef MFREE_CONFIG_BUILD_FROM_CONFIG_H_
#define MFREE_CONFIG_BUILD_FROM_CONFIG_H_

#include "config/simulation_config.h"

class body;

namespace mfree::config {

body *build_body_from_config(const simulation_config &cfg);

}

#endif
