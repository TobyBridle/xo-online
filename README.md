# Setup

**NOTE**: By default, there are no `bin` folders.

### Recommended Setup

```fish
toby@desktop:~/xo-online$ mkdir bin && make tests/bin \
  && make
```

### Only the Necessities

If you would like to create only the server & client, omitting
the tests, simply only create a `bin` folder within the root of the project.

```fish
toby@desktop:~/xo-online$ mkdir bin && make
```

### Compiling Tests

If you would like the tests, you will need to have **both**
the `bin` in the project root and the `bin` in the test folder

```fish
toby@desktop:~/xo-online$ mkdir bin && tests/bin && make tests
```

### Running All Tests

To run every test, run the `make test` command. As before,
this assumes that the `bin` and `tests/bin` folders are created.

```fish
toby@desktop:~/xo-online$ make test
```

---

# Configuring Makefile

Release builds are compiled without the `-DDEBUG` flag
and `-fsanitize=address` (Address Sanitizer)
They use a much higher level of optimisation compared to debug builds,
using `-O3` rather than `-O0`

If you would like to compile the debug build, use the options
within the `Makefile`:

```make
...
OPT=-O0
CFLAGS=-Wall -Wextra -g -DDEBUG -fsanitize=address $(INCDIRS) $(OPT)

all: server client tests
...
```

---

<b>May be useful..</b>

- [RFC 9110](https://www.rfc-editor.org/rfc/rfc9110)
- [Beej's Guide for Network Programming](https://beej.us/guide/bgnet/html/split/ip-addresses-structs-and-data-munging.html)
