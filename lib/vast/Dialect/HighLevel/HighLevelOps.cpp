// Copyright (c) 2021-present, Trail of Bits, Inc.

#include "vast/Util/Warnings.hpp"

VAST_RELAX_WARNINGS
#include <llvm/Support/ErrorHandling.h>
#include <mlir/Support/LLVM.h>
#include <mlir/Support/LogicalResult.h>

#include <mlir/IR/Builders.h>
#include <mlir/IR/OperationSupport.h>
#include <mlir/IR/SymbolTable.h>
#include <mlir/IR/OpImplementation.h>

#include <mlir/Dialect/CommonFolders.h>

#include <mlir/Interfaces/CallInterfaces.h>
#include <mlir/Interfaces/FunctionImplementation.h>
VAST_UNRELAX_WARNINGS

#include "vast/Dialect/HighLevel/HighLevelAttributes.hpp"
#include "vast/Dialect/HighLevel/HighLevelDialect.hpp"
#include "vast/Dialect/HighLevel/HighLevelTypes.hpp"
#include "vast/Dialect/HighLevel/HighLevelUtils.hpp"
#include "vast/Dialect/HighLevel/HighLevelOps.hpp"

#include "vast/Dialect/Core/CoreAttributes.hpp"
#include "vast/Dialect/Core/CoreDialect.hpp"
#include "vast/Dialect/Core/CoreTypes.hpp"
#include "vast/Dialect/Core/Func.hpp"
#include "vast/Dialect/Core/Linkage.hpp"
#include "vast/Dialect/Core/SymbolTable.hpp"
#include "vast/Dialect/Core/Interfaces/DesugarTypeInterface.hpp"

#include "vast/Util/Common.hpp"
#include "vast/Util/Region.hpp"
#include "vast/Util/TypeUtils.hpp"
#include "vast/Util/Enum.hpp"

#include <optional>
#include <variant>

namespace vast::hl
{
    using FoldResult = mlir::OpFoldResult;

    //===----------------------------------------------------------------------===//
    // TranslationUnitOp
    //===----------------------------------------------------------------------===//

    void TranslationUnitOp::build(Builder &bld, State &st, builder_callback_ref decls) {
        InsertionGuard guard(bld);
        build_region(bld, st, decls);
    }

    //===----------------------------------------------------------------------===//
    // ArithBinOps
    //===----------------------------------------------------------------------===//

    namespace {
        logical_result verify_float_arith_op(operation op) {
            VAST_ASSERT(op -> getNumResults() == 1);
            auto res_type = strip_complex(op->getResult(0).getType());

            for (auto sugared_type : op->getOperandTypes()) {
                auto t = core::desugar_type(sugared_type);
                if (t.hasTrait< core::TypedefTrait >() || t.hasTrait< core::TypeOfTrait >()) {
                    return logical_result::success();
                }
                if (strip_complex(t) != res_type) {
                    return logical_result::failure();
                }
            }

            return logical_result::success();
        }
    } // namespace

    logical_result FCmpOp::verify() {
        auto lhs = strip_complex(strip_elaborated(getLhs()));
        auto rhs = strip_complex(strip_elaborated(getRhs()));
        return logical_result::success(
            lhs == rhs
            || any_with_trait< core::TypedefTrait >(lhs, rhs)
            || any_with_trait< core::TypeOfTrait >(lhs, rhs)
        );
    }

    FoldResult checked_int_arithmetic(mlir_type type, auto adaptor, auto &&op) {
        if (auto lhs = mlir::dyn_cast_or_null< core::IntegerAttr >(adaptor.getLhs())) {
            if (auto rhs = mlir::dyn_cast_or_null< core::IntegerAttr >(adaptor.getRhs())) {
                if (auto result = op(lhs.getValue(), rhs.getValue())) {
                    return core::IntegerAttr::get(type, result.value());
                }
            }
        }

        return {};
    }

    namespace {

        FoldResult fold_integral_cast(auto &self, auto adaptor) {
            if (self.getResult().getType() == self.getValue().getType())
                return self.getValue();
            return {};
        }

    } // namespace

    FoldResult ImplicitCastOp::fold(FoldAdaptor adaptor) {
        if (getKind() == CastKind::IntegralCast)
            return fold_integral_cast(*this, adaptor);
        return {};
    }

    FoldResult AddIOp::fold(FoldAdaptor /* adaptor */) { 
        return {}; 
    }

    FoldResult SubIOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult AddFOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult SubFOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult MulIOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult MulFOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult DivSOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult DivUOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult DivFOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult RemSOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult RemUOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult RemFOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult BinXorOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult BinOrOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult BinAndOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult BinLAndOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult BinLOrOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult BinComma::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult BinShlOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult BinLShrOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    FoldResult BinAShrOp::fold(FoldAdaptor /* adaptor */) {
        return {};
    }

    void build_logic_op(
        Builder &bld, State &st, Type type,
        builder_callback_ref lhs,
        builder_callback_ref rhs
    ) {
        VAST_ASSERT(lhs && "the builder callback for 'lhs' region must be present");
        VAST_ASSERT(rhs && "the builder callback for 'rhs' region must be present");

        InsertionGuard guard(bld);
        build_region(bld, st, lhs);
        build_region(bld, st, rhs);
        st.addTypes(type);
    }

    void BinLAndOp::build(
        Builder &bld, State &st, Type type,
        builder_callback_ref lhs,
        builder_callback_ref rhs
    ) {
        build_logic_op(bld, st, type, lhs, rhs);
    }

    void BinLOrOp::build(
        Builder &bld, State &st, Type type,
        builder_callback_ref lhs,
        builder_callback_ref rhs
    ) {
        build_logic_op(bld, st, type, lhs, rhs);
    }

    //===----------------------------------------------------------------------===//
    // FunctionOp
    //===----------------------------------------------------------------------===//

    static llvm::StringRef getLinkageAttrNameString() { return "linkage"; }

    logical_result FuncOp::verify() {
        return core::verifyFuncOp(*this);
    }

    ParseResult parseFunctionSignatureAndBody(
        Parser &parser, Attribute &funcion_type, mlir::NamedAttrList &attr_dict, Region &body
    ) {
        return core::parseFunctionSignatureAndBody(parser, funcion_type, attr_dict, body);
    }

    void printFunctionSignatureAndBody(
        Printer &printer, FuncOp op, Attribute function_type,
        mlir::DictionaryAttr dict_attr, Region &body
    ) {
        return core::printFunctionSignatureAndBodyImpl(printer, op, function_type, dict_attr, body);
    }

    FoldResult ConstantOp::fold(FoldAdaptor adaptor) {
        VAST_CHECK(adaptor.getOperands().empty(), "constant has no operands");
        return adaptor.getValue();
    }

    bool ConstantOp::isBuildableWith(mlir_attr value, mlir_type type) {
        auto typed = mlir::dyn_cast< mlir::TypedAttr >(value);

        if (!typed || typed.getType() != type) {
            return false;
        }

        return value.hasTrait< core::ConstantLikeAttrTrait >();
    }

    void build_expr_trait(
        Builder &bld, State &st, Type rty,
        builder_callback_ref expr
    ) {
        InsertionGuard guard(bld);
        build_region(bld, st, expr);
        st.addTypes(rty);
    }

    void SizeOfExprOp::build(
        Builder &bld, State &st, Type rty,
        builder_callback_ref expr
    ) {
        build_expr_trait(bld, st, rty, expr);
    }

    void AlignOfExprOp::build(
        Builder &bld, State &st, Type rty,
        builder_callback_ref expr
    ) {
        build_expr_trait(bld, st, rty, expr);
    }

    void PreferredAlignOfExprOp::build(
        Builder &bld, State &st, Type rty,
        builder_callback_ref expr
    ) {
        build_expr_trait(bld, st, rty, expr);
    }

    void OffsetOfExprOp::build(
        Builder &bld, State &st, Type rty, mlir::TypeAttr source, mlir::ArrayAttr components,
        const std::vector< builder_callback > &builders
    ) {
        InsertionGuard guard(bld);
        st.addTypes(rty);
        st.addAttribute(getSourceAttrName(st.name), source);
        st.addAttribute(getComponentsAttrName(st.name), components);
        for (const auto &callback : builders) {
            build_region(bld, st, builder_callback_ref(callback));
        }
    }

    void OffsetOfExprOp::build(
        Builder &bld, State &st, Type rty, Type source, mlir::ArrayAttr components,
        const std::vector< builder_callback > &builders
    ) {
        OffsetOfExprOp::build(bld, st, rty, mlir::TypeAttr::get(source), components, builders);
    }

    void StmtExprOp::build(
        Builder &bld, State &st, Type rty,
        builder_callback_ref expr
    ) {
        InsertionGuard guard(bld);
        build_region(bld, st, expr);
        st.addTypes(rty);
    }

    void VarDeclOp::build(
        Builder &bld, State &st,
        Type type,
        llvm::StringRef name,
        core::StorageClass storage_class,
        core::TSClass thread_storage_class,
        bool constant,
        std::optional< core::GlobalLinkageKind > linkage,
        maybe_builder_callback_ref init,
        maybe_builder_callback_ref alloc
    ) {
        auto ctx = bld.getContext();
        st.addAttribute(core::symbol_attr_name(), bld.getStringAttr(name));
        st.addAttribute("type", mlir::TypeAttr::get(type));
        st.addAttribute("storageClass", core::StorageClassAttr::get(ctx, storage_class));
        st.addAttribute("threadStorageClass", core::TSClassAttr::get(ctx, thread_storage_class));
        if (constant) {
            st.addAttribute(getConstantAttrName(st.name), bld.getUnitAttr());
        }
        if (linkage) {
            st.addAttribute("linkage", core::GlobalLinkageKindAttr::get(ctx, linkage.value()));
        }
        InsertionGuard guard(bld);

        build_region(bld, st, init);
        build_region(bld, st, alloc);
    }

    void EnumDeclOp::build(
        Builder &bld, State &st, llvm::StringRef name, Type type,
        builder_callback_ref constants
    ) {
        st.addAttribute("sym_name", bld.getStringAttr(name));
        st.addAttribute("type", mlir::TypeAttr::get(type));
        InsertionGuard guard(bld);
        build_region(bld, st, constants);
    }

    void EnumDeclOp::build(Builder &bld, State &st, llvm::StringRef name) {
        st.addAttribute("sym_name", bld.getStringAttr(name));
        build_empty_region(bld, st);
    }

    // The following printer and parser are stolen from tablegen generated code
    // apart from adding the empty block to the constants region
    ParseResult EnumDeclOp::parse(Parser &parser, State &result) {
        mlir::StringAttr nameAttr;
        mlir::TypeAttr typeAttr;
        std::unique_ptr< Region > constantsRegion = std::make_unique< Region >();

        if (parser.parseSymbolName(nameAttr))
            return mlir::failure();
        if (nameAttr) result.attributes.append("sym_name", nameAttr);

        parser.getCurrentLocation();
        if (parser.parseOptionalAttrDict(result.attributes)) {
            return mlir::failure();
        }

        if (mlir::succeeded(parser.parseOptionalColon())) {
            if (parser.parseCustomAttributeWithFallback(
                    typeAttr, parser.getBuilder().getType< mlir::NoneType >()
                ))
            {
                return mlir::failure();
            }
            if (typeAttr) {
                result.attributes.append("type", typeAttr);
            }

            if (parser.parseRegion(*constantsRegion)) {
                return mlir::failure();
            }
        }
        // Here is the addition
        if (constantsRegion->empty()) {
            constantsRegion->emplaceBlock();
        }
        result.addRegion(std::move(constantsRegion));
        return mlir::success();
    }

    void EnumDeclOp::print(Printer &odsPrinter) {
        odsPrinter << ' ';
        odsPrinter.printSymbolName(getSymName());
        llvm::SmallVector< llvm::StringRef, 2 > elidedAttrs;
        elidedAttrs.push_back("sym_name");
        elidedAttrs.push_back("type");
        odsPrinter.printOptionalAttrDict((*this)->getAttrs(), elidedAttrs);
        if (getTypeAttr()) {
            odsPrinter << ' ' << ":";
            odsPrinter << ' ';
            odsPrinter.printAttributeWithoutType(getTypeAttr());
            odsPrinter << ' ';
            odsPrinter.printRegion(getConstants());
        }
    }

    namespace detail {
        void build_record_like_decl(
            Builder &bld, State &st, llvm::StringRef name,
            maybe_builder_callback_ref fields
        ) {
            st.addAttribute("sym_name", bld.getStringAttr(name));

            InsertionGuard guard(bld);
            build_region(bld, st, fields);
        }

        void build_cxx_record_like_decl(
            Builder &bld, State &st, llvm::StringRef name,
            maybe_builder_callback_ref bases,
            maybe_builder_callback_ref fields
        ) {
            st.addAttribute("sym_name", bld.getStringAttr(name));

            InsertionGuard guard(bld);
            build_region(bld, st, bases);
            build_region(bld, st, fields);
        }
    } // namespace detail

    void StructDeclOp::build(
        Builder &bld, State &st, llvm::StringRef name,
        maybe_builder_callback_ref fields
    ) {
        detail::build_record_like_decl(bld, st, name, fields);
    }

    void UnionDeclOp::build(
        Builder &bld, State &st, llvm::StringRef name,
        maybe_builder_callback_ref fields
    ) {
        detail::build_record_like_decl(bld, st, name, fields);
    }

    void CxxStructDeclOp::build(
        Builder &bld, State &st, llvm::StringRef name,
        maybe_builder_callback_ref bases,
        maybe_builder_callback_ref fields
    ) {
        detail::build_cxx_record_like_decl(bld, st, name, bases, fields);
    }

    void ClassDeclOp::build(
        Builder &bld, State &st, llvm::StringRef name,
        maybe_builder_callback_ref bases,
        maybe_builder_callback_ref fields
    ) {
        detail::build_cxx_record_like_decl(bld, st, name, bases, fields);
    }

    mlir::CallInterfaceCallable CallOp::getCallableForCallee() {
        return core::get_callable_for_callee(*this);
    }

    void CallOp::setCalleeFromCallable(mlir::CallInterfaceCallable callee) {
        setOperand(0, callee.get< mlir_value >());
    }

    mlir::CallInterfaceCallable IndirectCallOp::getCallableForCallee() {
        return (*this)->getOperand(0);
    }

    void IndirectCallOp::setCalleeFromCallable(mlir::CallInterfaceCallable callee) {
        setOperand(0, callee.get< mlir_value >());
    }

    mlir::Operation *CallOp::resolveCallable() {
        return core::symbol_table::lookup< core::func_symbol >(getOperation(), getCallee());
    }

    mlir::Operation *CallOp::resolveCallableInTable(::mlir::SymbolTableCollection &) {
        VAST_UNIMPLEMENTED;
    }

    mlir::ParseResult IfOp::parse(mlir::OpAsmParser &parser, mlir::OperationState &result) {
        std::unique_ptr< mlir::Region > condRegion = std::make_unique< mlir::Region >();
        std::unique_ptr< mlir::Region > thenRegion = std::make_unique< mlir::Region >();
        std::unique_ptr< mlir::Region > elseRegion = std::make_unique< mlir::Region >();

        if (parser.parseRegion(*condRegion))
            return mlir::failure();

        if (parser.parseKeyword("then"))
            return mlir::failure();

        if (parser.parseRegion(*thenRegion))
            return mlir::failure();

        if (thenRegion->empty())
            thenRegion->emplaceBlock();

        if (mlir::succeeded(parser.parseOptionalKeyword("else"))) {

            if (parser.parseRegion(*elseRegion))
                return mlir::failure();

            if (elseRegion->empty())
                elseRegion->emplaceBlock();
        }

        if (parser.parseOptionalAttrDict(result.attributes))
          return mlir::failure();

        result.addRegion(std::move(condRegion));
        result.addRegion(std::move(thenRegion));
        result.addRegion(std::move(elseRegion));
        return mlir::success();
    }

    void IfOp::print(mlir::OpAsmPrinter &odsPrinter) {
        odsPrinter << ' ';
        odsPrinter.printRegion(getCondRegion());
        odsPrinter << ' ' << "then" << ' ';
        odsPrinter.printRegion(getThenRegion());

        if (!getElseRegion().empty()) {
            odsPrinter << ' ' << "else" << ' ';
            odsPrinter.printRegion(getElseRegion());
        }

        llvm::SmallVector< llvm::StringRef, 2 > elidedAttrs;
        odsPrinter.printOptionalAttrDict((*this)->getAttrs(), elidedAttrs);
    }

    void IfOp::build(
        Builder &bld, State &st,
        builder_callback_ref cond,
        builder_callback_ref then_builder,
        maybe_builder_callback_ref else_builder
    ) {
        InsertionGuard guard(bld);
        build_region(bld, st, cond);
        build_region(bld, st, then_builder);
        build_region(bld, st, else_builder);
    }

    void BinaryCondOp::build(
        Builder &bld, State &st, Type type,
        builder_callback_ref common_builder,
        builder_callback_ref cond_builder,
        builder_callback_ref then_builder,
        builder_callback_ref else_builder
    ) {
        InsertionGuard guard(bld);
        build_region(bld, st, common_builder);
        build_region(bld, st, cond_builder);
        build_region(bld, st, then_builder);
        build_region(bld, st, else_builder);
        st.addTypes(type);
    }

    namespace {
        bool typesMatch(Type lhs, Type rhs) {
            if (auto e = mlir::dyn_cast< hl::ElaboratedType >(lhs)) {
                return typesMatch(e.getElementType(), rhs);
            }
            if (auto e = mlir::dyn_cast< hl::ElaboratedType >(rhs)) {
                return typesMatch(lhs, e.getElementType());
            }

            return lhs == rhs
                || all_with_trait< core::IntegralTypeTrait >(lhs, rhs)
                || any_with_trait< core::TypedefTrait >(lhs, rhs)
                || any_with_trait< core::TypeOfTrait >(lhs, rhs)
                || any_with_trait< core::AutoTrait >(lhs, rhs)
                || all_with_trait< core::PointerTypeTrait >(lhs, rhs);
        }

        logical_result verify_condop_yields(Region &lhs, Region &rhs, Location loc) {
            auto then_type = get_maybe_yielded_type(lhs);
            auto else_type = get_maybe_yielded_type(rhs);

            bool compatible = typesMatch(then_type, else_type);
            if (!compatible) {
                VAST_REPORT(
                    "Failed to verify that return types {0}, {1} in conditional operation "
                    "regions match. See location {2}",
                    then_type, else_type, loc
                );
            }
            return mlir::success(compatible);
        }
    } // namespace

    logical_result CondOp::verifyRegions() {
        return verify_condop_yields(getThenRegion(), getElseRegion(), getLoc());
    }

    logical_result BinaryCondOp::verifyRegions() {
        return verify_condop_yields(getThenRegion(), getElseRegion(), getLoc());
    }

    void WhileOp::build(
        Builder &bld, State &st,
        builder_callback_ref cond,
        builder_callback_ref body
    ) {
        InsertionGuard guard(bld);
        build_region(bld, st, cond);
        build_region(bld, st, body);
    }

    void ForOp::build(
        Builder &bld, State &st,
        builder_callback_ref cond,
        builder_callback_ref incr,
        builder_callback_ref body
    ) {
        InsertionGuard guard(bld);
        build_region(bld, st, cond);
        build_region(bld, st, incr);
        build_region(bld, st, body);
    }

    void DoOp::build(
        Builder &bld, State &st,
        builder_callback_ref body,
        builder_callback_ref cond
    ) {
        InsertionGuard guard(bld);
        build_region(bld, st, body);
        build_region(bld, st, cond);
    }

    void SwitchOp::build(
        Builder &bld, State &st,
        builder_callback_ref cond,
        builder_callback_ref body
    ) {
        InsertionGuard guard(bld);
        build_region(bld, st, cond);
        build_region(bld, st, body);
    }

    void CaseOp::build(
        Builder &bld, State &st,
        builder_callback_ref lhs,
        builder_callback_ref body
    ) {
        InsertionGuard guard(bld);
        build_region(bld, st, lhs);
        build_region(bld, st, body);
    }

    void DefaultOp::build(Builder &bld, State &st, builder_callback_ref body)
    {
        InsertionGuard guard(bld);
        build_region(bld, st, body);
    }

    void LabelStmt::build(
        Builder &bld, State &st, Value label,
        builder_callback_ref substmt
    ) {
        st.addOperands(label);

        InsertionGuard guard(bld);
        build_region(bld, st, substmt);
    }

    void IndirectGotoStmt::build(
        Builder &bld, State &st, builder_callback_ref target)
    {
        InsertionGuard guard(bld);
        build_region(bld, st, target);
    }


    void ExprOp::build(
        Builder &bld, State &st, Type rty,
        builder_callback_ref expr
    ) {
        InsertionGuard guard(bld);
        build_region(bld, st, expr);
        st.addTypes(rty);
    }

    void TypeOfExprOp::build(
        Builder &bld, State &st, llvm::StringRef name, Type type,
        maybe_builder_callback_ref expr
    ) {
        st.addAttribute("name", bld.getStringAttr(name));
        st.addAttribute("type", mlir::TypeAttr::get(type));

        InsertionGuard guard(bld);
        build_region(bld, st, expr);
    }

    mlir_type TypeDeclOp::getDefinedType() {
        return hl::RecordType::get(getContext(), getSymName());
    }

    mlir_type TypeDefOp::getDefinedType() {
        return hl::TypedefType::get(getContext(), getSymName());
    }

    FuncOp getCallee(CallOp call)
    {
        auto coi = mlir::cast< VastCallOpInterface >(call.getOperation());
        return mlir::dyn_cast_or_null<FuncOp>(coi.resolveCallable());
    }

    void AsmOp::build(
            Builder &bld,
            State &st,
            mlir::StringAttr asm_template,
            bool is_volatile,
            bool has_goto,
            llvm::ArrayRef< mlir::Value > outs,
            llvm::ArrayRef< mlir::Value > ins,
            mlir::ArrayAttr out_names,
            mlir::ArrayAttr in_names,
            mlir::ArrayAttr out_constraints,
            mlir::ArrayAttr in_constraints,
            mlir::ArrayAttr clobbers,
            llvm::ArrayRef< mlir::Value > labels)
    {
        st.addAttribute(getAsmTemplateAttrName(st.name), asm_template);
        st.addOperands(outs);
        st.addOperands(ins);
        st.addOperands(labels);

        st.addAttribute(getOperandSegmentSizesAttrName(st.name),
                        bld.getDenseI32ArrayAttr({static_cast<int32_t>(outs.size()),
                                                  static_cast<int32_t>(ins.size()),
                                                  static_cast<int32_t>(labels.size())
                                                 })
                        );

        if (is_volatile)
            st.addAttribute(getIsVolatileAttrName(st.name), bld.getUnitAttr());
        if (has_goto)
            st.addAttribute(getHasGotoAttrName(st.name), bld.getUnitAttr());

        if (outs.size() > 0 && out_names)
            st.addAttribute(getOutputNamesAttrName(st.name), out_names);
        if (ins.size() > 0 && in_names)
            st.addAttribute(getInputNamesAttrName(st.name), in_names);

        if (outs.size() > 0 && out_constraints)
            st.addAttribute(getOutputConstraintsAttrName(st.name), out_constraints);
        if (ins.size() > 0 && in_constraints)
            st.addAttribute(getInputConstraintsAttrName(st.name), in_constraints);

        if (clobbers && clobbers.size())
            st.addAttribute(getClobbersAttrName(st.name), clobbers);
    }


    void InitializedConstantOp::build(Builder &bld, State &st, Type type, builder_callback init)
    {
        VAST_ASSERT(init && "the builder callback for 'init' region must be present");

        InsertionGuard guard(bld);

        build_region(bld, st, builder_callback_ref(init));
        st.addTypes(type);
    }

    void CompoundLiteralOp::build(Builder &bld, State &st, Type type, builder_callback init)
    {
        VAST_ASSERT(init && "the builder callback for 'init' region must be present");

        InsertionGuard guard(bld);

        build_region(bld, st, builder_callback_ref(init));
        st.addTypes(type);
    }

    std::size_t handle_size_of(auto op, mlir_type type) {
        auto eval = [op] (mlir_type ty) -> std::size_t {
            // sizeof(void), sizeof(function) = 1 as a gcc extension
            if (ty.hasTrait< core::VoidTrait >()) {
                return 1;
            }

            if (mlir::isa< core::FunctionType >(ty)) {
                return 1;
            }

            // TODO: yield an error on dependent type

            // TODO: yield an error on vla

            return mlir::DataLayout::closest(*op).getTypeSize(ty);
        };

        // C++ [expr.sizeof]p2: "When applied to a reference or a reference type,
        // the result is the size of the referenced type."
        if (auto ref = mlir::dyn_cast< hl::ReferenceType >(type)) {
            return eval(ref.getElementType());
        }

        return eval(type);
    }

    //
    // SizeOfTypeOp
    //
    std::size_t SizeOfTypeOp::getValue() { return handle_size_of(this, getArg()); }

    FoldResult SizeOfTypeOp::fold(FoldAdaptor) {
        return core::IntegerAttr::get(getType(), apsint(getValue()));
    }

    //
    // SizeOfExprOp
    //
    std::size_t SizeOfExprOp::getValue() { return handle_size_of(this, getType()); }

    FoldResult SizeOfExprOp::fold(FoldAdaptor adaptor) {
        return core::IntegerAttr::get(getType(), apsint(getValue()));
    }

    //
    // BuiltinTypesCompatiblePOp
    //
    types_t BuiltinTypesCompatiblePOp::getArgs() {
        return types_t{getType1(), getType2()};
    }

    std::optional< bool > BuiltinTypesCompatiblePOp::getValue() {
        return std::optional(getCompatibleAttr().getValue());
    }

    //
    // GenericSelectionExpr
    //

    std::optional< region_t *> GenericSelectionExpr::getResultRegion() {
        if (auto selected = getSelected()) {
            auto op_it = getBody().op_begin();
            std::advance(op_it, selected.value().getZExtValue());
            if (auto assoc = mlir::dyn_cast< hl::GenericAssocExpr >(*op_it))
            return { &assoc.getBody() };
        }
        return std::nullopt;
    }

    bool GenericSelectionExpr::isExprPredicate() {
        return !getControl().empty();
    }

    bool GenericSelectionExpr::isTypePredicate() {
        return (*this)->hasAttr("matchType");
    }

    //
    // AtomicExpr
    //

    void AtomicExpr::build(
        Builder &bld,
        State &st,
        llvm::StringRef name,
        mlir_type type,
        const std::vector< builder_callback > &builders
    ) {
        InsertionGuard guard(bld);
        st.addAttribute(getNameAttrName(st.name), bld.getStringAttr(name));
        st.addTypes(type);
        for (builder_callback_ref builder : builders)
            build_region(bld, st, builder);
    }
}

//===----------------------------------------------------------------------===//
// TableGen generated logic.
//===----------------------------------------------------------------------===//

using namespace vast::hl;
using vast::core::parseStorageClasses;
using vast::core::printStorageClasses;

#define GET_OP_CLASSES
#include "vast/Dialect/HighLevel/HighLevel.cpp.inc"
