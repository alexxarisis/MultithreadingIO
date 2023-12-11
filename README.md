# Multithreading IO

### *University Project*
---

Implemented multithreaded IO operations in the [kiwi OS](https://github.com/aejsmith/kiwi).

* Enabled support for parallel reads and single write operations.

* Through use of threads and mutex locks.

The changes took place in a part of the Kiwi OS, its storage system.

All the changes are recorded in the `Edited Files` directory.

*The means of running this have been lost in time, but the results still remain.*

## Results

Test results for 1.000.000 IO operations, at different thread values.

|                    | Initial project (1 thread) (sec/op*) | 100 threads (sec/op*)  |
| -------------------|:------------------------------------:|:----------------------:|
| **Reads**          | 0.000014                             | 0.000003               |
| **Writes**         | 0.000022                             | 0.000010               |
| __Reads & Writes**__| 0.000034 / 0.000034                 | 0.000014 / 0.000017    |

*&nbsp; Seconds per operation.

** Reads and Writes happening at the same time.


## Report

An explanation of the changes is recorded in `Report.pdf`, but it is written in Greek.

*Although, it also contains the results of the tests above, in screenshots.*
