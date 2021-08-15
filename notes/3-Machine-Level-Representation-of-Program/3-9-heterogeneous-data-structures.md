# 3.9 Heterogeneous Data Structures

## 3.9.1 Structures

- `struct` groups objects of possibly different types into a single object.
- Components are stored in a contiguous region of memory.
- A poiner to a structure is the addres of the first byte.
- To access the structure fields, the compiler generates code that adds the appropriate offset to the address of the structure.

### Example:

```c
// declaration of node in linked list
struct node {
    short v;
    struct node *next;
}

// computes the sum of all list elements
short sum(struct node *ptr){
    int result = 0;
    while(ptr != 0){
        result += ptr->v;
        ptr = ptr->next;
    }
    return result;
}

// generated assembly code
// ptr in %rdi
sum:
    movl    $0, %eax            # initialize result to 0
    jmp     .L2
.L3:
    addq    (%rdi), %rax        # add v to result
                                # recall that ptr points to first byte of struct
    movq    2(%rdi), %rdi       # set ptr to ptr->next
.L2:
    testq   %rdi, %rdi          # test condition for loop
    jne     .L3
    ret;
```

<br>

## 3.9.2 Unions

- Allows a single object to be referenced according to multiple types.
- Declaration:

```c
union U3{
    char c;
    int i[2];
    double v;
}
```

- For a pointer `p` of type `union U3 *`, references `p->c`, `p->i[0]`, and `p->v` would all reference the beginning of the data structure (offset is 0).
- The overall size of a union is the maximum size of its fields.
- We can use unions when we know in davance that the use of two different fields in a data structure will be mutually exclusive, and in this case, declaring the two fields as a union rather than a structure will reduce the space needed.
- A common way of determining which field will be used in a given union is by introducing a `type` field:

```c
struct node{
    int type;           // indicates the type of union
    union {
        char c;
        int i;
    }
}
```

- Unions can also be used to access the bit patterns of different data types.

```c
// accessing the bit pattern of double d
unsigned long doubleToBits(double d){
    union {
        double d;
        unsigned long u;
    } temp;
    temp.d = d;         // store d as double
    return temp.u;      // access it as unsigned long
}

// Here, u will have the same bit representation as d, although the numeric value of u will bear no relation to that of d, except when d = 0.0.
```

<br>

## 3.9.3 Data Alignment

- *Alignment restrictions* require that the address of some objects must be a multiple of some value $K$.
- x86-64 hardware will work correctly regardless of data alignment, but Intel recommends that data be aligned to improve memory system performance.
  - **Principle: Any primitive object of $K$ bytes must have an address that is a multiple of $K$**.
- For structures, the compiler will insert gaps in the field allocation to ensure that each field satisfies the alignment requirement.

```c
struct S1{
    int i;          // offset 0
    char c;         // offset 4
    int j;          // offset 8, instead of 5
}
```