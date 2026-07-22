# libfastjson performance benchmarks

This suite isolates the libfastjson operations that dominate rsyslog's `$!`
variable path. It measures case-insensitive object lookup by width and key
position, replacement writes, string creation around inline-storage
boundaries, and narrow-object construction/destruction both with normal
duplicate checking and with the existing `FJSON_OBJECT_ADD_KEY_IS_NEW` flag.

Every workload uses a correctness checksum and structural oracle. The runner
performs one calibration, then 11 measured trials, records resumable raw JSON,
and reports medians, median absolute deviation, and outliers. Paired mode runs
baseline and candidate builds in alternating order.

Build libfastjson first:

```sh
CFLAGS='-O2 -g -fno-omit-frame-pointer' ./autogen.sh --configure
make -j"$(nproc)"
```

Record a characterization session:

```sh
benchmarks/run.sh \
  --build-dir "$PWD" \
  --label current-main-a \
  --output benchmarks/raw/current-main-a.json \
  --summary-json benchmarks/results/current-main-a.json \
  --summary-markdown benchmarks/results/current-main-a.md
```

Use `--smoke` for a reduced matrix. Filters `--operation`, `--width`,
`--position`, and `--value-bytes` are repeatable.

For candidate acceptance, use paired mode with separate worktrees/builds:

```sh
benchmarks/run.sh \
  --build-dir /path/to/baseline \
  --label baseline \
  --output benchmarks/raw/baseline.json \
  --pair-build-dir /path/to/candidate \
  --pair-label candidate \
  --pair-output benchmarks/raw/candidate.json \
  --comparison-json benchmarks/results/comparison.json \
  --comparison-markdown benchmarks/results/comparison.md
```

A runtime candidate should be retained only when two independent paired
sessions each improve its targeted workload by at least 10%, no core workload
regresses by more than 5%, and relative MAD is at most 0.05 after one rerun.

Run deterministic harness tests with:

```sh
python3 benchmarks/selftest.py
```

Tracked current-main characterization and direct-lookup candidate results are
under `benchmarks/results/`. Raw trial data remain ignored under
`benchmarks/raw/`.
