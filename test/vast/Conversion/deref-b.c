// RUN: %vast-cc1 -vast-emit-mlir-after=vast-lower-value-categories %s -o - | %file-check %s

void set(int *ptr, int *v)
{
    // CHECK: [[V0:%[0-9]+]] = ll.alloca : !hl.ptr<!hl.ptr<si32>>
    // CHECK: ll.store [[V0]], %arg0 : !hl.ptr<!hl.ptr<si32>>, !hl.ptr<si32>
    // CHECK: [[V1:%[0-9]+]] = ll.alloca : !hl.ptr<!hl.ptr<si32>>
    // CHECK: ll.store [[V1]], %arg1 : !hl.ptr<!hl.ptr<si32>>, !hl.ptr<si32>
    // CHECK: [[V2:%[0-9]+]] = ll.load [[V0]] : (!hl.ptr<!hl.ptr<si32>>) -> !hl.ptr<si32>
    // CHECK: [[V3:%[0-9]+]] = ll.load [[V1]] : (!hl.ptr<!hl.ptr<si32>>) -> !hl.ptr<si32>
    // CHECK: [[V4:%[0-9]+]] = ll.load [[V3]] : (!hl.ptr<si32>) -> si32
    // CHECK: ll.store [[V2]], [[V4]] : !hl.ptr<si32>, si32
    *ptr = *v;
}
