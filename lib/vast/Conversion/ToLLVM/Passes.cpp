// Copyright (c) 2023-present, Trail of Bits, Inc.

#include "vast/Util/Warnings.hpp"

VAST_RELAX_WARNINGS
#include <mlir/Pass/Pass.h>
#include <mlir/Pass/PassManager.h>
VAST_UNRELAX_WARNINGS

#include "vast/Conversion/Passes.hpp"

namespace vast::conv::pipeline {

    pipeline_step_ptr irs_to_llvm() {
        return top_level_pass(createIRsToLLVMPass).depends_on(abi);
    }

} // namespace vast::conv::pipeline
