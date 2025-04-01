; ModuleID = 'test/bc/multi_instr.opt.bc'
source_filename = "test/cpp/multi_instr.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local void @_Z15test_multiInstri(i32 noundef %0) #0 {
  %2 = add nsw i32 %0, 1
  %3 = add nsw i32 1, %0
  %4 = mul nsw i32 %0, %0
  %5 = sub nsw i32 %0, 1
  %6 = mul nsw i32 %0, %0
  %7 = sub nsw i32 1, %0
  %8 = mul nsw i32 %0, %0
  ret void
}

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local void @_Z17test_noMultiInstri(i32 noundef %0) #0 {
  %2 = add nsw i32 %0, 1
  %3 = add nsw i32 1, %0
  %4 = add nsw i32 %2, 1
  %5 = sub nsw i32 1, %3
  %6 = mul nsw i32 %5, %4
  %7 = sub nsw i32 %0, 1
  %8 = sub nsw i32 %7, 1
  %9 = sub nsw i32 1, %0
  %10 = add nsw i32 1, %9
  %11 = mul nsw i32 %8, %10
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
