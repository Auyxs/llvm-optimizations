; ModuleID = 'test/bc/test.bc'
source_filename = "test/cpp/test.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @_Z11bothGuardediii(i32 noundef %0, i32 noundef %1, i32 noundef %2) #0 {
  br label %4

4:                                                ; preds = %8, %3
  %.03 = phi i32 [ 0, %3 ], [ %7, %8 ]
  %.01 = phi i32 [ 1, %3 ], [ %9, %8 ]
  %5 = icmp slt i32 %.01, %2
  br i1 %5, label %6, label %10

6:                                                ; preds = %4
  %7 = add nsw i32 %.03, %0
  br label %8

8:                                                ; preds = %6
  %9 = add nsw i32 %.01, 1
  br label %4, !llvm.loop !6

10:                                               ; preds = %4
  br label %11

11:                                               ; preds = %15, %10
  %.02 = phi i32 [ 0, %10 ], [ %14, %15 ]
  %.0 = phi i32 [ 1, %10 ], [ %16, %15 ]
  %12 = icmp slt i32 %.0, %2
  br i1 %12, label %13, label %17

13:                                               ; preds = %11
  %14 = add nsw i32 %.02, %1
  br label %15

15:                                               ; preds = %13
  %16 = add nsw i32 %.0, 1
  br label %11, !llvm.loop !8

17:                                               ; preds = %11
  %18 = add nsw i32 %.03, %.02
  ret i32 %18
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @_Z14BothNotGuardedv() #0 {
  br label %1

1:                                                ; preds = %8, %0
  %.04 = phi i32 [ 1, %0 ], [ %9, %8 ]
  %.03 = phi i32 [ 0, %0 ], [ %7, %8 ]
  %.01 = phi i32 [ 0, %0 ], [ %6, %8 ]
  %.0 = phi i32 [ 0, %0 ], [ %4, %8 ]
  %2 = icmp slt i32 %.04, 10
  br i1 %2, label %3, label %10

3:                                                ; preds = %1
  %4 = add nsw i32 %.0, 1
  %5 = add nsw i32 %.01, 2
  %6 = add nsw i32 %5, 3
  %7 = add nsw i32 %.03, 4
  br label %8

8:                                                ; preds = %3
  %9 = add nsw i32 %.04, 1
  br label %1, !llvm.loop !9

10:                                               ; preds = %1
  br label %11

11:                                               ; preds = %17, %10
  %.05 = phi i32 [ 1, %10 ], [ %18, %17 ]
  %.12 = phi i32 [ %.01, %10 ], [ %16, %17 ]
  %.1 = phi i32 [ %.0, %10 ], [ %15, %17 ]
  %12 = icmp slt i32 %.05, 10
  br i1 %12, label %13, label %19

13:                                               ; preds = %11
  %14 = add nsw i32 %.1, 10
  %15 = add nsw i32 %14, 20
  %16 = add nsw i32 %.12, 30
  br label %17

17:                                               ; preds = %13
  %18 = add nsw i32 %.05, 1
  br label %11, !llvm.loop !10

19:                                               ; preds = %11
  %20 = add nsw i32 %.1, %.12
  %21 = add nsw i32 %20, %.03
  ret i32 %21
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
  br label %21

21:                                               ; preds = %20, %12
  %.13 = phi i32 [ %16, %20 ], [ %2, %12 ]
  ret i32 %.13
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local void @_Z5noDepPiS_i(ptr noundef %0, ptr noundef %1, i32 noundef %2) #0 {
  br label %4

4:                                                ; preds = %10, %3
  %.01 = phi i32 [ 0, %3 ], [ %11, %10 ]
  %5 = icmp slt i32 %.01, %2
  br i1 %5, label %6, label %12

6:                                                ; preds = %4
  %7 = add nsw i32 %.01, 1
  %8 = sext i32 %7 to i64
  %9 = getelementptr inbounds i32, ptr %0, i64 %8
  store i32 0, ptr %9, align 4
  br label %10

10:                                               ; preds = %6
  %11 = add nsw i32 %.01, 1
  br label %4, !llvm.loop !13

12:                                               ; preds = %4
  br label %13

13:                                               ; preds = %18, %12
  %.0 = phi i32 [ 0, %12 ], [ %19, %18 ]
  %14 = icmp slt i32 %.0, %2
  br i1 %14, label %15, label %20

15:                                               ; preds = %13
  %16 = sext i32 %.0 to i64
  %17 = getelementptr inbounds i32, ptr %1, i64 %16
  store i32 2, ptr %17, align 4
  br label %18

18:                                               ; preds = %15
  %19 = add nsw i32 %.0, 1
  br label %13, !llvm.loop !14

20:                                               ; preds = %13
  ret void
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local void @_Z3DepPiS_i(ptr noundef %0, ptr noundef %1, i32 noundef %2) #0 {
  br label %4

4:                                                ; preds = %9, %3
  %.01 = phi i32 [ 0, %3 ], [ %10, %9 ]
  %5 = icmp slt i32 %.01, %2
  br i1 %5, label %6, label %11

6:                                                ; preds = %4
  %7 = sext i32 %.01 to i64
  %8 = getelementptr inbounds i32, ptr %0, i64 %7
  store i32 0, ptr %8, align 4
  br label %9

9:                                                ; preds = %6
  %10 = add nsw i32 %.01, 1
  br label %4, !llvm.loop !15

11:                                               ; preds = %4
  br label %12

12:                                               ; preds = %22, %11
  %.0 = phi i32 [ 0, %11 ], [ %23, %22 ]
  %13 = icmp slt i32 %.0, %2
  br i1 %13, label %14, label %24

14:                                               ; preds = %12
  %15 = add nsw i32 %.0, 3
  %16 = sext i32 %15 to i64
  %17 = getelementptr inbounds i32, ptr %0, i64 %16
  %18 = load i32, ptr %17, align 4
  %19 = add nsw i32 %18, 1
  %20 = sext i32 %.0 to i64
  %21 = getelementptr inbounds i32, ptr %1, i64 %20
  store i32 %19, ptr %21, align 4
  br label %22

22:                                               ; preds = %14
  %23 = add nsw i32 %.0, 1
  br label %12, !llvm.loop !16

24:                                               ; preds = %12
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
