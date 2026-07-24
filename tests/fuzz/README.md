# libfastjson fuzzing

The checked-in corpus is replayed by `make check` through deterministic
standalone executables. Each input must remain safe under normal and sanitizer
builds.

Build libFuzzer targets with Clang after configuring and building the library:

```sh
make fuzz
make fuzz-smoke FUZZ_SECONDS=60
```

The targets need Clang and its libFuzzer runtime plus a C++ standard library.
Set `FUZZ_CXX_LIBS` if the linker cannot find its default `-lstdc++`.
`FUZZ_RSS_LIMIT_MB` sets the libFuzzer memory limit (default: 2048 MiB).

`fuzz-smoke` runs both targets with AddressSanitizer and
UndefinedBehaviorSanitizer. A minimized input that reproduces a crash must be
added to the relevant `corpus/` directory. Do not commit automatically
generated corpus growth without reviewing and minimizing it.
