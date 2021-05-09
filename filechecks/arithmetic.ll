define void @main(i32 %a, i32 %b, i32 %c) {
; CHECK: start main 3:
entry:
    %0 = add i32 %a, %a
    %1 = shl i32 %a, 5
    %2 = lshr i32 %a, 3
    %3 = add i32 %a, 0
    %4 = add i32 %3, %b
    %5 = mul i32 %a, 1
    %6 = add i32 %5, %b
    %7 = and i32 %a, %a
    %8 = add i32 %7, %b
    %9 = udiv i32 %a, %a
    %10 = add i32 %a, %9
    br label %cond.branch
;CHECK: .entry:
;CHECK-NEXT: r[[#]] = mul arg1 2 32 
;CHECK-NEXT: r[[#]] = mul arg1 32 32 
;CHECK-NEXT: r[[#]] = udiv arg1 8 32 
;CHECK-NEXT: r[[#]] = add arg1 arg2 32 
;CHECK-NEXT: r[[#]] = add arg1 arg2 32
;CHECK-NEXT: r[[#]] = add arg1 arg2 32
;CHECK-NEXT: r[[#]] = add arg1 1 32
cond.branch:
    %cmp = icmp eq i32 %a, %b
    br i1 %cmp, label %cond.true, label %cond.false
;CHECK: .cond.branch:
;CHECK-NEXT: [[CMP:r[0-9]+]] = icmp eq arg1 arg2 32 
;CHECK-NEXT: br [[CMP]] .cond.true .cond.false 
cond.true:
    %11 = add i32 %a, %b
    br label %cond.end
;CHECK: .cond.true:
;CHECK-NEXT: r[[#]] = mul arg[[#]] 2 32
cond.false:
    %12 = add i32 %a, %b
    br label %cond.end
;CHECK: .cond.false:
;CHECK-NEXT: r[[#]] = add arg1 arg2 32
cond.end:
    ret void
}
; CHECK: end main