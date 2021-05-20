define i32 @main(i32 %n) {
;CHECK:		start main 1:

entry:
  br label %while.cond
;CHECK:		.entry:
;CHECK-NEXT:	[[regA:r[0-9]+]] = mul arg1 1 32
;CHECK-NEXT:	[[regB:r[0-9]+]] = mul 0 1 32
;CHECK-NEXT:	br .while.cond

while.cond:
  %n.addr.0 = phi i32 [ %n, %entry ], [ %shr, %while.body ]
  %count.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %tobool = icmp ne i32 %n.addr.0, 0
  br i1 %tobool, label %while.body, label %while.end
;CHECK:		.while.cond:
;CHECK-NEXT:	[[regC:r[0-9]+]] = icmp eq [[regA]] 0 32
;CHECK-NEXT:	br [[regC]] .while.end .while.body

while.body:
  %and = and i32 %n.addr.0, 1
  %add = add i32 %count.0, %and
  %shr = lshr i32 %n.addr.0, 1
  br label %while.cond
;CHECK:		.while.body:
;CHECK-NEXT:	[[regC]] = and [[regA]] 1 32
;CHECK-NEXT:	[[regD:r[0-9]+]] = add [[regB]] [[regC]] 32
;CHECK-NEXT:	[[regC]] = lshr [[regA]] 1 32
;CHECK-NEXT:	[[regB]] = mul [[regD]] 1 32
;CHECK-NEXT:	[[regA]] = mul [[regC]] 1 32
;CHECK-NEXT:	br .while.cond

while.end:
  ret i32 %count.0
;CHECK:		.while.end
;CHECK-NEXT:	ret [[regB]]
}
;CHECK:		end main