define i32 @main(i64 %idx) {
; CHECK: start main 1:
entry:
  %count = alloca [3 x i32], align 16
  br label %tmp
;CHECK: .entry:
;CHECK: .splitted
;CHECK: switch
;CHECK: .splitted_bb
;CHECK: r[[#]] = mul r[[#]] 1 32
;CHECK: br .tmp
;CHECK: .splitted_bb
;CHECK: r[[#]] = mul r[[#]] 1 32
;CHECK: br .tmp 
;CHECK: .splitted_bb
;CHECK: r[[#]] = mul r[[#]] 1 32
;CHECK: br .tmp
tmp:
  %arrayidx = getelementptr inbounds [3 x i32], [3 x i32]* %count, i64 0, i64 %idx
  %0 = load i32, i32* %arrayidx, align 4
;CHECK: .tmp:
  ret i32 0
}
