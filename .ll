source_filename = "test"
target datalayout = "e-m:e-p:64:64-i64:64-f80:128-n8:16:32:64-S128"

@global_var_4018 = global i64 0
@global_var_3fe0 = local_unnamed_addr global i64 0
@global_var_2004 = constant [15 x i8] c"Hello Abdullah\00"
@0 = external global i32

define i64 @_init() local_unnamed_addr {
dec_label_pc_1000:
  %rax.0.reg2mem = alloca i64, !insn.addr !0
  %0 = load i64, i64* inttoptr (i64 16336 to i64*), align 16, !insn.addr !1
  %1 = icmp eq i64 %0, 0, !insn.addr !2
  store i64 0, i64* %rax.0.reg2mem, !insn.addr !3
  br i1 %1, label %dec_label_pc_1016, label %dec_label_pc_1014, !insn.addr !3

dec_label_pc_1014:                                ; preds = %dec_label_pc_1000
  call void @__gmon_start__(), !insn.addr !4
  store i64 ptrtoint (i32* @0 to i64), i64* %rax.0.reg2mem, !insn.addr !4
  br label %dec_label_pc_1016, !insn.addr !4

dec_label_pc_1016:                                ; preds = %dec_label_pc_1014, %dec_label_pc_1000
  %rax.0.reload = load i64, i64* %rax.0.reg2mem
  ret i64 %rax.0.reload, !insn.addr !5
}

define i32 @function_1030(i8* %s) local_unnamed_addr {
dec_label_pc_1030:
  %0 = call i32 @puts(i8* %s), !insn.addr !6
  ret i32 %0, !insn.addr !6
}

define i64 @_start(i64 %arg1, i64 %arg2, i64 %arg3, i64 %arg4, i64 %arg5, i64 %arg6) local_unnamed_addr {
dec_label_pc_1040:
  %stack_var_8 = alloca i64, align 8
  %0 = trunc i64 %arg6 to i32, !insn.addr !7
  %1 = bitcast i64* %stack_var_8 to i8**, !insn.addr !7
  %2 = inttoptr i64 %arg3 to void ()*, !insn.addr !7
  %3 = call i32 @__libc_start_main(i64 4409, i32 %0, i8** nonnull %1, void ()* null, void ()* null, void ()* %2), !insn.addr !7
  %4 = call i64 @__asm_hlt(), !insn.addr !8
  unreachable, !insn.addr !8
}

define i64 @function_1070() local_unnamed_addr {
dec_label_pc_1070:
  ret i64 ptrtoint (i64* @global_var_4018 to i64), !insn.addr !9
}

define i64 @function_10a0() local_unnamed_addr {
dec_label_pc_10a0:
  ret i64 0, !insn.addr !10
}

define i64 @function_10e0() local_unnamed_addr {
dec_label_pc_10e0:
  %0 = alloca i64
  %1 = load i64, i64* %0
  %2 = load i8, i8* bitcast (i64* @global_var_4018 to i8*), align 8, !insn.addr !11
  %3 = icmp eq i8 %2, 0, !insn.addr !11
  %4 = icmp eq i1 %3, false, !insn.addr !12
  br i1 %4, label %dec_label_pc_1120, label %dec_label_pc_10ed, !insn.addr !12

dec_label_pc_10ed:                                ; preds = %dec_label_pc_10e0
  %5 = load i64, i64* @global_var_3fe0, align 8, !insn.addr !13
  %6 = icmp eq i64 %5, 0, !insn.addr !13
  br i1 %6, label %dec_label_pc_1108, label %dec_label_pc_10fb, !insn.addr !14

dec_label_pc_10fb:                                ; preds = %dec_label_pc_10ed
  %7 = load i64, i64* inttoptr (i64 16400 to i64*), align 16, !insn.addr !15
  %8 = inttoptr i64 %7 to i64*, !insn.addr !16
  call void @__cxa_finalize(i64* %8), !insn.addr !16
  br label %dec_label_pc_1108, !insn.addr !16

dec_label_pc_1108:                                ; preds = %dec_label_pc_10fb, %dec_label_pc_10ed
  %9 = call i64 @function_1070(), !insn.addr !17
  store i8 1, i8* bitcast (i64* @global_var_4018 to i8*), align 8, !insn.addr !18
  ret i64 %9, !insn.addr !19

dec_label_pc_1120:                                ; preds = %dec_label_pc_10e0
  ret i64 %1, !insn.addr !20
}

define i64 @function_1130() local_unnamed_addr {
dec_label_pc_1130:
  %0 = call i64 @function_10a0(), !insn.addr !21
  ret i64 %0, !insn.addr !21
}

define i64 @main(i64 %argc, i8** %argv) local_unnamed_addr {
dec_label_pc_1139:
  %0 = call i32 @puts(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @global_var_2004, i64 0, i64 0)), !insn.addr !22
  ret i64 0, !insn.addr !23

; uselistorder directives
  uselistorder i64 0, { 1, 3, 4, 7, 2, 0, 8, 5, 6 }
}

define i64 @_fini() local_unnamed_addr {
dec_label_pc_1160:
  %0 = alloca i64
  %1 = load i64, i64* %0
  ret i64 %1, !insn.addr !24

; uselistorder directives
  uselistorder i32 1, { 1, 0, 3, 2 }
}

declare i32 @__libc_start_main(i64, i32, i8**, void ()*, void ()*, void ()*) local_unnamed_addr

declare void @__gmon_start__() local_unnamed_addr

declare void @__cxa_finalize(i64*) local_unnamed_addr

declare i32 @puts(i8*) local_unnamed_addr

declare i64 @__asm_hlt() local_unnamed_addr

!0 = !{i64 4096}
!1 = !{i64 4104}
!2 = !{i64 4111}
!3 = !{i64 4114}
!4 = !{i64 4116}
!5 = !{i64 4122}
!6 = !{i64 4144}
!7 = !{i64 4191}
!8 = !{i64 4197}
!9 = !{i64 4248}
!10 = !{i64 4312}
!11 = !{i64 4324}
!12 = !{i64 4331}
!13 = !{i64 4334}
!14 = !{i64 4345}
!15 = !{i64 4347}
!16 = !{i64 4354}
!17 = !{i64 4360}
!18 = !{i64 4365}
!19 = !{i64 4373}
!20 = !{i64 4384}
!21 = !{i64 4404}
!22 = !{i64 4434}
!23 = !{i64 4445}
!24 = !{i64 4460}
