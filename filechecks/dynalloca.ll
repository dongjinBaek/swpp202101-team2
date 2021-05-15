declare i8* @malloc(i64) #0
declare void @free(i8*) #1
declare i64 @read() #2

define void @func(i64 %n, i64* %temp) #3 {
; CHECK: start func 2:
entry:
; CHECK: .entry:
  %call = call i8* @malloc(i64 %n)
; CHECK-NEXT: r[[#R1:]] = malloc arg1
  call void @free(i8* %call)
; CHECK-NEXT: r[[#R2:]] = mul r[[#R1]] 1 64
; CHECK-NEXT: r[[#R3:]] = icmp ule r[[#R2]] 102400 64
; CHECK-NEXT: br r[[#R3]] ._defaultBB[[#BB1:]] ._defaultBB[[#BB2:]]
; CHECK: ._defaultBB[[#BB2]]:
; CHECK-NEXT: free r[[#R1]]
; CHECK-NEXT: br ._defaultBB[[#BB1]]
; CHECK: ._defaultBB[[#BB1]]:
  ret void
; CHECK-NEXT: ret 0
}
; CHECK: end func

define i32 @main() #4 {
; CHECK: start main 0
entry:
; CHECK: .entry:
; CHECK-NEXT: ; Init global variables
; CHECK-NEXT: ; Init stack pointer
  %temp = alloca [256 x i64], align 16
; CHECK-NEXT: sp = sub sp 2048 64
; CHECK-NEXT: r32 = mul sp 1 64
  %call = call i64 @read()
; CHECK-NEXT: r[[#R4:]] = call read
  %call1 = call i8* @malloc(i64 %call)
; CHECK-NEXT: r[[#R5:]] = icmp ult sp r[[#R4]] 64
; CHECK-NEXT: br r[[#R5]] .__sp.then[[#]] .__sp.else[[#]]
; CHECK: .__sp.then[[#]]:
; CHECK-NEXT: r[[#R6:]] = malloc r[[#R4]]
; CHECK-NEXT: br .__sp.next[[#]]
; CHECK: .__sp.else[[#]]:
; CHECK-NEXT: sp = sub sp r[[#R4]] 64
; CHECK-NEXT: r[[#R6]] = mul sp 1 64
; CHECK-NEXT: br .__sp.next[[#]]
; CHECK: .__sp.next[[#]]:
  %arraydecay = getelementptr inbounds [256 x i64], [256 x i64]* %temp, i64 0, i64 0
; CHECK-NEXT: r[[#R7:]] = add r32 0 64
; CHECK-NEXT: r[[#R8:]] = mul 0 2048 64
; CHECK-NEXT: r[[#R9:]] = add r[[#R7]] r[[#R8]] 64
; CHECK-NEXT: r[[#R10:]] = mul 0 8 64
; CHECK-NEXT: r[[#R11:]] = add r[[#R9]] r[[#R10]] 64
  call void @func(i64 %call, i64* %arraydecay)
; CHECK-NEXT: call func r[[#R4]] r[[#R11]]
  call void @free(i8* %call1)
; CHECK-NEXT: r[[#R12:]] = mul r[[#R6]] 1 64
; CHECK-NEXT: r[[#R13:]] = icmp ule r[[#R12]] 102400 64
; CHECK-NEXT: br r[[#R13]] ._defaultBB[[#BB3:]] ._defaultBB[[#BB4:]]
; CHECK: ._defaultBB[[#BB4]]:
; CHECK-NEXT: free r[[#R6]]
; CHECK-NEXT: br ._defaultBB[[#BB3]]
; CHECK: ._defaultBB[[#BB3]]:
; CHECK-NEXT: sp = add sp 2048 64
  ret i32 0
; CHECK-NEXT: ret 0
}
; CHECK: end main
