// RUN: %vast-front %s -vast-emit-mlir=hl -o - | FileCheck %s

// CHECK: hl.var "a" sc_static
static int a = 2;

int main() {
    int x = a;
    return 0;
}
