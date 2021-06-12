define dso_local i64 @InlineFunction() #0 {
entry:
  %call = call i64 (...) @read()
  ret i64 %call
}

declare dso_local i64 @read(...) #1

define dso_local i64 @RecursiveFunction(i64 %level) #0 {
entry:
  %tobool = icmp ne i64 %level, 0
  br i1 %tobool, label %cond.true, label %cond.false

cond.true:
  %sub = sub i64 %level, 1
  %call = call i64 @RecursiveFunction(i64 %sub)
  %call1 = call i64 (...) @read()
  %add = add i64 %call, %call1
  br label %cond.end

cond.false:
  %call2 = call i64 (...) @read()
  br label %cond.end

cond.end:
  %cond = phi i64 [ %add, %cond.true ], [ %call2, %cond.false ]
  ret i64 %cond
}

define dso_local i64 @ManyColorFunction() #0 {
entry:
  %call = call i64 (...) @read()
  %call1 = call i64 (...) @read()
  %call2 = call i64 (...) @read()
  %call3 = call i64 (...) @read()
  %call4 = call i64 (...) @read()
  %call5 = call i64 (...) @read()
  %call6 = call i64 (...) @read()
  %call7 = call i64 (...) @read()
  %call8 = call i64 (...) @read()
  %call9 = call i64 (...) @read()
  %call10 = call i64 (...) @read()
  %call11 = call i64 (...) @read()
  %call12 = call i64 (...) @read()
  %call13 = call i64 (...) @read()
  %call14 = call i64 (...) @read()
  %call15 = call i64 (...) @read()
  %call16 = call i64 (...) @read()
  %call17 = call i64 (...) @read()
  %call18 = call i64 (...) @read()
  %call19 = call i64 (...) @read()
  %call20 = call i64 (...) @read()
  %call21 = call i64 (...) @read()
  %call22 = call i64 (...) @read()
  %call23 = call i64 (...) @read()
  %call24 = call i64 (...) @read()
  %call25 = call i64 (...) @read()
  %call26 = call i64 (...) @read()
  %call27 = call i64 (...) @read()
  %call28 = call i64 (...) @read()
  %call29 = call i64 (...) @read()
  %call30 = call i64 (...) @read()
  %call31 = call i64 (...) @read()
  %add = add i64 %call, %call1
  %add32 = add i64 %add, %call2
  %add33 = add i64 %add32, %call3
  %add34 = add i64 %add33, %call4
  %add35 = add i64 %add34, %call5
  %add36 = add i64 %add35, %call6
  %add37 = add i64 %add36, %call7
  %add38 = add i64 %add37, %call8
  %add39 = add i64 %add38, %call9
  %add40 = add i64 %add39, %call10
  %add41 = add i64 %add40, %call11
  %add42 = add i64 %add41, %call12
  %add43 = add i64 %add42, %call13
  %add44 = add i64 %add43, %call14
  %add45 = add i64 %add44, %call15
  %add46 = add i64 %add45, %call16
  %add47 = add i64 %add46, %call17
  %add48 = add i64 %add47, %call18
  %add49 = add i64 %add48, %call19
  %add50 = add i64 %add49, %call20
  %add51 = add i64 %add50, %call21
  %add52 = add i64 %add51, %call22
  %add53 = add i64 %add52, %call23
  %add54 = add i64 %add53, %call24
  %add55 = add i64 %add54, %call25
  %add56 = add i64 %add55, %call26
  %add57 = add i64 %add56, %call27
  %add58 = add i64 %add57, %call28
  %add59 = add i64 %add58, %call29
  %add60 = add i64 %add59, %call30
  %add61 = add i64 %add60, %call31
  ret i64 %add61
}

define dso_local i32 @main() #0 {
; CHECK: start main 0:
entry:
; CHECK: .entry:
  %call = call i64 @InlineFunction()
  ; CHECK-NEXT: r[[#R1:]] = call read
  call void @write(i64 %call)
  ; CHECK-NEXT: call write r[[#R1]]
  %call1 = call i64 @RecursiveFunction(i64 32)
  ; CHECK-NEXT: r[[#R2:]] = call RecursiveFunction 32
  call void @write(i64 %call1)
  ; CHECK-NEXT: call write r[[#R2]]
  %call2 = call i64 @ManyColorFunction()
  ; CHECK-NEXT: r[[#R3:]] = call ManyColorFunction
  call void @write(i64 %call2)
  ; CHECK-NEXT: call write r[[#R3]]
  ret i32 0
  ; CHECK-NEXT: ret 0
}
; CHECK-NEXT: end main

declare dso_local void @write(i64) #1
