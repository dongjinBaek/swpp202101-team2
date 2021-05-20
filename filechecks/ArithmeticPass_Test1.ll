define i32 @main(i32 %a, i32 %b, i16 %c, i16 %d) {
; CHECK: start main 4:
entry:
    %0 = add i32 %a, %a
    %1 = shl i32 %a, 5
    %2 = lshr i32 %a, 3
    %3 = add i32 %a, 0
    %4 = add i32 %3, %b
    %5 = mul i32 %a, 1
    %6 = add i32 %5, %4
    %7 = and i32 %a, %a
    %8 = add i32 %7, %6
    %9 = udiv i32 %a, %a
    %10 = add i32 %a, %9
    ret i32 %0
}
;CHECK: .entry:
;CHECK-NEXT: r[[#]] = mul arg1 2 32 
;CHECK-NEXT: r[[#]] = mul arg1 32 32 
;CHECK-NEXT: r[[#]] = udiv arg1 8 32 
;CHECK-NEXT: r[[#]] = add arg1 arg2 32 
;CHECK-NEXT: r[[#]] = add arg1 r[[#]] 32
;CHECK-NEXT: r[[#]] = add arg1 r[[#]] 32
;CHECK-NEXT: r[[#]] = add arg1 1 32
; CHECK: end main