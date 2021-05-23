; ModuleID = '/tmp/a.ll'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
  ;CHECK:          start main 0
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
  %cmp = icmp ugt i64 %call15, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  call void @write(i64 %call15)
  ret i32 -1

if.end:                                           ; preds = %entry
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
  call void @write(i64 %call31)
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

declare dso_local i64 @read(...) #2

declare dso_local void @write(i64) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git 4990141a4366eb00abdc8252d7cbb8adeacb9954)"}
