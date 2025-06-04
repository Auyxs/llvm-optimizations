; ModuleID = 'test/bc/test.opt.bc'
source_filename = "test/cpp/test.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @_Z11bothGuardediii(i32 noundef %0, i32 noundef %1, i32 noundef %2) #0 {
  %4 = icmp slt i32 1, %2
  br i1 %4, label %.lr.ph, label %12

.lr.ph:                                           ; preds = %3
  br label %5

5:                                                ; preds = %.lr.ph, %9
  %.012 = phi i32 [ 1, %.lr.ph ], [ %10, %9 ]
  %.031 = phi i32 [ 0, %.lr.ph ], [ %6, %9 ]
  %.023 = phi i32 [ 0, %.lr.ph ], [ %8, %9 ]
  %6 = add nsw i32 %.031, %0
  br label %7

7:                                                ; preds = %5
  %8 = add nsw i32 %.023, %1
  br label %9

9:                                                ; preds = %7
  %10 = add nsw i32 %.012, 1
  %11 = icmp slt i32 %10, %2
  br i1 %11, label %5, label %._crit_edge7, !llvm.loop !6

._crit_edge7:                                     ; preds = %9
  %split8 = phi i32 [ %8, %9 ]
  %split = phi i32 [ %6, %9 ]
  br label %12

12:                                               ; preds = %3, %._crit_edge7
  %.02.lcssa = phi i32 [ %split8, %._crit_edge7 ], [ 0, %3 ]
  %.03.lcssa = phi i32 [ %split, %._crit_edge7 ], [ 0, %3 ]
  %13 = add nsw i32 %.03.lcssa, %.02.lcssa
  ret i32 %13
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @_Z14BothNotGuardedv() #0 {
  br label %1

1:                                                ; preds = %0, %10
  %.06 = phi i32 [ 0, %0 ], [ %8, %10 ]
  %.013 = phi i32 [ 0, %0 ], [ %9, %10 ]
  %.032 = phi i32 [ 0, %0 ], [ %5, %10 ]
  %.041 = phi i32 [ 1, %0 ], [ %11, %10 ]
  %2 = add nsw i32 %.06, 1
  %3 = add nsw i32 %.013, 2
  %4 = add nsw i32 %3, 3
  %5 = add nsw i32 %.032, 4
  br label %6

6:                                                ; preds = %1
  %7 = add nsw i32 %2, 10
  %8 = add nsw i32 %7, 20
  %9 = add nsw i32 %4, 30
  br label %10

10:                                               ; preds = %6
  %11 = add nsw i32 %.041, 1
  %12 = icmp slt i32 %11, 10
  br i1 %12, label %1, label %13, !llvm.loop !8

13:                                               ; preds = %10
  %.12.lcssa = phi i32 [ %9, %10 ]
  %.1.lcssa = phi i32 [ %8, %10 ]
  %.03.lcssa = phi i32 [ %5, %10 ]
  %14 = add nsw i32 %.1.lcssa, %.12.lcssa
  %15 = add nsw i32 %14, %.03.lcssa
  ret i32 %15
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @_Z12guardedWhileiii(i32 noundef %0, i32 noundef %1, i32 noundef %2) #0 {
  %4 = icmp sgt i32 %0, 0
  br i1 %4, label %5, label %15

5:                                                ; preds = %3
  br label %6

6:                                                ; preds = %12, %5
  %.01 = phi i32 [ %1, %5 ], [ %7, %12 ]
  %.0 = phi i32 [ 0, %5 ], [ %8, %12 ]
  %.02 = phi i32 [ %2, %5 ], [ %10, %12 ]
  %7 = add nsw i32 %.01, 5
  %8 = add nsw i32 %.0, 1
  br label %9

9:                                                ; preds = %6
  %10 = add nsw i32 %.02, 6
  %11 = add nsw i32 %.0, 1
  br label %12

12:                                               ; preds = %9
  %13 = icmp slt i32 %8, %0
  br i1 %13, label %6, label %14, !llvm.loop !9

14:                                               ; preds = %12
  %.lcssa = phi i32 [ %10, %12 ]
  br label %15

15:                                               ; preds = %3, %14
  %.13 = phi i32 [ %.lcssa, %14 ], [ %2, %3 ]
  ret i32 %.13
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local void @_Z5noDepPiS_i(ptr noundef %0, ptr noundef %1, i32 noundef %2) #0 {
  %4 = icmp slt i32 0, %2
  br i1 %4, label %.lr.ph, label %15

.lr.ph:                                           ; preds = %3
  br label %5

5:                                                ; preds = %.lr.ph, %12
  %.011 = phi i32 [ 0, %.lr.ph ], [ %13, %12 ]
  %6 = add nsw i32 %.011, 1
  %7 = sext i32 %6 to i64
  %8 = getelementptr inbounds i32, ptr %0, i64 %7
  store i32 0, ptr %8, align 4
  br label %9

9:                                                ; preds = %5
  %10 = sext i32 %.011 to i64
  %11 = getelementptr inbounds i32, ptr %1, i64 %10
  store i32 2, ptr %11, align 4
  br label %12

12:                                               ; preds = %9
  %13 = add nsw i32 %.011, 1
  %14 = icmp slt i32 %13, %2
  br i1 %14, label %5, label %._crit_edge5, !llvm.loop !10

._crit_edge5:                                     ; preds = %12
  br label %15

15:                                               ; preds = %3, %._crit_edge5
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
  br i1 %10, label %5, label %._crit_edge, !llvm.loop !11

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
  br i1 %23, label %13, label %._crit_edge5, !llvm.loop !12

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
