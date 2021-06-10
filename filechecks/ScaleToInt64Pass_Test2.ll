; ModuleID = '/tmp/a.ll'
source_filename = "filechecks/Int32To64Pass_Test2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@gvarray = external dso_local global [17 x i32], align 16

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
; i32 load/store with global variable
; Check doubled global variable size
; Check doubled offset
; CHECK: main
; CHECK: 136
; CHECK: mul 3 2
entry:
  store i32 2, i32* getelementptr inbounds ([17 x i32], [17 x i32]* @gvarray, i64 0, i64 3), align 4
  %0 = load i32, i32* getelementptr inbounds ([17 x i32], [17 x i32]* @gvarray, i64 0, i64 3), align 4
  %conv = sext i32 %0 to i64
  call void @write(i64 %conv)
  ret i32 0
}

declare dso_local void @write(i64) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git 4990141a4366eb00abdc8252d7cbb8adeacb9954)"}
