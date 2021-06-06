define i32 @main(i64 %idx, i32 %a) {
; CHECK: start main 2:
entry:
  %count = alloca [3 x i32], align 16
  br label %tmp
;CHECK: .entry:
;CHECK: .splitted
;CHECK: switch
;CHECK: .splitted_bb
;CHECK: r[[#]] = mul 1 arg2 32
;CHECK: br .tmp
;CHECK: .splitted_bb
;CHECK: r[[#]] = mul 1 arg2 32 
;CHECK: br .tmp
;CHECK: .splitted_bb
;CHECK: r[[#]] = mul 1 arg2 32
;CHECK: br .tmp
tmp:
  %arrayidx = getelementptr inbounds [3 x i32], [3 x i32]* %count, i64 0, i64 %idx
  store i32 %a, i32* %arrayidx, align 4
;CHECK: .tmp:
  ret i32 0
}
