# Note

**All** tests are run alongside `leaks` (a MacOS alternative for `Vanguard`) to ensure that there are no memory leaks.
Currently, due to the shortcomings of the `EXPECT` macro, there is a leak whenever the test(s) fail.
I plan to rework the test module so that there is some form of `FALLBACK` macro to perform a procedure
whenever a test fails. This would allow me to `free` the memory used when the test fails.
