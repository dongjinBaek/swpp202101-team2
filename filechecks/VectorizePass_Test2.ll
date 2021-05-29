; ModuleID = '/tmp/a.ll'
source_filename = "filechecks/VectorizePass_Test2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
; Vectorizing stores after loop unrolling
; CHECK: vstore 8
entry:
  %call = call i64 (...) @read()
  %conv = trunc i64 %call to i32
  %mul = mul i32 %conv, %conv
  %conv1 = zext i32 %mul to i64
  %mul2 = mul i64 %conv1, 8
  %call3 = call noalias i8* @malloc(i64 %mul2) #4
  %0 = bitcast i8* %call3 to i64*
  br label %for.cond

for.cond:                                         ; preds = %for.inc11, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc12, %for.inc11 ]
  %cmp = icmp ult i32 %i.0, %conv
  br i1 %cmp, label %for.body, label %for.end13

for.body:                                         ; preds = %for.cond
  br label %for.cond5

for.cond5:                                        ; preds = %for.inc, %for.body
  %j.0 = phi i32 [ 0, %for.body ], [ %inc, %for.inc ]
  %cmp6 = icmp ult i32 %j.0, %conv
  br i1 %cmp6, label %for.body8, label %for.end

for.body8:                                        ; preds = %for.cond5
  %call9 = call i64 (...) @read()
  %mul10 = mul i32 %i.0, %conv
  %add = add i32 %mul10, %j.0
  %idxprom = zext i32 %add to i64
  %arrayidx = getelementptr inbounds i64, i64* %0, i64 %idxprom
  store i64 %call9, i64* %arrayidx, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body8
  %inc = add i32 %j.0, 1
  br label %for.cond5, !llvm.loop !2

for.end:                                          ; preds = %for.cond5
  br label %for.inc11

for.inc11:                                        ; preds = %for.end
  %inc12 = add i32 %i.0, 1
  br label %for.cond, !llvm.loop !5

for.end13:                                        ; preds = %for.cond
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

declare dso_local i64 @read(...) #2

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) #3

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git 4990141a4366eb00abdc8252d7cbb8adeacb9954)"}
!2 = distinct !{!2, !3, !4}
!3 = !{!"llvm.loop.mustprogress"}
!4 = !{!"llvm.loop.unroll.disable"}
!5 = distinct !{!5, !3, !4}
