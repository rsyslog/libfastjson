libfastjson
===========
**NOTE: libfastjson is a fork from json-c, and is currently under development.**

The aim of this project is **not** to provide a slightly modified clone
of json-c. It's aim is to provide

* a **small** library with essential json handling functions
* sufficiently good json support (not 100% standards compliant)
* be very fast in processing

In order to reach these goals, we reduce the features of json-c. For
similarities and differences, see the file DIFFERENCES.
 
**IMPORTANT**
The current API is **not** stable and will change until version 1.0.0 is
reached. We plan to reach it by summer 2016 at latest. With 1.0.0, the API
will be stable. Until then, everything may change. Of course, we will not
deliberatly break things but we need freedom to restructure.


Building on Unix with `git`, `gcc` and `autotools`
--------------------------------------------------

Prerequisites:

 - `gcc`, `clang`, or another C compiler
 - `libtool`

If you're not using a release tarball, you'll also need:

 - `autoconf` (`autoreconf`)
 - `automake`

Make sure you have a complete `libtool` install, including `libtoolize`.

`libfastjson` GitHub repo: https://github.com/rsyslog/libfastjson

```bash
$ git clone https://github.com/rsyslog/libfastjson.git
$ cd libfastjson
$ sh autogen.sh
```

followed by

```bash
$ ./configure
$ make
$ make install
```

To build and run the test programs:

```bash
$ make check
```

Linking to `libfastjson`
---------------------------

If your system has `pkgconfig`,
then you can just add this to your `makefile`:

```make
CFLAGS += $(shell pkg-config --cflags libfastjson)
LDFLAGS += $(shell pkg-config --libs libfastjson)
```

Without `pkgconfig`, you would do something like this:

```make
LIBFASTJSON_DIR=/path/to/json_c/install
CFLAGS += -I$(LIBFASTJSON_DIR)/include/libfastjson
LDFLAGS+= -L$(LIBFASTJSON_DIR)/lib -lfastjson
```

