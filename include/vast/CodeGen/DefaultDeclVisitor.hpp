// Copyright (c) 2022-present, Trail of Bits, Inc.

#pragma once

#include "vast/Util/Warnings.hpp"

VAST_RELAX_WARNINGS
#include <clang/AST/DeclVisitor.h>
VAST_UNRELAX_WARNINGS

#include "vast/CodeGen/CodeGenVisitorBase.hpp"

namespace vast::cg {

    struct default_decl_visitor : decl_visitor_base< default_decl_visitor >
    {
        using base = decl_visitor_base< default_decl_visitor >;

        explicit default_decl_visitor(codegen_builder &bld, visitor_view self)
            : base(bld, self)
        {}

        using decl_visitor_base< default_decl_visitor >::Visit;

        //
        // Function Declaration
        //

        operation visit(const clang_decl *decl) { return Visit(decl); }
        operation visit_prototype(const clang_function *decl);
        mlir_attr_list visit_attrs(const clang_function *decl);

        //
        // Variable Declaration
        //

        operation VisitVarDecl(const clang_var_decl *decl);
        operation visit_var_init(const clang_var_decl *decl);
    };

} // namespace vast::cg