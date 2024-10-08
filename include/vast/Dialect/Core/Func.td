// Copyright (c) 2023, Trail of Bits, Inc.

#ifndef VAST_DIALECT_CORE_FUNC
#define VAST_DIALECT_CORE_FUNC

include "mlir/IR/OpBase.td"

include "mlir/Interfaces/CallInterfaces.td"
include "vast/Dialect/Core/Interfaces/SymbolInterface.td"
include "vast/Dialect/Core/Interfaces/FunctionInterface.td"

include "vast/Dialect/Core/CoreTraits.td"
include "vast/Dialect/Core/LinkageHelper.td"

def Core_FunctionType : Type<
    CPred<"::llvm::isa<::vast::core::FunctionType>($_self)">,
    "function type",
    "::vast::core::FunctionType"
>;

class Core_FuncBaseOp< Dialect dialect, string mnemonic, list< Trait > traits = [] >
  : Op< dialect, mnemonic,
    !listconcat(traits, [
      AutomaticAllocationScope,
      IsolatedFromAbove,
      Core_FunctionOpInterface,
      Core_FuncSymbol,
      NoTerminator
    ])
> {
  let summary = "VAST function template";

  let description = [{
    Inspired by `cir::FuncOp` and `mlir::func::FuncOp`:
    > Operations within the function cannot implicitly capture values defined
    > outside of the function, i.e. Functions are `IsolatedFromAbove`. All
    > external references must use function arguments or attributes that establish
    > a symbolic connection (e.g. symbols referenced by name via a string
    > attribute like SymbolRefAttr). An external function declaration (used when
    > referring to a function declared in some other module) has no body.

    The function linkage information is specified by `linkage`, as defined by
    `GlobalLinkageKind` attribute.
  }];

  let arguments =(ins
    SymbolNameAttr:$sym_name,
    TypeAttrOf<Core_FunctionType>:$function_type,
    OptionalAttr<Core_GlobalLinkageKind>:$linkage,
    OptionalAttr<StrAttr>:$sym_visibility,
    OptionalAttr<DictArrayAttr>:$arg_attrs,
    OptionalAttr<DictArrayAttr>:$res_attrs
  );

  let regions = (region AnyRegion:$body);

  let skipDefaultBuilders = 1;

  let builders = [
    OpBuilder< (ins
      "llvm::StringRef":$name,
      "core::FunctionType":$type,
      "std::unique_ptr< mlir::Region >":$body,
      CArg< "core::GlobalLinkageKind", "core::GlobalLinkageKind::ExternalLinkage" >:$linkage,
      CArg< "llvm::ArrayRef<mlir::NamedAttribute>", "{}" >:$attrs,
      CArg< "llvm::ArrayRef<mlir::DictionaryAttr>", "{}" >:$arg_attrs,
      CArg< "llvm::ArrayRef<mlir::DictionaryAttr>", "{}" >:$res_attrs
    ), [{
      InsertionGuard guard($_builder);
      if (body)
        $_state.addRegion(std::move(body));
      else
        $_state.addRegion();

      $_state.addAttribute(
        mlir::SymbolTable::getSymbolAttrName(), $_builder.getStringAttr(name)
      );
      $_state.addAttribute(getFunctionTypeAttrName($_state.name), mlir::TypeAttr::get(type));
      $_state.addAttribute(
        "linkage", core::GlobalLinkageKindAttr::get($_builder.getContext(), linkage)
      );

      $_state.attributes.append(attrs.begin(), attrs.end());

      if (arg_attrs.empty())
        return;

      vast::core::function_interface_impl::addArgAndResultAttrs(
        $_builder, $_state, arg_attrs, res_attrs,
        getArgAttrsAttrName($_state.name), getResAttrsAttrName($_state.name)
      );
    }] >,
    OpBuilder< (ins
      "llvm::StringRef":$name,
      "core::FunctionType":$type,
      CArg< "core::GlobalLinkageKind", "core::GlobalLinkageKind::ExternalLinkage" >:$linkage,
      CArg< "llvm::ArrayRef<mlir::NamedAttribute>", "{}" >:$attrs,
      CArg< "llvm::ArrayRef<mlir::DictionaryAttr>", "{}" >:$arg_attrs,
      CArg< "llvm::ArrayRef<mlir::DictionaryAttr>", "{}" >:$res_attrs
    ), [{
      build($_builder, $_state, name, type, nullptr, linkage, attrs, arg_attrs, res_attrs);
    }]>,
    OpBuilder< (ins
      "llvm::StringRef":$name,
      "core::FunctionType":$type,
      CArg< "core::GlobalLinkageKind", "core::GlobalLinkageKind::ExternalLinkage" >:$linkage,
      CArg< "llvm::ArrayRef<mlir::NamedAttribute>", "{}" >:$attrs,
      CArg< "llvm::ArrayRef<mlir::DictionaryAttr>", "{}" >:$arg_attrs,
      CArg< "llvm::ArrayRef<mlir::DictionaryAttr>", "{}" >:$res_attrs,
      CArg< "maybe_builder_callback_ref", "std::nullopt" >:$body
    ), [{
      InsertionGuard guard($_builder);
      auto region = std::make_unique< mlir::Region >();
      build_region(region.get(), $_builder, $_state, body);
      build($_builder, $_state, name, type, std::move(region), linkage, attrs, arg_attrs, res_attrs);
    }] >
  ];

  let assemblyFormat = [{
    $sym_name custom< FunctionSignatureAndBody >($function_type, attr-dict, $body)
  }];

  let hasVerifier = 1;

  let extraClassDeclaration = [{
    bool isVarArg() { return getFunctionType().isVarArg(); }

    //===------------------------------------------------------------------===//
    // CallableOpInterface
    //===------------------------------------------------------------------===//

    /// Returns the region on the current operation that is callable. This may
    /// return null in the case of an external callable object, e.g. an external
    /// function.
    ::mlir::Region *getCallableRegion() {
      return isExternal() ? nullptr : &getBody();
    }

    /// Returns the results types that the callable region produces when
    /// executed.
    llvm::ArrayRef<Type> getCallableResults() {
      return getFunctionType().getResults();
    }

    /// Returns the argument attributes for all callable region arguments or
    /// null if there are none.
    ::mlir::ArrayAttr getCallableArgAttrs() {
      return getArgAttrs().value_or(nullptr);
    }

    /// Returns the result attributes for all callable region results or
    /// null if there are none.
    ::mlir::ArrayAttr getCallableResAttrs() {
      return getResAttrs().value_or(nullptr);
    }

    //===------------------------------------------------------------------===//
    // FunctionOpInterface Methods
    //===------------------------------------------------------------------===//

    /// Returns the argument types of this function.
    llvm::ArrayRef<Type> getArgumentTypes() { return getFunctionType().getInputs(); }

    /// Returns the result types of this function.
    llvm::ArrayRef<Type> getResultTypes() { return getFunctionType().getResults(); }

    //===------------------------------------------------------------------===//
    // SymbolOpInterface Methods
    //===------------------------------------------------------------------===//

    // TODO(#638): Find better way to mark alias functions as definitions
    bool isDeclaration() { return isExternal() && !getOperation()->hasAttr("hl.alias"); }
  }];
}

#endif // VAST_DIALECT_CORE_FUNC
