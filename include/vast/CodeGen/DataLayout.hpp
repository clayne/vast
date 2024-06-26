// Copyright (c) 2021-present, Trail of Bits, Inc.

#pragma once

#include "vast/Util/Common.hpp"
#include "vast/Util/DataLayout.hpp"

namespace vast::cg
{
    void emit_data_layout(mcontext_t &ctx, owning_module_ref &mod, const dl::DataLayoutBlueprint &dl);

} // namespace vast::cg
