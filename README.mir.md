    $rax = LEA64r $rip, 1, $noreg, %bb.1, $noreg
    PUSH64r $rax, implicit-def $rsp, implicit $rsp
    TAILJMPd64 @printf, csr_win64, implicit $rsp, implicit $ssp, implicit $rsp, implicit $ssp
    TAILJMPd64 @printf, csr_win64, implicit $rsp, implicit $ssp, implicit $rcx, implicit $edx, implicit-def $rsp, implicit-def $ssp, implicit-def $eax
  bb.1:

    o 

    $rax = LEA64r $rip, 1, $noreg, %bb.1, $noreg
    PUSH64r $rax, implicit-def $rsp, implicit $rsp

    $rax =  LEA64r $rip, 1, $noreg, @printf, $noreg
    PUSH64r $rax, implicit-def $rsp, implicit $rsp
    RET64 $noreg
    
llc --mtriple=x86_64-pc-windows-msvc -stop-after=x86-isel hello.bc -o hello.mir
llc --mtriple=x86_64-pc-windows-gnu -start-after=x86-isel -stop-after=irtranslator hello.mir -o hello.late.mir  
llc --mtriple=x86_64-pc-windows-msvc -start-after=irtranslator hello.late.mir -filetype=asm -o hello.s --x86-asm-syntax=intel 