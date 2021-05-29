define i32 @main(i32 %n) {
;CHECK:		start main 1:

entry:
  br label %while.cond
;CHECK:		.entry:

while.cond:
  %n.addr.0 = phi i32 [ %n, %entry ], [ %shr, %while.body ]
  %count.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %tobool = icmp ne i32 %n.addr.0, 0
  br i1 %tobool, label %while.body, label %while.end
;CHECK:		icmp eq

while.body:
  %and = and i32 %n.addr.0, 1
  %add = add i32 %count.0, %and
  %shr = lshr i32 %n.addr.0, 1
  br label %while.cond

while.end:
  ret i32 %count.0
}