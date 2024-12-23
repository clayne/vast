// Copyright (c) 2024, Trail of Bits, Inc.

#include "vast/CodeGen/DefaultAttrVisitor.hpp"

#include "vast/Dialect/HighLevel/HighLevelAttributes.hpp"

namespace vast::cg
{
    mlir_attr default_attr_visitor::VisitSectionAttr(const clang::SectionAttr *attr) {
        return make< hl::SectionAttr >(attr->getName());
    }

    mlir_attr default_attr_visitor::VisitTargetAttr(const clang::TargetAttr *attr) {
        return make< hl::TargetAttr >(attr->getFeaturesStr());
    }

    mlir_attr default_attr_visitor::VisitAliasAttr(const clang::AliasAttr *attr) {
        return make< hl::AliasAttr >(attr->getAliasee());
    }

    mlir_attr default_attr_visitor::VisitErrorAttr(const clang::ErrorAttr *attr) {
        return make< hl::ErrorAttr >(attr->getUserDiagnostic());
    }

    mlir_attr default_attr_visitor::VisitFlattenAttr(const clang::FlattenAttr *attr) {
        return make< hl::FlattenAttr >();
    }

    mlir_attr default_attr_visitor::VisitFormatAttr(const clang::FormatAttr *attr) {
        return make< hl::FormatAttr >(attr->getType()->getName());
    }

    mlir_attr default_attr_visitor::VisitFormatArgAttr(const clang::FormatArgAttr *attr) {
        return make< hl::FormatArgAttr >(attr->getFormatIdx().serialize());
    }

    mlir_attr default_attr_visitor::VisitAnnotateAttr(const clang::AnnotateAttr *attr) {
        return make< hl::AnnotationAttr >(attr->getAnnotation());
    }

    mlir_attr default_attr_visitor::VisitAlignedAttr(const clang::AlignedAttr *attr) {
        // TODO(#595): Keep the alignment in the attribute
        return make< hl::AlignedAttr >();
    }

    mlir_attr default_attr_visitor::VisitAlwaysInlineAttr(const clang::AlwaysInlineAttr *) {
        return make< hl::AlwaysInlineAttr >();
    }

    mlir_attr default_attr_visitor::VisitArtificialAttr(const clang::ArtificialAttr *attr) {
        return make< hl::ArtificialAttr >();
    }

    mlir_attr default_attr_visitor::VisitColdAttr(const clang::ColdAttr *attr) {
        return make< hl::ColdAttr >();
    }

    mlir_attr default_attr_visitor::VisitCommonAttr(const clang::CommonAttr *attr) {
        return make< hl::CommonAttr >();
    }

    mlir_attr default_attr_visitor::VisitConstAttr(const clang::ConstAttr *attr) {
        return make< hl::ConstAttr >();
    }

    mlir_attr default_attr_visitor::VisitConstInitAttr(const clang::ConstInitAttr *attr) {
        return make< hl::ConstInitAttr >();
    }

    mlir_attr default_attr_visitor::VisitConstructorAttr(const clang::ConstructorAttr *attr) {
        return make< hl::ConstructorAttr >(attr->getPriority());
    }

    mlir_attr default_attr_visitor::VisitLoaderUninitializedAttr(const clang::LoaderUninitializedAttr *) {
        return make< hl::LoaderUninitializedAttr >();
    }

    mlir_attr default_attr_visitor::VisitPackedAttr(const clang::PackedAttr *) {
        return make< hl::PackedAttr >();
    }

    mlir_attr default_attr_visitor::VisitPureAttr(const clang::PureAttr *) {
        return make< hl::PureAttr >();
    }

    mlir_attr default_attr_visitor::VisitSwiftAttrAttr(const clang::SwiftAttrAttr *attr) {
        return make< hl::SwiftAttrAttr >(attr->getAttribute());
    }

    mlir_attr default_attr_visitor::VisitWarnUnusedResultAttr(const clang::WarnUnusedResultAttr *) {
        return make< hl::WarnUnusedResultAttr >();
    }

    mlir_attr default_attr_visitor::VisitRestrictAttr(const clang::RestrictAttr *) {
        return make< hl::RestrictAttr >();
    }

    mlir_attr default_attr_visitor::VisitNoThrowAttr(const clang::NoThrowAttr *) {
        return make< hl::NoThrowAttr >();
    }

    mlir_attr default_attr_visitor::VisitOverloadableAttr(const clang::OverloadableAttr *attr) {
        return make< hl::OverloadableAttr >();
}

    mlir_attr default_attr_visitor::VisitNonNullAttr(const clang::NonNullAttr *) {
        return make< hl::NonNullAttr >();
    }

    mlir_attr default_attr_visitor::VisitPassObjectSizeAttr(const clang::PassObjectSizeAttr *attr) {
        return make< hl::PassObjectSizeAttr >(attr->getType(), attr->isDynamic());
    }

    mlir_attr default_attr_visitor::VisitModeAttr(const clang::ModeAttr *attr) {
        return make< hl::ModeAttr >(attr->getMode()->getName());
    }

    mlir_attr default_attr_visitor::VisitBuiltinAttr(const clang::BuiltinAttr *attr) {
        return make< hl::BuiltinAttr >(attr->getID());
    }

    mlir_attr default_attr_visitor::VisitAsmLabelAttr(const clang::AsmLabelAttr *attr) {
        return make< hl::AsmLabelAttr >(attr->getLabel(), attr->getIsLiteralLabel());
    }

    mlir_attr default_attr_visitor::VisitAllocAlignAttr(const clang::AllocAlignAttr *attr) {
        return make< hl::AllocAlignAttr >(attr->getParamIndex().getSourceIndex());
    }

    mlir_attr default_attr_visitor::VisitNoInstrumentFunctionAttr(const clang::NoInstrumentFunctionAttr *) {
        return make< hl::NoInstrumentFunctionAttr >();
    }

    mlir_attr default_attr_visitor::VisitNoProfileFunctionAttr(const clang::NoProfileFunctionAttr *) {
        return make< hl::NoProfileFunctionAttr >();
    }

    mlir_attr default_attr_visitor::VisitNotTailCalledAttr(const clang::NotTailCalledAttr *) {
        return make< hl::NotTailCalledAttr >();
    }

    mlir_attr default_attr_visitor::VisitAllocSizeAttr(const clang::AllocSizeAttr *attr) {
        auto num = attr->getNumElemsParam().isValid() ? attr->getNumElemsParam().getSourceIndex() : int();
        return make< hl::AllocSizeAttr >(attr->getElemSizeParam().getSourceIndex(), num);
    }

    mlir_attr default_attr_visitor::VisitEnableIfAttr(const clang::EnableIfAttr *attr) {
        return make< hl::EnableIfAttr >();
    }

    mlir_attr default_attr_visitor::VisitC11NoReturnAttr(const clang::C11NoReturnAttr *) {
        return make< hl::C11NoReturnAttr >();
    }

    mlir_attr default_attr_visitor::VisitCXX11NoReturnAttr(const clang::CXX11NoReturnAttr *) {
        return make< hl::CXX11NoReturnAttr >();
    }

    mlir_attr default_attr_visitor::VisitNonBlockingAttr(const clang::NonBlockingAttr *) {
        return make< hl::NonBlockingAttr >();
    }

    mlir_attr default_attr_visitor::VisitNonAllocatingAttr(const clang::NonAllocatingAttr *) {
        return make< hl::NonAllocatingAttr >();
    }

    mlir_attr default_attr_visitor::VisitBlockingAttr(const clang::BlockingAttr *) {
        return make< hl::BlockingAttr >();
    }

    mlir_attr default_attr_visitor::VisitAllocatingAttr(const clang::AllocatingAttr *) {
        return make< hl::AllocatingAttr >();
    }

    mlir_attr default_attr_visitor::VisitLeafAttr(const clang::LeafAttr *attr) {
        return make< hl::LeafAttr >();
    }

    mlir_attr default_attr_visitor::VisitDeprecatedAttr(const clang::DeprecatedAttr *attr) {
        return make< hl::DeprecatedAttr >(attr->getMessage(), attr->getReplacement());
    }

    mlir_attr default_attr_visitor::VisitDestructorAttr(const clang::DestructorAttr *attr) {
        return make< hl::DestructorAttr >(attr->getPriority());
    }

    mlir_attr default_attr_visitor::VisitTransparentUnionAttr(const clang::TransparentUnionAttr *attr) {
        return make< hl::TransparentUnionAttr >();
    }

    mlir_attr default_attr_visitor::VisitUnavailableAttr(const clang::UnavailableAttr *attr) {
        return make< hl::UnavailableAttr >(attr->getMessage());
    }

    mlir_attr default_attr_visitor::VisitDiagnoseIfAttr(const clang::DiagnoseIfAttr *attr) {
        return make< hl::DiagnoseIfAttr >();
    }

    mlir_attr default_attr_visitor::VisitReturnsTwiceAttr(const clang::ReturnsTwiceAttr *attr) {
        return make< hl::ReturnsTwiceAttr >();
    }

    mlir_attr default_attr_visitor::VisitDisableTailCallsAttr(const clang::DisableTailCallsAttr *attr) {
        return make< hl::DisableTailCallsAttr >();
    }

    mlir_attr default_attr_visitor::VisitNoAliasAttr(const clang::NoAliasAttr *attr) {
        return make< hl::NoAliasAttr >();
    }

    mlir_attr default_attr_visitor::VisitNoCommonAttr(const clang::NoCommonAttr *attr) {
        return make< hl::NoCommonAttr >();
    }

    mlir_attr default_attr_visitor::VisitNoDebugAttr(const clang::NoDebugAttr *attr) {
        return make< hl::NoDebugAttr >();
    }

    mlir_attr default_attr_visitor::VisitStandaloneDebugAttr(const clang::StandaloneDebugAttr *attr) {
        return make< hl::StandaloneDebugAttr >();
    }

    mlir_attr default_attr_visitor::VisitNoDuplicateAttr(const clang::NoDuplicateAttr *attr) {
        return make< hl::NoDuplicateAttr >();
    }

    mlir_attr default_attr_visitor::VisitConvergentAttr(const clang::ConvergentAttr *attr) {
        return make< hl::ConvergentAttr >();
    }

    mlir_attr default_attr_visitor::VisitNoInlineAttr(const clang::NoInlineAttr *attr) {
        return make< hl::NoInlineAttr >();
    }

    mlir_attr default_attr_visitor::VisitMaxFieldAlignmentAttr(const clang::MaxFieldAlignmentAttr *attr) {
        return make< hl::MaxFieldAlignmentAttr >(attr->getAlignment());
    }

    mlir_attr default_attr_visitor::VisitMayAliasAttr(const clang::MayAliasAttr *attr) {
        return make< hl::MayAliasAttr >();
    }

    mlir_attr default_attr_visitor::VisitUnusedAttr(const clang::UnusedAttr *attr) {
        return make< hl::UnusedAttr >();
    }

    mlir_attr default_attr_visitor::VisitUsedAttr(const clang::UsedAttr *attr) {
        return make< hl::UsedAttr >();
    }

    mlir_attr default_attr_visitor::VisitGNUInlineAttr(const clang::GNUInlineAttr *attr) {
        return make< hl::GNUInlineAttr >();
    }

    mlir_attr default_attr_visitor::VisitAnyX86NoCfCheckAttr(const clang::AnyX86NoCfCheckAttr *attr) {
        return make< hl::NoCfCheckAttr >();
    }

    mlir_attr default_attr_visitor::VisitAvailableOnlyInDefaultEvalMethodAttr(const clang::AvailableOnlyInDefaultEvalMethodAttr *attr) {
        return make< hl::AvailableOnlyInDefaultEvalMethodAttr >();
    }

    mlir_attr default_attr_visitor::VisitAvailabilityAttr(const clang::AvailabilityAttr *attr) {
        return make< hl::AvailabilityAttrAttr >();
    }

    mlir_attr default_attr_visitor::VisitFallThroughAttr(const clang::FallThroughAttr *attr) {
        return make< hl::FallthroughAttr >();
    }

    mlir_attr default_attr_visitor::VisitVisibilityAttr(const clang::VisibilityAttr *attr) {
        return make< hl::VisibilityAttr >(hl::Visibility(attr->getVisibility()));
    }

    mlir_attr default_attr_visitor::VisitAssumeAlignedAttr(const clang::AssumeAlignedAttr *attr) {
        auto alignment = attr->getAlignment()->getIntegerConstantExpr(actx);
        if (!alignment) {
            VAST_REPORT("could not evaluate assume aligned attribute alignment value.");
            return {};
        }
        if (auto offset_expr = attr->getOffset()) {
            auto offset = offset_expr->getIntegerConstantExpr(actx);
            if (!offset) {
                VAST_REPORT("could not evaluate assume aligned attribute offest value.");
                return {};
            }
            return make< hl::AssumeAlignedAttr >(alignment.value(), offset.value());
        }
        return make< hl::AssumeAlignedAttr >(alignment.value(), llvm::APInt());
    }

    mlir_attr default_attr_visitor::VisitCountedByAttr(const clang::CountedByAttr *attr) {
        return make< hl::CountedByAttr >();
    }

    mlir_attr default_attr_visitor::VisitCleanupAttr(const clang::CleanupAttr *attr) {
        return make< hl::CleanupAttr >(attr->getFunctionDecl()->getName());
    }

} // namespace vast::cg
