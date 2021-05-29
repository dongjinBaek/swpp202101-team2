; ModuleID = '/tmp/a.ll'
source_filename = "filechecks/VectorizePass_Test1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
; Vectorizing already unrolled loads and stores
; Code from matmul3
; CHECK: vload 4
; CHECK: vstore 4
entry:
  %mul = mul nsw i32 100, 100
  %conv = sext i32 %mul to i64
  %mul1 = mul i64 %conv, 8
  %call = call noalias i8* @malloc(i64 %mul1) #3
  %0 = bitcast i8* %call to i64*
  %mul2 = mul nsw i32 100, 100
  %conv3 = sext i32 %mul2 to i64
  %mul4 = mul i64 %conv3, 8
  %call5 = call noalias i8* @malloc(i64 %mul4) #3
  %1 = bitcast i8* %call5 to i64*
  %mul6 = mul nsw i32 100, 100
  %conv7 = sext i32 %mul6 to i64
  %mul8 = mul i64 %conv7, 8
  %call9 = call noalias i8* @malloc(i64 %mul8) #3
  %2 = bitcast i8* %call9 to i64*
  br label %for.cond

for.cond:                                         ; preds = %for.inc438, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %add439, %for.inc438 ]
  %cmp = icmp slt i32 %i.0, 100
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  br label %for.end440

for.body:                                         ; preds = %for.cond
  br label %for.cond11

for.cond11:                                       ; preds = %for.inc435, %for.body
  %j.0 = phi i32 [ 0, %for.body ], [ %add436, %for.inc435 ]
  %cmp12 = icmp slt i32 %j.0, 100
  br i1 %cmp12, label %for.body15, label %for.cond.cleanup14

for.cond.cleanup14:                               ; preds = %for.cond11
  br label %for.end437

for.body15:                                       ; preds = %for.cond11
  br label %for.cond16

for.cond16:                                       ; preds = %for.inc, %for.body15
  %k.0 = phi i32 [ 0, %for.body15 ], [ %add434, %for.inc ]
  %cmp17 = icmp slt i32 %k.0, 100
  br i1 %cmp17, label %for.body20, label %for.cond.cleanup19

for.cond.cleanup19:                               ; preds = %for.cond16
  br label %for.end

for.body20:                                       ; preds = %for.cond16
  %add = add nsw i32 %i.0, 0
  %mul21 = mul nsw i32 %add, 100
  %add22 = add nsw i32 %mul21, %k.0
  %add23 = add nsw i32 %add22, 0
  %idxprom = sext i32 %add23 to i64
  %arrayidx = getelementptr inbounds i64, i64* %0, i64 %idxprom
  %3 = load i64, i64* %arrayidx, align 8
  %add24 = add nsw i32 %i.0, 0
  %mul25 = mul nsw i32 %add24, 100
  %add26 = add nsw i32 %mul25, %k.0
  %add27 = add nsw i32 %add26, 1
  %idxprom28 = sext i32 %add27 to i64
  %arrayidx29 = getelementptr inbounds i64, i64* %0, i64 %idxprom28
  %4 = load i64, i64* %arrayidx29, align 8
  %add30 = add nsw i32 %i.0, 0
  %mul31 = mul nsw i32 %add30, 100
  %add32 = add nsw i32 %mul31, %k.0
  %add33 = add nsw i32 %add32, 2
  %idxprom34 = sext i32 %add33 to i64
  %arrayidx35 = getelementptr inbounds i64, i64* %0, i64 %idxprom34
  %5 = load i64, i64* %arrayidx35, align 8
  %add36 = add nsw i32 %i.0, 0
  %mul37 = mul nsw i32 %add36, 100
  %add38 = add nsw i32 %mul37, %k.0
  %add39 = add nsw i32 %add38, 3
  %idxprom40 = sext i32 %add39 to i64
  %arrayidx41 = getelementptr inbounds i64, i64* %0, i64 %idxprom40
  %6 = load i64, i64* %arrayidx41, align 8
  %add42 = add nsw i32 %i.0, 1
  %mul43 = mul nsw i32 %add42, 100
  %add44 = add nsw i32 %mul43, %k.0
  %add45 = add nsw i32 %add44, 0
  %idxprom46 = sext i32 %add45 to i64
  %arrayidx47 = getelementptr inbounds i64, i64* %0, i64 %idxprom46
  %7 = load i64, i64* %arrayidx47, align 8
  %add48 = add nsw i32 %i.0, 1
  %mul49 = mul nsw i32 %add48, 100
  %add50 = add nsw i32 %mul49, %k.0
  %add51 = add nsw i32 %add50, 1
  %idxprom52 = sext i32 %add51 to i64
  %arrayidx53 = getelementptr inbounds i64, i64* %0, i64 %idxprom52
  %8 = load i64, i64* %arrayidx53, align 8
  %add54 = add nsw i32 %i.0, 1
  %mul55 = mul nsw i32 %add54, 100
  %add56 = add nsw i32 %mul55, %k.0
  %add57 = add nsw i32 %add56, 2
  %idxprom58 = sext i32 %add57 to i64
  %arrayidx59 = getelementptr inbounds i64, i64* %0, i64 %idxprom58
  %9 = load i64, i64* %arrayidx59, align 8
  %add60 = add nsw i32 %i.0, 1
  %mul61 = mul nsw i32 %add60, 100
  %add62 = add nsw i32 %mul61, %k.0
  %add63 = add nsw i32 %add62, 3
  %idxprom64 = sext i32 %add63 to i64
  %arrayidx65 = getelementptr inbounds i64, i64* %0, i64 %idxprom64
  %10 = load i64, i64* %arrayidx65, align 8
  %add66 = add nsw i32 %i.0, 2
  %mul67 = mul nsw i32 %add66, 100
  %add68 = add nsw i32 %mul67, %k.0
  %add69 = add nsw i32 %add68, 0
  %idxprom70 = sext i32 %add69 to i64
  %arrayidx71 = getelementptr inbounds i64, i64* %0, i64 %idxprom70
  %11 = load i64, i64* %arrayidx71, align 8
  %add72 = add nsw i32 %i.0, 2
  %mul73 = mul nsw i32 %add72, 100
  %add74 = add nsw i32 %mul73, %k.0
  %add75 = add nsw i32 %add74, 1
  %idxprom76 = sext i32 %add75 to i64
  %arrayidx77 = getelementptr inbounds i64, i64* %0, i64 %idxprom76
  %12 = load i64, i64* %arrayidx77, align 8
  %add78 = add nsw i32 %i.0, 2
  %mul79 = mul nsw i32 %add78, 100
  %add80 = add nsw i32 %mul79, %k.0
  %add81 = add nsw i32 %add80, 2
  %idxprom82 = sext i32 %add81 to i64
  %arrayidx83 = getelementptr inbounds i64, i64* %0, i64 %idxprom82
  %13 = load i64, i64* %arrayidx83, align 8
  %add84 = add nsw i32 %i.0, 2
  %mul85 = mul nsw i32 %add84, 100
  %add86 = add nsw i32 %mul85, %k.0
  %add87 = add nsw i32 %add86, 3
  %idxprom88 = sext i32 %add87 to i64
  %arrayidx89 = getelementptr inbounds i64, i64* %0, i64 %idxprom88
  %14 = load i64, i64* %arrayidx89, align 8
  %add90 = add nsw i32 %i.0, 3
  %mul91 = mul nsw i32 %add90, 100
  %add92 = add nsw i32 %mul91, %k.0
  %add93 = add nsw i32 %add92, 0
  %idxprom94 = sext i32 %add93 to i64
  %arrayidx95 = getelementptr inbounds i64, i64* %0, i64 %idxprom94
  %15 = load i64, i64* %arrayidx95, align 8
  %add96 = add nsw i32 %i.0, 3
  %mul97 = mul nsw i32 %add96, 100
  %add98 = add nsw i32 %mul97, %k.0
  %add99 = add nsw i32 %add98, 1
  %idxprom100 = sext i32 %add99 to i64
  %arrayidx101 = getelementptr inbounds i64, i64* %0, i64 %idxprom100
  %16 = load i64, i64* %arrayidx101, align 8
  %add102 = add nsw i32 %i.0, 3
  %mul103 = mul nsw i32 %add102, 100
  %add104 = add nsw i32 %mul103, %k.0
  %add105 = add nsw i32 %add104, 2
  %idxprom106 = sext i32 %add105 to i64
  %arrayidx107 = getelementptr inbounds i64, i64* %0, i64 %idxprom106
  %17 = load i64, i64* %arrayidx107, align 8
  %add108 = add nsw i32 %i.0, 3
  %mul109 = mul nsw i32 %add108, 100
  %add110 = add nsw i32 %mul109, %k.0
  %add111 = add nsw i32 %add110, 3
  %idxprom112 = sext i32 %add111 to i64
  %arrayidx113 = getelementptr inbounds i64, i64* %0, i64 %idxprom112
  %18 = load i64, i64* %arrayidx113, align 8
  %add114 = add nsw i32 %k.0, 0
  %mul115 = mul nsw i32 %add114, 100
  %add116 = add nsw i32 %mul115, %j.0
  %add117 = add nsw i32 %add116, 0
  %idxprom118 = sext i32 %add117 to i64
  %arrayidx119 = getelementptr inbounds i64, i64* %1, i64 %idxprom118
  %19 = load i64, i64* %arrayidx119, align 8
  %add120 = add nsw i32 %k.0, 0
  %mul121 = mul nsw i32 %add120, 100
  %add122 = add nsw i32 %mul121, %j.0
  %add123 = add nsw i32 %add122, 1
  %idxprom124 = sext i32 %add123 to i64
  %arrayidx125 = getelementptr inbounds i64, i64* %1, i64 %idxprom124
  %20 = load i64, i64* %arrayidx125, align 8
  %add126 = add nsw i32 %k.0, 0
  %mul127 = mul nsw i32 %add126, 100
  %add128 = add nsw i32 %mul127, %j.0
  %add129 = add nsw i32 %add128, 2
  %idxprom130 = sext i32 %add129 to i64
  %arrayidx131 = getelementptr inbounds i64, i64* %1, i64 %idxprom130
  %21 = load i64, i64* %arrayidx131, align 8
  %add132 = add nsw i32 %k.0, 0
  %mul133 = mul nsw i32 %add132, 100
  %add134 = add nsw i32 %mul133, %j.0
  %add135 = add nsw i32 %add134, 3
  %idxprom136 = sext i32 %add135 to i64
  %arrayidx137 = getelementptr inbounds i64, i64* %1, i64 %idxprom136
  %22 = load i64, i64* %arrayidx137, align 8
  %add138 = add nsw i32 %k.0, 1
  %mul139 = mul nsw i32 %add138, 100
  %add140 = add nsw i32 %mul139, %j.0
  %add141 = add nsw i32 %add140, 0
  %idxprom142 = sext i32 %add141 to i64
  %arrayidx143 = getelementptr inbounds i64, i64* %1, i64 %idxprom142
  %23 = load i64, i64* %arrayidx143, align 8
  %add144 = add nsw i32 %k.0, 1
  %mul145 = mul nsw i32 %add144, 100
  %add146 = add nsw i32 %mul145, %j.0
  %add147 = add nsw i32 %add146, 1
  %idxprom148 = sext i32 %add147 to i64
  %arrayidx149 = getelementptr inbounds i64, i64* %1, i64 %idxprom148
  %24 = load i64, i64* %arrayidx149, align 8
  %add150 = add nsw i32 %k.0, 1
  %mul151 = mul nsw i32 %add150, 100
  %add152 = add nsw i32 %mul151, %j.0
  %add153 = add nsw i32 %add152, 2
  %idxprom154 = sext i32 %add153 to i64
  %arrayidx155 = getelementptr inbounds i64, i64* %1, i64 %idxprom154
  %25 = load i64, i64* %arrayidx155, align 8
  %add156 = add nsw i32 %k.0, 1
  %mul157 = mul nsw i32 %add156, 100
  %add158 = add nsw i32 %mul157, %j.0
  %add159 = add nsw i32 %add158, 3
  %idxprom160 = sext i32 %add159 to i64
  %arrayidx161 = getelementptr inbounds i64, i64* %1, i64 %idxprom160
  %26 = load i64, i64* %arrayidx161, align 8
  %add162 = add nsw i32 %k.0, 2
  %mul163 = mul nsw i32 %add162, 100
  %add164 = add nsw i32 %mul163, %j.0
  %add165 = add nsw i32 %add164, 0
  %idxprom166 = sext i32 %add165 to i64
  %arrayidx167 = getelementptr inbounds i64, i64* %1, i64 %idxprom166
  %27 = load i64, i64* %arrayidx167, align 8
  %add168 = add nsw i32 %k.0, 2
  %mul169 = mul nsw i32 %add168, 100
  %add170 = add nsw i32 %mul169, %j.0
  %add171 = add nsw i32 %add170, 1
  %idxprom172 = sext i32 %add171 to i64
  %arrayidx173 = getelementptr inbounds i64, i64* %1, i64 %idxprom172
  %28 = load i64, i64* %arrayidx173, align 8
  %add174 = add nsw i32 %k.0, 2
  %mul175 = mul nsw i32 %add174, 100
  %add176 = add nsw i32 %mul175, %j.0
  %add177 = add nsw i32 %add176, 2
  %idxprom178 = sext i32 %add177 to i64
  %arrayidx179 = getelementptr inbounds i64, i64* %1, i64 %idxprom178
  %29 = load i64, i64* %arrayidx179, align 8
  %add180 = add nsw i32 %k.0, 2
  %mul181 = mul nsw i32 %add180, 100
  %add182 = add nsw i32 %mul181, %j.0
  %add183 = add nsw i32 %add182, 3
  %idxprom184 = sext i32 %add183 to i64
  %arrayidx185 = getelementptr inbounds i64, i64* %1, i64 %idxprom184
  %30 = load i64, i64* %arrayidx185, align 8
  %add186 = add nsw i32 %k.0, 3
  %mul187 = mul nsw i32 %add186, 100
  %add188 = add nsw i32 %mul187, %j.0
  %add189 = add nsw i32 %add188, 0
  %idxprom190 = sext i32 %add189 to i64
  %arrayidx191 = getelementptr inbounds i64, i64* %1, i64 %idxprom190
  %31 = load i64, i64* %arrayidx191, align 8
  %add192 = add nsw i32 %k.0, 3
  %mul193 = mul nsw i32 %add192, 100
  %add194 = add nsw i32 %mul193, %j.0
  %add195 = add nsw i32 %add194, 1
  %idxprom196 = sext i32 %add195 to i64
  %arrayidx197 = getelementptr inbounds i64, i64* %1, i64 %idxprom196
  %32 = load i64, i64* %arrayidx197, align 8
  %add198 = add nsw i32 %k.0, 3
  %mul199 = mul nsw i32 %add198, 100
  %add200 = add nsw i32 %mul199, %j.0
  %add201 = add nsw i32 %add200, 2
  %idxprom202 = sext i32 %add201 to i64
  %arrayidx203 = getelementptr inbounds i64, i64* %1, i64 %idxprom202
  %33 = load i64, i64* %arrayidx203, align 8
  %add204 = add nsw i32 %k.0, 3
  %mul205 = mul nsw i32 %add204, 100
  %add206 = add nsw i32 %mul205, %j.0
  %add207 = add nsw i32 %add206, 3
  %idxprom208 = sext i32 %add207 to i64
  %arrayidx209 = getelementptr inbounds i64, i64* %1, i64 %idxprom208
  %34 = load i64, i64* %arrayidx209, align 8
  %mul210 = mul i64 %3, %19
  %mul211 = mul i64 %4, %23
  %add212 = add i64 %mul210, %mul211
  %mul213 = mul i64 %5, %27
  %add214 = add i64 %add212, %mul213
  %mul215 = mul i64 %6, %31
  %add216 = add i64 %add214, %mul215
  %add217 = add nsw i32 %i.0, 0
  %mul218 = mul nsw i32 %add217, 100
  %add219 = add nsw i32 %mul218, %j.0
  %add220 = add nsw i32 %add219, 0
  %idxprom221 = sext i32 %add220 to i64
  %arrayidx222 = getelementptr inbounds i64, i64* %2, i64 %idxprom221
  %35 = load i64, i64* %arrayidx222, align 8
  %add223 = add i64 %35, %add216
  store i64 %add223, i64* %arrayidx222, align 8
  %mul224 = mul i64 %3, %20
  %mul225 = mul i64 %4, %24
  %add226 = add i64 %mul224, %mul225
  %mul227 = mul i64 %5, %28
  %add228 = add i64 %add226, %mul227
  %mul229 = mul i64 %6, %32
  %add230 = add i64 %add228, %mul229
  %add231 = add nsw i32 %i.0, 0
  %mul232 = mul nsw i32 %add231, 100
  %add233 = add nsw i32 %mul232, %j.0
  %add234 = add nsw i32 %add233, 1
  %idxprom235 = sext i32 %add234 to i64
  %arrayidx236 = getelementptr inbounds i64, i64* %2, i64 %idxprom235
  %36 = load i64, i64* %arrayidx236, align 8
  %add237 = add i64 %36, %add230
  store i64 %add237, i64* %arrayidx236, align 8
  %mul238 = mul i64 %3, %21
  %mul239 = mul i64 %4, %25
  %add240 = add i64 %mul238, %mul239
  %mul241 = mul i64 %5, %29
  %add242 = add i64 %add240, %mul241
  %mul243 = mul i64 %6, %33
  %add244 = add i64 %add242, %mul243
  %add245 = add nsw i32 %i.0, 0
  %mul246 = mul nsw i32 %add245, 100
  %add247 = add nsw i32 %mul246, %j.0
  %add248 = add nsw i32 %add247, 2
  %idxprom249 = sext i32 %add248 to i64
  %arrayidx250 = getelementptr inbounds i64, i64* %2, i64 %idxprom249
  %37 = load i64, i64* %arrayidx250, align 8
  %add251 = add i64 %37, %add244
  store i64 %add251, i64* %arrayidx250, align 8
  %mul252 = mul i64 %3, %22
  %mul253 = mul i64 %4, %26
  %add254 = add i64 %mul252, %mul253
  %mul255 = mul i64 %5, %30
  %add256 = add i64 %add254, %mul255
  %mul257 = mul i64 %6, %34
  %add258 = add i64 %add256, %mul257
  %add259 = add nsw i32 %i.0, 0
  %mul260 = mul nsw i32 %add259, 100
  %add261 = add nsw i32 %mul260, %j.0
  %add262 = add nsw i32 %add261, 3
  %idxprom263 = sext i32 %add262 to i64
  %arrayidx264 = getelementptr inbounds i64, i64* %2, i64 %idxprom263
  %38 = load i64, i64* %arrayidx264, align 8
  %add265 = add i64 %38, %add258
  store i64 %add265, i64* %arrayidx264, align 8
  %mul266 = mul i64 %7, %19
  %mul267 = mul i64 %8, %23
  %add268 = add i64 %mul266, %mul267
  %mul269 = mul i64 %9, %27
  %add270 = add i64 %add268, %mul269
  %mul271 = mul i64 %10, %31
  %add272 = add i64 %add270, %mul271
  %add273 = add nsw i32 %i.0, 1
  %mul274 = mul nsw i32 %add273, 100
  %add275 = add nsw i32 %mul274, %j.0
  %add276 = add nsw i32 %add275, 0
  %idxprom277 = sext i32 %add276 to i64
  %arrayidx278 = getelementptr inbounds i64, i64* %2, i64 %idxprom277
  %39 = load i64, i64* %arrayidx278, align 8
  %add279 = add i64 %39, %add272
  store i64 %add279, i64* %arrayidx278, align 8
  %mul280 = mul i64 %7, %20
  %mul281 = mul i64 %8, %24
  %add282 = add i64 %mul280, %mul281
  %mul283 = mul i64 %9, %28
  %add284 = add i64 %add282, %mul283
  %mul285 = mul i64 %10, %32
  %add286 = add i64 %add284, %mul285
  %add287 = add nsw i32 %i.0, 1
  %mul288 = mul nsw i32 %add287, 100
  %add289 = add nsw i32 %mul288, %j.0
  %add290 = add nsw i32 %add289, 1
  %idxprom291 = sext i32 %add290 to i64
  %arrayidx292 = getelementptr inbounds i64, i64* %2, i64 %idxprom291
  %40 = load i64, i64* %arrayidx292, align 8
  %add293 = add i64 %40, %add286
  store i64 %add293, i64* %arrayidx292, align 8
  %mul294 = mul i64 %7, %21
  %mul295 = mul i64 %8, %25
  %add296 = add i64 %mul294, %mul295
  %mul297 = mul i64 %9, %29
  %add298 = add i64 %add296, %mul297
  %mul299 = mul i64 %10, %33
  %add300 = add i64 %add298, %mul299
  %add301 = add nsw i32 %i.0, 1
  %mul302 = mul nsw i32 %add301, 100
  %add303 = add nsw i32 %mul302, %j.0
  %add304 = add nsw i32 %add303, 2
  %idxprom305 = sext i32 %add304 to i64
  %arrayidx306 = getelementptr inbounds i64, i64* %2, i64 %idxprom305
  %41 = load i64, i64* %arrayidx306, align 8
  %add307 = add i64 %41, %add300
  store i64 %add307, i64* %arrayidx306, align 8
  %mul308 = mul i64 %7, %22
  %mul309 = mul i64 %8, %26
  %add310 = add i64 %mul308, %mul309
  %mul311 = mul i64 %9, %30
  %add312 = add i64 %add310, %mul311
  %mul313 = mul i64 %10, %34
  %add314 = add i64 %add312, %mul313
  %add315 = add nsw i32 %i.0, 1
  %mul316 = mul nsw i32 %add315, 100
  %add317 = add nsw i32 %mul316, %j.0
  %add318 = add nsw i32 %add317, 3
  %idxprom319 = sext i32 %add318 to i64
  %arrayidx320 = getelementptr inbounds i64, i64* %2, i64 %idxprom319
  %42 = load i64, i64* %arrayidx320, align 8
  %add321 = add i64 %42, %add314
  store i64 %add321, i64* %arrayidx320, align 8
  %mul322 = mul i64 %11, %19
  %mul323 = mul i64 %12, %23
  %add324 = add i64 %mul322, %mul323
  %mul325 = mul i64 %13, %27
  %add326 = add i64 %add324, %mul325
  %mul327 = mul i64 %14, %31
  %add328 = add i64 %add326, %mul327
  %add329 = add nsw i32 %i.0, 2
  %mul330 = mul nsw i32 %add329, 100
  %add331 = add nsw i32 %mul330, %j.0
  %add332 = add nsw i32 %add331, 0
  %idxprom333 = sext i32 %add332 to i64
  %arrayidx334 = getelementptr inbounds i64, i64* %2, i64 %idxprom333
  %43 = load i64, i64* %arrayidx334, align 8
  %add335 = add i64 %43, %add328
  store i64 %add335, i64* %arrayidx334, align 8
  %mul336 = mul i64 %11, %20
  %mul337 = mul i64 %12, %24
  %add338 = add i64 %mul336, %mul337
  %mul339 = mul i64 %13, %28
  %add340 = add i64 %add338, %mul339
  %mul341 = mul i64 %14, %32
  %add342 = add i64 %add340, %mul341
  %add343 = add nsw i32 %i.0, 2
  %mul344 = mul nsw i32 %add343, 100
  %add345 = add nsw i32 %mul344, %j.0
  %add346 = add nsw i32 %add345, 1
  %idxprom347 = sext i32 %add346 to i64
  %arrayidx348 = getelementptr inbounds i64, i64* %2, i64 %idxprom347
  %44 = load i64, i64* %arrayidx348, align 8
  %add349 = add i64 %44, %add342
  store i64 %add349, i64* %arrayidx348, align 8
  %mul350 = mul i64 %11, %21
  %mul351 = mul i64 %12, %25
  %add352 = add i64 %mul350, %mul351
  %mul353 = mul i64 %13, %29
  %add354 = add i64 %add352, %mul353
  %mul355 = mul i64 %14, %33
  %add356 = add i64 %add354, %mul355
  %add357 = add nsw i32 %i.0, 2
  %mul358 = mul nsw i32 %add357, 100
  %add359 = add nsw i32 %mul358, %j.0
  %add360 = add nsw i32 %add359, 2
  %idxprom361 = sext i32 %add360 to i64
  %arrayidx362 = getelementptr inbounds i64, i64* %2, i64 %idxprom361
  %45 = load i64, i64* %arrayidx362, align 8
  %add363 = add i64 %45, %add356
  store i64 %add363, i64* %arrayidx362, align 8
  %mul364 = mul i64 %11, %22
  %mul365 = mul i64 %12, %26
  %add366 = add i64 %mul364, %mul365
  %mul367 = mul i64 %13, %30
  %add368 = add i64 %add366, %mul367
  %mul369 = mul i64 %14, %34
  %add370 = add i64 %add368, %mul369
  %add371 = add nsw i32 %i.0, 2
  %mul372 = mul nsw i32 %add371, 100
  %add373 = add nsw i32 %mul372, %j.0
  %add374 = add nsw i32 %add373, 3
  %idxprom375 = sext i32 %add374 to i64
  %arrayidx376 = getelementptr inbounds i64, i64* %2, i64 %idxprom375
  %46 = load i64, i64* %arrayidx376, align 8
  %add377 = add i64 %46, %add370
  store i64 %add377, i64* %arrayidx376, align 8
  %mul378 = mul i64 %15, %19
  %mul379 = mul i64 %16, %23
  %add380 = add i64 %mul378, %mul379
  %mul381 = mul i64 %17, %27
  %add382 = add i64 %add380, %mul381
  %mul383 = mul i64 %18, %31
  %add384 = add i64 %add382, %mul383
  %add385 = add nsw i32 %i.0, 3
  %mul386 = mul nsw i32 %add385, 100
  %add387 = add nsw i32 %mul386, %j.0
  %add388 = add nsw i32 %add387, 0
  %idxprom389 = sext i32 %add388 to i64
  %arrayidx390 = getelementptr inbounds i64, i64* %2, i64 %idxprom389
  %47 = load i64, i64* %arrayidx390, align 8
  %add391 = add i64 %47, %add384
  store i64 %add391, i64* %arrayidx390, align 8
  %mul392 = mul i64 %15, %20
  %mul393 = mul i64 %16, %24
  %add394 = add i64 %mul392, %mul393
  %mul395 = mul i64 %17, %28
  %add396 = add i64 %add394, %mul395
  %mul397 = mul i64 %18, %32
  %add398 = add i64 %add396, %mul397
  %add399 = add nsw i32 %i.0, 3
  %mul400 = mul nsw i32 %add399, 100
  %add401 = add nsw i32 %mul400, %j.0
  %add402 = add nsw i32 %add401, 1
  %idxprom403 = sext i32 %add402 to i64
  %arrayidx404 = getelementptr inbounds i64, i64* %2, i64 %idxprom403
  %48 = load i64, i64* %arrayidx404, align 8
  %add405 = add i64 %48, %add398
  store i64 %add405, i64* %arrayidx404, align 8
  %mul406 = mul i64 %15, %21
  %mul407 = mul i64 %16, %25
  %add408 = add i64 %mul406, %mul407
  %mul409 = mul i64 %17, %29
  %add410 = add i64 %add408, %mul409
  %mul411 = mul i64 %18, %33
  %add412 = add i64 %add410, %mul411
  %add413 = add nsw i32 %i.0, 3
  %mul414 = mul nsw i32 %add413, 100
  %add415 = add nsw i32 %mul414, %j.0
  %add416 = add nsw i32 %add415, 2
  %idxprom417 = sext i32 %add416 to i64
  %arrayidx418 = getelementptr inbounds i64, i64* %2, i64 %idxprom417
  %49 = load i64, i64* %arrayidx418, align 8
  %add419 = add i64 %49, %add412
  store i64 %add419, i64* %arrayidx418, align 8
  %mul420 = mul i64 %15, %22
  %mul421 = mul i64 %16, %26
  %add422 = add i64 %mul420, %mul421
  %mul423 = mul i64 %17, %30
  %add424 = add i64 %add422, %mul423
  %mul425 = mul i64 %18, %34
  %add426 = add i64 %add424, %mul425
  %add427 = add nsw i32 %i.0, 3
  %mul428 = mul nsw i32 %add427, 100
  %add429 = add nsw i32 %mul428, %j.0
  %add430 = add nsw i32 %add429, 3
  %idxprom431 = sext i32 %add430 to i64
  %arrayidx432 = getelementptr inbounds i64, i64* %2, i64 %idxprom431
  %50 = load i64, i64* %arrayidx432, align 8
  %add433 = add i64 %50, %add426
  store i64 %add433, i64* %arrayidx432, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body20
  %add434 = add nsw i32 %k.0, 4
  br label %for.cond16, !llvm.loop !2

for.end:                                          ; preds = %for.cond.cleanup19
  br label %for.inc435

for.inc435:                                       ; preds = %for.end
  %add436 = add nsw i32 %j.0, 4
  br label %for.cond11, !llvm.loop !5

for.end437:                                       ; preds = %for.cond.cleanup14
  br label %for.inc438

for.inc438:                                       ; preds = %for.end437
  %add439 = add nsw i32 %i.0, 4
  br label %for.cond, !llvm.loop !6

for.end440:                                       ; preds = %for.cond.cleanup
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare dso_local noalias i8* @malloc(i64) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 12.0.0 (https://github.com/llvm/llvm-project.git 4990141a4366eb00abdc8252d7cbb8adeacb9954)"}
!2 = distinct !{!2, !3, !4}
!3 = !{!"llvm.loop.mustprogress"}
!4 = !{!"llvm.loop.unroll.disable"}
!5 = distinct !{!5, !3, !4}
!6 = distinct !{!6, !3, !4}
