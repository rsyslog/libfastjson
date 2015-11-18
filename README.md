libfastjson
===========
**NOTE: libfastjson is a fork from json-c, and is currently under development.**


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
$ git clone https://github.com/libfastjson/libfastjson.git
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

Linking to `liblibfastjson`
---------------------------

If your system has `pkgconfig`,
then you can just add this to your `makefile`:

```make
CFLAGS += $(shell pkg-config --cflags libfastjson)
LDFLAGS += $(shell pkg-config --libs libfastjson)
```

Without `pkgconfig`, you would do something like this:

```make
JSON_C_DIR=/path/to/json_c/install
CFLAGS += -I$(JSON_C_DIR)/include/libfastjson
LDFLAGS+= -L$(JSON_C_DIR)/lib -llibfastjson
```
