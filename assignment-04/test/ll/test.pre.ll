; ModuleID = 'test/bc/test.pre.bc'
source_filename = "test/cpp/test.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @_Z11bothGuardediii(i32 noundef %0, i32 noundef %1, i32 noundef %2) #0 {
  %4 = icmp slt i32 1, %2
  br i1 %4, label %.lr.ph, label %10

.lr.ph:                                           ; preds = %3
  br label %5

5:                                                ; preds = %.lr.ph, %7
  %.012 = phi i32 [ 1, %.lr.ph ], [ %8, %7 ]
  %.031 = phi i32 [ 0, %.lr.ph ], [ %6, %7 ]
  %6 = add nsw i32 %.031, %0
  br label %7

7:                                                ; preds = %5
  %8 = add nsw i32 %.012, 1
  %9 = icmp slt i32 %8, %2
  br i1 %9, label %5, label %._crit_edge, !llvm.loop !6

._crit_edge:                                      ; preds = %7
  %split = phi i32 [ %6, %7 ]
  br label %10

10:                                               ; preds = %._crit_edge, %3
  %.03.lcssa = phi i32 [ %split, %._crit_edge ], [ 0, %3 ]
  %11 = icmp slt i32 1, %2
  br i1 %11, label %.lr.ph6, label %17

.lr.ph6:                                          ; preds = %10
  br label %12

12:                                               ; preds = %.lr.ph6, %14
  %.04 = phi i32 [ 1, %.lr.ph6 ], [ %15, %14 ]
  %.023 = phi i32 [ 0, %.lr.ph6 ], [ %13, %14 ]
  %13 = add nsw i32 %.023, %1
  br label %14

14:                                               ; preds = %12
  %15 = add nsw i32 %.04, 1
  %16 = icmp slt i32 %15, %2
  br i1 %16, label %12, label %._crit_edge7, !llvm.loop !8

._crit_edge7:                                     ; preds = %14
  %split8 = phi i32 [ %13, %14 ]
  br label %17

17:                                               ; preds = %._crit_edge7, %10
  %.02.lcssa = phi i32 [ %split8, %._crit_edge7 ], [ 0, %10 ]
  %18 = add nsw i32 %.03.lcssa, %.02.lcssa
  ret i32 %18
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @_Z14BothNotGuardedv() #0 {
  br label %1

1:                                                ; preds = %0, %6
  %.06 = phi i32 [ 0, %0 ], [ %2, %6 ]
  %.013 = phi i32 [ 0, %0 ], [ %4, %6 ]
  %.032 = phi i32 [ 0, %0 ], [ %5, %6 ]
  %.041 = phi i32 [ 1, %0 ], [ %7, %6 ]
  %2 = add nsw i32 %.06, 1
  %3 = add nsw i32 %.013, 2
  %4 = add nsw i32 %3, 3
  %5 = add nsw i32 %.032, 4
  br label %6

6:                                                ; preds = %1
  %7 = add nsw i32 %.041, 1
  %8 = icmp slt i32 %7, 10
  br i1 %8, label %1, label %9, !llvm.loop !9

9:                                                ; preds = %6
  %.03.lcssa = phi i32 [ %5, %6 ]
  %.01.lcssa = phi i32 [ %4, %6 ]
  %.0.lcssa = phi i32 [ %2, %6 ]
  br label %10

10:                                               ; preds = %9, %14
  %.19 = phi i32 [ %.0.lcssa, %9 ], [ %12, %14 ]
  %.128 = phi i32 [ %.01.lcssa, %9 ], [ %13, %14 ]
  %.057 = phi i32 [ 1, %9 ], [ %15, %14 ]
  %11 = add nsw i32 %.19, 10
  %12 = add nsw i32 %11, 20
  %13 = add nsw i32 %.128, 30
  br label %14

14:                                               ; preds = %10
  %15 = add nsw i32 %.057, 1
  %16 = icmp slt i32 %15, 10
  br i1 %16, label %10, label %17, !llvm.loop !10

17:                                               ; preds = %14
  %.12.lcssa = phi i32 [ %13, %14 ]
  %.1.lcssa = phi i32 [ %12, %14 ]
  %18 = add nsw i32 %.1.lcssa, %.12.lcssa
  %19 = add nsw i32 %18, %.03.lcssa
  ret i32 %19
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @_Z12guardedWhileiii(i32 noundef %0, i32 noundef %1, i32 noundef %2) #0 {
  %4 = icmp sgt i32 %0, 0
  br i1 %4, label %5, label %12

5:                                                ; preds = %3
  br label %6

6:                                                ; preds = %9, %5
  %.01 = phi i32 [ %1, %5 ], [ %7, %9 ]
  %.0 = phi i32 [ 0, %5 ], [ %8, %9 ]
  %7 = add nsw i32 %.01, 5
  %8 = add nsw i32 %.0, 1
  br label %9

9:                                                ; preds = %6
  %10 = icmp slt i32 %8, %0
  br i1 %10, label %6, label %11, !llvm.loop !11

11:                                               ; preds = %9
  br label %12

12:                                               ; preds = %11, %3
  %13 = icmp sgt i32 %0, 0
  br i1 %13, label %14, label %21

14:                                               ; preds = %12
  br label %15

15:                                               ; preds = %18, %14
  %.02 = phi i32 [ %2, %14 ], [ %16, %18 ]
  %.1 = phi i32 [ 0, %14 ], [ %17, %18 ]
  %16 = add nsw i32 %.02, 6
  %17 = add nsw i32 %.1, 1
  br label %18

18:                                               ; preds = %15
  %19 = icmp slt i32 %17, %0
  br i1 %19, label %15, label %20, !llvm.loop !12

20:                                               ; preds = %18
  %.lcssa = phi i32 [ %16, %18 ]
  br label %21

21:                                               ; preds = %20, %12
  %.13 = phi i32 [ %.lcssa, %20 ], [ %2, %12 ]
  ret i32 %.13
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local void @_Z5noDepPiS_i(ptr noundef %0, ptr noundef %1, i32 noundef %2) #0 {
  %4 = icmp slt i32 0, %2
  br i1 %4, label %.lr.ph, label %12

.lr.ph:                                           ; preds = %3
  br label %5

5:                                                ; preds = %.lr.ph, %9
  %.011 = phi i32 [ 0, %.lr.ph ], [ %10, %9 ]
  %6 = add nsw i32 %.011, 1
  %7 = sext i32 %6 to i64
  %8 = getelementptr inbounds i32, ptr %0, i64 %7
  store i32 0, ptr %8, align 4
  br label %9

9:                                                ; preds = %5
  %10 = add nsw i32 %.011, 1
  %11 = icmp slt i32 %10, %2
  br i1 %11, label %5, label %._crit_edge, !llvm.loop !13

._crit_edge:                                      ; preds = %9
  br label %12

12:                                               ; preds = %._crit_edge, %3
  %13 = icmp slt i32 0, %2
  br i1 %13, label %.lr.ph4, label %20

.lr.ph4:                                          ; preds = %12
  br label %14

14:                                               ; preds = %.lr.ph4, %17
  %.02 = phi i32 [ 0, %.lr.ph4 ], [ %18, %17 ]
  %15 = sext i32 %.02 to i64
  %16 = getelementptr inbounds i32, ptr %1, i64 %15
  store i32 2, ptr %16, align 4
  br label %17

17:                                               ; preds = %14
  %18 = add nsw i32 %.02, 1
  %19 = icmp slt i32 %18, %2
  br i1 %19, label %14, label %._crit_edge5, !llvm.loop !14

._crit_edge5:                                     ; preds = %17
  br label %20

20:                                               ; preds = %._crit_edge5, %12
  ret void
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local void @_Z3DepPiS_i(ptr noundef %0, ptr noundef %1, i32 noundef %2) #0 {
  %4 = icmp slt i32 0, %2
  br i1 %4, label %.lr.ph, label %11

.lr.ph:                                           ; preds = %3
  br label %5

5:                                                ; preds = %.lr.ph, %8
  %.011 = phi i32 [ 0, %.lr.ph ], [ %9, %8 ]
  %6 = sext i32 %.011 to i64
  %7 = getelementptr inbounds i32, ptr %0, i64 %6
  store i32 0, ptr %7, align 4
  br label %8

8:                                                ; preds = %5
  %9 = add nsw i32 %.011, 1
  %10 = icmp slt i32 %9, %2
  br i1 %10, label %5, label %._crit_edge, !llvm.loop !15

._crit_edge:                                      ; preds = %8
  br label %11

11:                                               ; preds = %._crit_edge, %3
  %12 = icmp slt i32 0, %2
  br i1 %12, label %.lr.ph4, label %24

.lr.ph4:                                          ; preds = %11
  br label %13

13:                                               ; preds = %.lr.ph4, %21
  %.02 = phi i32 [ 0, %.lr.ph4 ], [ %22, %21 ]
  %14 = add nsw i32 %.02, 3
  %15 = sext i32 %14 to i64
  %16 = getelementptr inbounds i32, ptr %0, i64 %15
  %17 = load i32, ptr %16, align 4
  %18 = add nsw i32 %17, 1
  %19 = sext i32 %.02 to i64
  %20 = getelementptr inbounds i32, ptr %1, i64 %19
  store i32 %18, ptr %20, align 4
  br label %21

21:                                               ; preds = %13
  %22 = add nsw i32 %.02, 1
  %23 = icmp slt i32 %22, %2
  br i1 %23, label %13, label %._crit_edge5, !llvm.loop !16

._crit_edge5:                                     ; preds = %21
  br label %24

24:                                               ; preds = %._crit_edge5, %11
  ret void
}

attributes #0 = { mustprogress noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 19.1.7 (++20250114103320+cd708029e0b2-1~exp1~20250114103432.75)"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
!9 = distinct !{!9, !7}
!10 = distinct !{!10, !7}
!11 = distinct !{!11, !7}
!12 = distinct !{!12, !7}
!13 = distinct !{!13, !7}
!14 = distinct !{!14, !7}
!15 = distinct !{!15, !7}
!16 = distinct !{!16, !7}
