; ModuleID = 'test/bc/tests.pre.bc'
source_filename = "test/cpp/tests.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @_Z5test1i(i32 noundef %0) #0 {
  br label %2

2:                                                ; preds = %1, %4
  %.02 = phi i32 [ 0, %1 ], [ %5, %4 ]
  %3 = mul nsw i32 %0, 2
  br label %4

4:                                                ; preds = %2
  %5 = add nsw i32 %.02, 1
  %6 = icmp slt i32 %5, 20
  br i1 %6, label %2, label %7, !llvm.loop !6

7:                                                ; preds = %4
  %.01.lcssa = phi i32 [ %3, %4 ]
  ret i32 %.01.lcssa
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local noundef i32 @_Z5test2i(i32 noundef %0) #0 {
  br label %2

2:                                                ; preds = %1, %6
  %.01 = phi i32 [ 0, %1 ], [ %7, %6 ]
  %3 = mul nsw i32 %0, 2
  %4 = add nsw i32 %3, 1
  %5 = mul nsw i32 %4, 2
  br label %6

6:                                                ; preds = %2
  %7 = add nsw i32 %.01, 1
  %8 = icmp slt i32 %7, 20
  br i1 %8, label %2, label %9, !llvm.loop !8

9:                                                ; preds = %6
  ret i32 0
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local void @_Z5test3b(i1 noundef zeroext %0) #0 {
  %2 = zext i1 %0 to i8
  br label %3

3:                                                ; preds = %1, %9
  %.03 = phi i32 [ 0, %1 ], [ %10, %9 ]
  %.012 = phi i32 [ 0, %1 ], [ %4, %9 ]
  %.021 = phi i32 [ undef, %1 ], [ %.1, %9 ]
  %4 = add nsw i32 %.012, %.021
  %5 = trunc i8 %2 to i1
  br i1 %5, label %6, label %8

6:                                                ; preds = %3
  %7 = mul nsw i32 10, 2
  br label %8

8:                                                ; preds = %6, %3
  %.1 = phi i32 [ %7, %6 ], [ %.021, %3 ]
  br label %9

9:                                                ; preds = %8
  %10 = add nsw i32 %.03, 1
  %11 = icmp slt i32 %10, 20
  br i1 %11, label %3, label %12, !llvm.loop !9

12:                                               ; preds = %9
  ret void
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local void @_Z5test4i(i32 noundef %0) #0 {
  br label %2

2:                                                ; preds = %1, %10
  %.02 = phi i32 [ 0, %1 ], [ %11, %10 ]
  %3 = icmp eq i32 %.02, 4
  br i1 %3, label %4, label %6

4:                                                ; preds = %2
  %5 = add nsw i32 3, %0
  br label %8

6:                                                ; preds = %2
  %7 = add nsw i32 4, %0
  br label %8

8:                                                ; preds = %6, %4
  %.01 = phi i32 [ %5, %4 ], [ %7, %6 ]
  %9 = mul nsw i32 %.01, 3
  br label %10

10:                                               ; preds = %8
  %11 = add nsw i32 %.02, 1
  %12 = icmp slt i32 %11, 20
  br i1 %12, label %2, label %13, !llvm.loop !10

13:                                               ; preds = %10
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
