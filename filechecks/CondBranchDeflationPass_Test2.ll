define i32 @main(i32 %n) {
;CHECK:		start main 1:

entry:
  %tobool = icmp sgt i32 %n, 0
  br i1 %tobool, label %if.then, label %if.else
;CHECK:		.entry:
;CHECK:		switch

if.then:
  ret i32 1

if.else:
  ret i32 0
}