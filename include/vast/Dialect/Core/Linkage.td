// Copyright (c) 2022-present, Trail of Bits, Inc.

#ifndef VAST_DIALECT_CORE_IR_LINKAGE
#define VAST_DIALECT_CORE_IR_LINKAGE

include "mlir/IR/EnumAttr.td"

// Linkage types inspired from Clang IR - remove ones present in upstream MLIR

// Linkage types. This is currently a replay of llvm/IR/GlobalValue.h, this is
// currently handy as part of forwarding appropriate linkage types for LLVM
// lowering, specially useful for C++ support.

// Externally visible function
def Core_GlobalExternalLinkage :
  I32EnumAttrCase<"ExternalLinkage", 0, "external">;
// Available for inspection, not emission.
def Core_GlobalAvailableExternallyLinkage :
  I32EnumAttrCase<"AvailableExternallyLinkage", 1, "available_externally">;
// Keep one copy of function when linking (inline)
def Core_GlobalLinkOnceAnyLinkage :
  I32EnumAttrCase<"LinkOnceAnyLinkage", 2, "linkonce">;
// Same, but only replaced by something equivalent.
def Core_GlobalLinkOnceODRLinkage :
  I32EnumAttrCase<"LinkOnceODRLinkage", 3, "linkonce_odr">;
// Keep one copy of named function when linking (weak)
def Core_GlobalWeakAnyLinkage :
  I32EnumAttrCase<"WeakAnyLinkage", 4, "weak">;
// Same, but only replaced by something equivalent.
def Core_GlobalWeakODRLinkage :
  I32EnumAttrCase<"WeakODRLinkage", 5, "weak_odr">;
// Special purpose, only applies to global arrays
def Core_GlobalAppendingLinkage :
    I32EnumAttrCase<"AppendingLinkage", 6, "appending">;
// Rename collisions when linking (static functions).
def Core_GlobalInternalLinkage :
  I32EnumAttrCase<"InternalLinkage", 7, "internal">;
// Like Internal, but omit from symbol table.
def Core_GlobalPrivateLinkage :
  I32EnumAttrCase<"PrivateLinkage", 8, "private">;
// ExternalWeak linkage description.
def Core_GlobalExternalWeakLinkage :
  I32EnumAttrCase<"ExternalWeakLinkage", 9, "extern_weak">;
// Tentative definitions.
def Core_GlobalCommonLinkage :
  I32EnumAttrCase<"CommonLinkage", 10, "common">;

def Core_GlobalLinkageKind : Core_I32Enum<
    "GlobalLinkageKind", "Linkage type/kind",
    [
        Core_GlobalExternalLinkage,
        Core_GlobalAvailableExternallyLinkage,
        Core_GlobalLinkOnceAnyLinkage,
        Core_GlobalLinkOnceODRLinkage,
        Core_GlobalWeakAnyLinkage,
        Core_GlobalWeakODRLinkage,
        Core_GlobalInternalLinkage,
        Core_GlobalPrivateLinkage,
        Core_GlobalExternalWeakLinkage,
        Core_GlobalCommonLinkage,
        Core_GlobalAppendingLinkage
    ]>
{}

def Core_GlobalLinkageKindAttr : Core_I32EnumAttr<
  "global_linkage_kind", Core_GlobalLinkageKind
>;

#endif // VAST_DIALECT_CORE_IR_LINKAGE
