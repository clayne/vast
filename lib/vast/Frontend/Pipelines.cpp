// Copyright (c) 2023-present, Trail of Bits, Inc.

#include "vast/Frontend/Pipelines.hpp"

#include "vast/Dialect/HighLevel/Passes.hpp"
#include "vast/Conversion/Passes.hpp"

namespace vast::cc {

    namespace pipeline {

        // Generates almost AST like MLIR, without any conversions applied
        pipeline_step_ptr high_level() {
            return hl::pipeline::splice_trailing_scopes();
        }

        // Simplifies high level MLIR
        pipeline_step_ptr reduce_high_level() {
            return compose("reduce-hl",
                hl::pipeline::simplify
            );
        }

        // Generates MLIR with standard types
        pipeline_step_ptr standard_types() {
            return compose("standard-types",
                hl::pipeline::stdtypes
            ).depends_on(reduce_high_level);
        }

        pipeline_step_ptr abi() {
            return conv::pipeline::abi();
        }

        // Conversion to LLVM dialects
        pipeline_step_ptr llvm() {
            return conv::pipeline::to_llvm();
        }

        gap::generator< pipeline_step_ptr > codegen() {
            // TODO: pass further options to augment high level MLIR
            co_yield high_level();
        }

        // Defines a sequence of dialects and for each conversion dialect a
        // passes to be run. This allows to stop conversion at any point (i.e.,
        // specify some intermediate taarget dialect) but produce all dependend
        // steps before.
        using conversion_path = std::vector<
            std::tuple<
                target_dialect, /* target dialect */
                llvm::function_ref< bool(const vast_args &) >, /* constraint to run the step */
                std::vector< llvm::function_ref< pipeline_step_ptr() > > /* pipeline for the step */
             >
        >;

        static auto unconstrained = [] (const vast_args & /* vargs */) { return true; };

        static auto has_option = [] (string_ref option) {
            return [option] (const vast_args &vargs) { return vargs.has_option(option); };
        };

        conversion_path default_conversion_path = {
            { target_dialect::high_level, has_option(opt::simplify), { reduce_high_level } },
            { target_dialect::std , unconstrained, { standard_types } },
            { target_dialect::abi , unconstrained, { abi } },
            { target_dialect::llvm, unconstrained, { llvm } }
        };

        gap::generator< pipeline_step_ptr > conversion(
            pipeline_source src,
            target_dialect trg,
            const vast_args &vargs
        ) {
            // TODO: add support for custom conversion paths
            // deduced from source, target and vargs
            const auto path = default_conversion_path;

            for (const auto &[dialect, is_to_be_applied, step_passes] : path) {
                if (is_to_be_applied(vargs)) {
                    for (auto &step : step_passes) {
                        co_yield step();
                    }
                }

                if (trg == dialect) {
                    break;
                }
            }

            if (vargs.has_option(opt::canonicalize)) {
                co_yield conv::pipeline::canonicalize();
            }
        }

    } // namespace pipeline

    bool vast_pipeline::is_disabled(const pipeline_step_ptr &step) const {
        auto disable_step_option = opt::disable(step->name()).str();
        return vargs.has_option(disable_step_option);
    }

    void vast_pipeline::schedule(pipeline_step_ptr step) {
        if (is_disabled(step)) {
            VAST_PIPELINE_DEBUG("step is disabled: {0}", step->name());
            return;
        }

        for (auto &&dep : step->dependencies()) {
            schedule(std::move(dep));
        }

        step->schedule_on(*this);
    }

    std::unique_ptr< vast_pipeline > setup_pipeline(
        pipeline_source src,
        target_dialect trg,
        mcontext_t &mctx,
        const vast_args &vargs
    ) {
        auto passes = std::make_unique< vast_pipeline >(mctx, vargs);

        passes->enableIRPrinting(
            [](auto *, auto *) { return false; }, // before
            [](auto *, auto *) { return true; },  // after
            false,                                // module scope
            false,                                // after change
            true,                                 // after failure
            llvm::errs()
        );

        // generate high level MLIR in case of AST input
        if (pipeline_source::ast == src) {
            for (auto &&step : pipeline::codegen()) {
                passes->schedule(std::move(step));
            }
        }

        // Apply desired conversion to target dialect, if target is llvm or
        // binary/assembly. We perform entire conversion to llvm dialect. Vargs
        // can specify how we want to convert to llvm dialect and allows to turn
        // off optional pipelines.
        for (auto &&step : pipeline::conversion(src, trg, vargs)) {
            passes->schedule(std::move(step));
        }

        if (vargs.has_option(opt::print_pipeline)) {
            passes->dump();
        }

        if (vargs.has_option(opt::disable_multithreading) || vargs.has_option(opt::emit_crash_reproducer)) {
            mctx.disableMultithreading();
        }

        if (vargs.has_option(opt::emit_crash_reproducer)) {
            auto reproducer_path = vargs.get_option(opt::emit_crash_reproducer);
            VAST_CHECK(reproducer_path.has_value(), "expected path to reproducer");
            passes->enableCrashReproducerGeneration(reproducer_path.value(), true /* local reproducer */);
        }

        return passes;
    }

} // namespace vast::cc
