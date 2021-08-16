# 8.3 System Call Error Handling

- When Unix system-level functions encounter an error, they typically return `-1` and set the global integer variable `errno` to indicate the error.
- Programs should always check for errors.

Example of ***error-reporting function***:

```c
void unix_error(char *msg){
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    // strerror returns a text string describing the error
    exit(0);
}
```

We can also use ***error-handling wrappers***:

```c
// Instead of using fork(), we use a wrapped Fork() that checks for errors
pid_t Fork(void){
    pid_t pid;
    if((pid = fork()) < 0)
        unix_error("Fork error");
    return pid;
}
```