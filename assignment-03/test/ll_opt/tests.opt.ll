; ModuleID = 'test/bc/tests.opt.bc'
source_filename = "test/cpp/tests.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @_Z5test1ib(i32 noundef %0, i1 noundef zeroext %1) #0 {
  %3 = zext i1 %1 to i8
  %4 = mul nsw i32 %0, 2
  br label %5

5:                                                ; preds = %8, %2
  %6 = trunc i8 %3 to i1
  br i1 %6, label %7, label %8

7:                                                ; preds = %5
  br label %9

8:                                                ; preds = %5
  br label %5, !llvm.loop !6

9:                                                ; preds = %7
  ret i32 %4
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local void @_Z5test2ib(i32 noundef %0, i1 noundef zeroext %1) #0 {
  %3 = zext i1 %1 to i8
  %4 = mul nsw i32 %0, 2
  %5 = add nsw i32 %4, 1
  %6 = mul nsw i32 %5, 2
  br label %7

7:                                                ; preds = %10, %2
  %8 = trunc i8 %3 to i1
  br i1 %8, label %9, label %10

9:                                                ; preds = %7
  br label %11

10:                                               ; preds = %7
  br label %7, !llvm.loop !8

11:                                               ; preds = %9
  ret void
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local void @_Z5test3b(i1 noundef zeroext %0) #0 {
  %2 = zext i1 %0 to i8
  br label %3

3:                                                ; preds = %11, %1
  %.02 = phi i32 [ 0, %1 ], [ %12, %11 ]
  %.01 = phi i32 [ 0, %1 ], [ %6, %11 ]
  %.0 = phi i32 [ undef, %1 ], [ %.1, %11 ]
  %4 = icmp slt i32 %.02, 20
  br i1 %4, label %5, label %13

5:                                                ; preds = %3
  %6 = add nsw i32 %.01, %.0
  %7 = trunc i8 %2 to i1
  br i1 %7, label %8, label %10

8:                                                ; preds = %5
  %9 = mul nsw i32 10, 2
  br label %10

10:                                               ; preds = %8, %5
  %.1 = phi i32 [ %9, %8 ], [ %.0, %5 ]
  br label %11

11:                                               ; preds = %10
  %12 = add nsw i32 %.02, 1
  br label %3, !llvm.loop !9

13:                                               ; preds = %3
  ret void
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local void @_Z5test4i(i32 noundef %0) #0 {
  br label %2

2:                                                ; preds = %12, %1
  %.0 = phi i32 [ 0, %1 ], [ %13, %12 ]
  %3 = icmp slt i32 %.0, 20
  br i1 %3, label %4, label %14

4:                                                ; preds = %2
  %5 = icmp eq i32 %.0, 4
  br i1 %5, label %6, label %8

6:                                                ; preds = %4
  %7 = add nsw i32 3, %0
  br label %10

8:                                                ; preds = %4
  %9 = add nsw i32 4, %0
  br label %10

10:                                               ; preds = %8, %6
  %.01 = phi i32 [ %7, %6 ], [ %9, %8 ]
  %11 = mul nsw i32 %.01, 3
  br label %12

12:                                               ; preds = %10
  %13 = add nsw i32 %.0, 1
  br label %2, !llvm.loop !10

14:                                               ; preds = %2
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
