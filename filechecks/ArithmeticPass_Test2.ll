; It checks for both ArithmeticPass and GVMalloc2Alloca backend supports.
; GV @A goes to stack, @B and @C to heap.
; ArithmeticPass removes redundant instructions like "%1 = mul i64 2400, 8".

@A = global [6400 x i64] zeroinitializer, align 16
@B = global [6400 x i64] zeroinitializer, align 16
@C = global [6400 x i64] zeroinitializer, align 16

define i32 @main() {
; CHECK: start main 0:
entry:
; CHECK: .entry:
; CHECK-NEXT: ; Init global variables
; CHECK-NEXT:   sp = sub sp 51200 64 
; CHECK-NEXT:   r[[#R1:]] = malloc 51200 
; CHECK-NEXT:   r[[#R2:]] = malloc 51200 
  %call = call i64 (...) @read()
; CHECK-NEXT:   r[[#R3:]] = call read 
  %0 = ptrtoint [6400 x i64]* @A to i64
; CHECK-NEXT:   r[[#R4:]] = sub 102400 51200 64
  %1 = mul i64 2400, 8
  %2 = add i64 %0, %1
; CHECK-NEXT:   r[[#R5:]] = add r[[#R4]] 19200 64
  %3 = inttoptr i64 %2 to i64*
  store i64 %call, i64* %3, align 16
; CHECK-NEXT:   store 8 r[[#R3]] r[[#R5]] 0
  %4 = ptrtoint [6400 x i64]* @C to i64
; CHECK-NEXT:   r[[#R6:]] = add 204800 51200 64
  %5 = mul i64 4800, 8
  %6 = add i64 %4, %5
; CHECK-NEXT:   r[[#R7:]] = add r[[#R6]] 38400 64
  %7 = inttoptr i64 %6 to i64*
  store i64 %call, i64* %7, align 16
; CHECK-NEXT:   store 8 r[[#R3]] r[[#R7]] 0
  ret i32 0
; CHECK-NEXT:   ret 0
}
; CHECK: end main

declare i64 @read(...)