// RUN: %vast-front -vast-emit-mlir=hl %s -o - | %vast-opt --vast-hl-ude | %file-check %s

// CHECK-NOT: hl.struct @unused_typedecl
struct unused_typedecl;

// CHECK: hl.struct @used_typedecl
struct used_typedecl;

void use(struct used_typedecl *) {}
