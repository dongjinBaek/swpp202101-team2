define dso_local void @matmul(i32 %dim, i64* %c, i64* %a, i64* %b) #0 {
entry:
  br label %for.cond

for.cond:
  %i.0 = phi i32 [ 0, %entry ], [ %inc21, %for.inc20 ]
  %cmp = icmp ult i32 %i.0, %dim
  br i1 %cmp, label %for.body, label %for.end22

for.body:
  br label %for.cond1

for.cond1:
  %j.0 = phi i32 [ 0, %for.body ], [ %inc18, %for.inc17 ]
  %cmp2 = icmp ult i32 %j.0, %dim
  br i1 %cmp2, label %for.body3, label %for.end19

for.body3:
  br label %for.cond4

for.cond4:
  %k.0 = phi i32 [ 0, %for.body3 ], [ %inc, %for.inc ]
  %cmp5 = icmp ult i32 %k.0, %dim
  br i1 %cmp5, label %for.body6, label %for.end
;CHECK: .for.cond4:
for.body6:
  %mul = mul i32 %i.0, %dim
  %add = add i32 %mul, %k.0
  %idxprom = zext i32 %add to i64
  %arrayidx = getelementptr inbounds i64, i64* %a, i64 %idxprom
  %0 = load i64, i64* %arrayidx, align 8
  %mul7 = mul i32 %k.0, %dim
  %add8 = add i32 %mul7, %j.0
  %idxprom9 = zext i32 %add8 to i64
  %arrayidx10 = getelementptr inbounds i64, i64* %b, i64 %idxprom9
  %1 = load i64, i64* %arrayidx10, align 8
  %mul11 = mul i64 %0, %1
  %mul12 = mul i32 %i.0, %dim
  %add13 = add i32 %mul12, %j.0
  %idxprom14 = zext i32 %add13 to i64
  %arrayidx15 = getelementptr inbounds i64, i64* %c, i64 %idxprom14
  %2 = load i64, i64* %arrayidx15, align 8
  %add16 = add i64 %2, %mul11
  store i64 %add16, i64* %arrayidx15, align 8
  br label %for.inc

for.inc:
  %inc = add i32 %k.0, 1
  br label %for.cond4, !llvm.loop !4

for.end:
  br label %for.inc17
;CHECK: .for.end:
;CHECK: store

for.inc17:
  %inc18 = add i32 %j.0, 1
  br label %for.cond1, !llvm.loop !7
;CHECK: .for.inc17

for.end19:
  br label %for.inc20

for.inc20:
  %inc21 = add i32 %i.0, 1
  br label %for.cond, !llvm.loop !8

for.end22:
  ret void
}
!4 = distinct !{!4, !5, !6}
!5 = !{!"llvm.loop.mustprogress"}
!6 = !{!"llvm.loop.unroll.disable"}
!7 = distinct !{!7, !5, !6}
!8 = distinct !{!8, !5, !6}