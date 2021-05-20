; ModuleID = '/tmp/a.ll'
source_filename = "filechecks/VectorizePass_Test3.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
; Vectorizing stores when another loads inside them
; Not vectorizing loads/store with another unknown difference loads/stores
; CHECK: vstore 4
; CHECK-NOT: vload 4
; CHECK-NOT: vstore 4
; CHECK: vstore 4
entry:
  %call = call noalias i8* @malloc(i64 64) #4
  %0 = bitcast i8* %call to i64*
  %call1 = call noalias i8* @malloc(i64 32) #4
  %1 = bitcast i8* %call1 to i64*
  %arrayidx = getelementptr inbounds i64, i64* %0, i64 0
  store i64 1, i64* %arrayidx, align 8
  %arrayidx2 = getelementptr inbounds i64, i64* %0, i64 1
  store i64 1, i64* %arrayidx2, align 8
  %arrayidx3 = getelementptr inbounds i64, i64* %0, i64 2
  store i64 1, i64* %arrayidx3, align 8
  %arrayidx4 = getelementptr inbounds i64, i64* %0, i64 3
  store i64 1, i64* %arrayidx4, align 8
  %arrayidx5 = getelementptr inbounds i64, i64* %0, i64 0
  %2 = load i64, i64* %arrayidx5, align 8
  %arrayidx6 = getelementptr inbounds i64, i64* %1, i64 0
  store i64 %2, i64* %arrayidx6, align 8
  %arrayidx7 = getelementptr inbounds i64, i64* %0, i64 1
  %3 = load i64, i64* %arrayidx7, align 8
  %arrayidx8 = getelementptr inbounds i64, i64* %1, i64 1
  store i64 %3, i64* %arrayidx8, align 8
  %arrayidx9 = getelementptr inbounds i64, i64* %0, i64 2
  %4 = load i64, i64* %arrayidx9, align 8
  %arrayidx10 = getelementptr inbounds i64, i64* %1, i64 2
  store i64 %4, i64* %arrayidx10, align 8
  %arrayidx11 = getelementptr inbounds i64, i64* %0, i64 3
  %5 = load i64, i64* %arrayidx11, align 8
  %arrayidx12 = getelementptr inbounds i64, i64* %1, i64 3
  store i64 %5, i64* %arrayidx12, align 8
  %arrayidx13 = getelementptr inbounds i64, i64* %0, i64 4
  store i64 1, i64* %arrayidx13, align 8
  %arrayidx14 = getelementptr inbounds i64, i64* %0, i64 5
  store i64 1, i64* %arrayidx14, align 8
  %arrayidx15 = getelementptr inbounds i64, i64* %0, i64 6
  store i64 1, i64* %arrayidx15, align 8
  %arrayidx16 = getelementptr inbounds i64, i64* %0, i64 7
  store i64 1, i64* %arrayidx16, align 8
  %arrayidx17 = getelementptr inbounds i64, i64* %1, i64 0
  %6 = load i64, i64* %arrayidx17, align 8
  call void @write(i64 %6)
  %arrayidx18 = getelementptr inbounds i64, i64* %1, i64 1
  %7 = load i64, i64* %arrayidx18, align 8
  call void @write(i64 %7)
  %arrayidx19 = getelementptr inbounds i64, i64* %1, i64 2
  %8 = load i64, i64* %arrayidx19, align 8
  call void @write(i64 %8)
  %arrayidx20 = getelementptr inbounds i64, i64* %1, i64 3
  %9 = load i64, i64* %arrayidx20, align 8
  call void @write(i64 %9)
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) #2

declare dso_local void @write(i64) #3

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git 4990141a4366eb00abdc8252d7cbb8adeacb9954)"}
