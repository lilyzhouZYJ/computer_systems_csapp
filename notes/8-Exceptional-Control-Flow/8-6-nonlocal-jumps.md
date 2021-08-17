# 8.6 Nonlocal Jumps

- **Nonlocal jump** is a form of *user-level* exceptional control flow, which transfers control directly from one function to another without going through the call-and-return sequence.
- Use `setjmp` and `longjmp` functions.

### **`setjmp`**

```c
#include <setjmp.h>

int setjmp(jmp_buf env);
int sigsetjmp(sigjump_buf env, int savesigs);
// returns 0 from setjmp, nonzero from longjmp
```

- `setjmp` saves the current *calling environment* in `env` for later use by `longjmp`, and returns 0.
  - Calling environment: includes program counter, stack pointer, and general-purpose registers.
- Note that the return value of `setjmp` should not be assigned a variable, but can only be used in `switch` or conditional statement.

### **`longjmp`**

```c
#include <setjmp.h>

void longjmp(jmp_buf env, int retval);
void siglongjmp(sigjmp_buf env, int retval);
// never returns
```

- `longjmp` restores the calling environment from `env` and triggers a return from the most recent `setjmp` call that initialized `env`. The `setjmp` then returns with the nonzero return value `retval`.

### **`setjmp` and `longjmp`**

- `setjmp` is called once but returns multiple times: once when `setjmp` is first called, and once for each corresponding `longjmp` call.
- `longjmp` is called once but never returns.

<br>

### **Applications of Nonlocal Jumps**

**I.** To permit an immediate return from a deeply nested function call, usually after detecting some error:

```c
#include "csapp.h"

jmp_buf buf;

int error1 = 0;
int error2 = 1;

// functions that will be called in main
void foo(void), bar(void);

int main(){
    switch(setjmp(buf)){        // setjmp saves calling environment in buf
        case 0:
            foo();      // call foo()
            break;
        case 1:
            printf("Detected error1 condition in foo\n");
            break;
        case 2:
            printf("Deteched error2 condition in foo\n");
            break;
        default:
            printf("Unknown error condition in foo\n");
    }
    exit(0);
}

// deeply nested function foo
void foo(void){
    if(error1)
        longjmp(buf, 1);
    bar();                  // foo calls bar()
}

void bar(void){
    if(error2)
        longjmp(buf, 2);    
        // longjmp causes the setjmp in main to return again, with return value 2
        // So we skip the intermediate function of foo() and return to main directly
}
```

**II.** To branch out of a signal handler to a specific code section, rather than returning to the instruction that was interrupted by the signal:

```c
#include "csapp.h"

sigjmp_buf buf;

void handler(int sig){
    siglongjmp(buf, 1);     // equivalent to longjmp, but used by signal handlers
}

int main(){
    if(!sigsetjmp(buf, 1)){         // equivalent to setjmp, but for signal handlers
        Signal(SIGINT, handler);    // install handler
        Sio_puts("starting\n");
    } else
        Sio_puts("restarting\n");

    while(1) {
        Sleep(1);
        Sio_puts("processing...\n");
    }
    exit(0);        // control never reaches here
}
```

- Note that to avoid a race, we have to install the handler after `sigsetjmp` is called. This prevents the situation where the handler runs before `sigsetjmp` sets up the calling environment.

<br>

# 8.7 Tools for Manipulating Processes

- `strace` - prints a trace of each system call invoked by a running program and its children.
  - Compile with `-static` to get cleaner output.
- `ps` - lists processes (including zombies) currently in the system.
- `top` - prints information about the resource usage of current processes.
- `pmap` - displays the memory map of a process.
- `\proc` - a virtual file system that exports the contents of numerous kernel data structures in an ASCII text form that can be read by user programs.