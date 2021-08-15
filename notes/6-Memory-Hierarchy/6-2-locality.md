# 6.2 Locality

- **Principle of locality**: Good computer programs tend to reference data items that are near other recently referenced data items or that were recently referenced themselves.
- Two distinct forms of locality:
  - *Temporal locality*: a memory location that is referenced once is likely to be referenced again multiple times in the near future.
  - *Spatial locality*: if a memory location is referenced once, then a nearby memory location is likely to be referenced in the near future.
- Locality is important because in general, *programs with good locality run faster than programs with poor locality.*

<br>

## 6.2.1 Locality of References to Program Data

Consider the following example:

```c
int sumvec(int v[N]){
    int i, sum = 0;
    for(i = 0; i < N; i++)
        sum += v[i];
    return sum;
}
```

- Variable `sum`:
  - It is referenced once in each loop iteration, so the function exhibits good temporal locality with respect to `sum`.
  - It is a scalar, so there is no spatial locality with respect to `sum`.
- Elements of vector `v`:
  - They are read sequentially, in the order they are stored in memory, so the function exhibits good spatial locality with respect to `v`.
  - Each element of `v` is read only once, so there is poor temporal locality.
- Since the function has either good spatial or temporal locality with respect to each variable in the loop, we can cnclude that the function has good locality.

<br>

- A function like `sumvec` above is said to have a *stride-1 reference pattern* or *sequential reference pattern*.
  - Visiting every *k*th element of a contiguous vector is called a *stride-k reference pattern*. In general, as te stride increases, the spatial locality decreases.

<br>

Another interesting point on locality is the comparison between accessing a multi-dimensional array in *row-major order* and *column-major order*. 

- *Row-major order*: good locality, since the references occur in the order that the array is stored.
- *Column-major order*: bad locality, since the references do not occur in the order that the array is stored.

<br>

## 6.2.2 Locality of Instruction Fetches

Take the `sumvec` function above as an example. The instructions in the body of the `for` loop are executed in sequential memory order, and thus the loop enjoys good spatial locality. The loop body is executed multiple times, so it also enjoys good temporal locality.

In general, loops have good temporal and spatial locality with respect to instruction fetches. *The smaller the loop body and the greater the number of loop iterations, the better the locality.*

<br>

## 6.2.3 Summary of Locality

