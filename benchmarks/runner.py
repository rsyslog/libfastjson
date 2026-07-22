#!/usr/bin/env python3
"""Paired, resumable libfastjson microbenchmark runner and report generator."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import statistics
import subprocess
import sys
import tempfile


LOOKUP_WIDTHS = (1, 4, 8, 16, 32)
STRING_LENGTHS = (8, 16, 31, 32, 33, 64, 127, 128, 129, 208, 209, 256, 4096)
OBJECT_WIDTHS = (0, 1, 4, 8, 9, 16, 32)
OPERATIONS = ("lookup", "replace", "string", "object", "object-new")
POSITIONS = ("first", "middle", "last", "miss")
SCHEMA_VERSION = 1


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", required=True, type=Path)
    parser.add_argument("--label", default="baseline")
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--pair-build-dir", type=Path)
    parser.add_argument("--pair-label", default="candidate")
    parser.add_argument("--pair-output", type=Path)
    parser.add_argument("--trials", type=int, default=11)
    parser.add_argument("--calibrations", type=int, choices=(1,), default=1)
    parser.add_argument("--target-ms", type=int, default=200)
    parser.add_argument("--operation", action="append", choices=OPERATIONS)
    parser.add_argument("--width", action="append", type=int)
    parser.add_argument("--position", action="append", choices=POSITIONS)
    parser.add_argument("--value-bytes", action="append", type=int)
    parser.add_argument("--case-sensitive", action="store_true")
    parser.add_argument("--smoke", action="store_true")
    parser.add_argument("--summary-json", type=Path)
    parser.add_argument("--summary-markdown", type=Path)
    parser.add_argument("--comparison-json", type=Path)
    parser.add_argument("--comparison-markdown", type=Path)
    args = parser.parse_args(argv)
    if args.trials < 1 or args.target_ms < 10:
        parser.error("trials must be positive and target-ms at least 10")
    if bool(args.pair_build_dir) != bool(args.pair_output):
        parser.error("--pair-build-dir and --pair-output must be provided together")
    if args.pair_build_dir and args.label == args.pair_label:
        parser.error("paired labels must differ")
    if args.output.resolve() == (args.pair_output.resolve() if args.pair_output else None):
        parser.error("paired output paths must differ")
    if bool(args.comparison_json) != bool(args.pair_build_dir):
        parser.error("--comparison-json requires paired mode")
    if bool(args.comparison_markdown) != bool(args.pair_build_dir):
        parser.error("--comparison-markdown requires paired mode")
    if args.smoke:
        args.trials = 1
        args.calibrations = 1
        args.target_ms = 20
    return args


def workloads(operations=None, widths=None, positions=None, value_bytes=None, case_sensitive=False, smoke=False):
    selected = set(operations or OPERATIONS)
    width_filter = set(widths or ())
    position_filter = set(positions or ())
    value_filter = set(value_bytes or ())
    output = []

    if "lookup" in selected:
        lookup_widths = (1, 8) if smoke else LOOKUP_WIDTHS
        for width in lookup_widths:
            valid_positions = ("first", "miss") if width == 1 else POSITIONS
            for position in valid_positions:
                if width_filter and width not in width_filter:
                    continue
                if position_filter and position not in position_filter:
                    continue
                output.append(workload("lookup", width, position, 0, case_sensitive))
    if "replace" in selected:
        replace_widths = (1, 8) if smoke else (1, 8, 32)
        for width in replace_widths:
            valid_positions = ("first",) if width == 1 else ("first", "last")
            for position in valid_positions:
                if width_filter and width not in width_filter:
                    continue
                if position_filter and position not in position_filter:
                    continue
                output.append(workload("replace", width, position, 0, case_sensitive))
    if "string" in selected:
        lengths = (31, 32, 33, 128) if smoke else STRING_LENGTHS
        for length in lengths:
            if value_filter and length not in value_filter:
                continue
            output.append(workload("string", 0, "none", length, case_sensitive))
    for operation in ("object", "object-new"):
        if operation in selected:
            object_widths = (0, 1, 8, 9) if smoke else OBJECT_WIDTHS
            for width in object_widths:
                if width_filter and width not in width_filter:
                    continue
                output.append(workload(operation, width, "none", 0, case_sensitive))
    if not output:
        raise SystemExit("filters selected no workloads")
    return output


def workload(operation, width, position, value_bytes, case_sensitive):
    return {
        "operation": operation,
        "width": width,
        "position": position,
        "value_bytes": value_bytes,
        "case_sensitive": int(case_sensitive),
    }


def workload_key(record):
    return tuple(record[name] for name in
                 ("operation", "width", "position", "value_bytes", "case_sensitive"))


def command_output(command, cwd=None):
    return subprocess.run(command, cwd=cwd, check=True, text=True, stdout=subprocess.PIPE).stdout.strip()


def cpu_model():
    try:
        for line in Path("/proc/cpuinfo").read_text().splitlines():
            if line.startswith("model name"):
                return line.split(":", 1)[1].strip()
    except OSError:
        pass
    return platform.processor() or platform.machine()


def file_hash(path):
    digest = hashlib.sha256()
    with open(path, "rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def build_metadata(build_dir, label, harness_hash):
    build_dir = build_dir.resolve()
    revision = command_output(["git", "rev-parse", "HEAD"], build_dir)
    tracked_diff = subprocess.run(
        ["git", "diff", "--binary", "HEAD", "--", "*.c", "*.h", "Makefile.am", "configure.ac"],
        cwd=build_dir, check=True, stdout=subprocess.PIPE).stdout
    config_status = Path(build_dir, "config.status")
    library = Path(build_dir, ".libs", "libfastjson.so")
    if not library.exists():
        raise SystemExit("build directory lacks .libs/libfastjson.so; run configure and make first")
    return {
        "label": label,
        "revision": revision,
        "library_source_fingerprint": hashlib.sha256(tracked_diff).hexdigest(),
        "library_source_dirty": bool(tracked_diff),
        "library_sha256": file_hash(library.resolve()),
        "config_status_sha256": file_hash(config_status) if config_status.exists() else None,
        "harness_sha256": harness_hash,
        "architecture": platform.machine(),
        "cpu": cpu_model(),
        "kernel": platform.release(),
        "python": platform.python_version(),
        "clock": "CLOCK_MONOTONIC_RAW",
        "host_exclusive": False,
        "compiler": command_output([os.environ.get("CC", "cc"), "--version"]).splitlines()[0],
        "configure_args": command_output([str(config_status), "--config"], build_dir)
        if config_status.exists() else None,
        "cflags": next((line.split("=", 1)[1].strip() for line in Path(build_dir, "Makefile").read_text().splitlines()
                        if line.startswith("CFLAGS =")), None),
    }


def compile_benchmark(build_dir, output, source):
    output.parent.mkdir(parents=True, exist_ok=True)
    compiler = os.environ.get("CC", "cc")
    command = [compiler, "-O2", "-std=c99", "-Wall", "-Wextra", "-Werror",
               "-I", str(build_dir.resolve()), "-I", str(source.parent.parent.resolve()),
               str(source.resolve()), str(Path(build_dir, ".libs", "libfastjson.so").resolve()),
               "-Wl,-rpath," + str(Path(build_dir, ".libs").resolve()), "-lm", "-o", str(output)]
    subprocess.run(command, check=True)


def run_once(binary, spec, iterations):
    command = [str(binary), spec["operation"], str(spec["width"]), spec["position"],
               str(spec["value_bytes"]), str(iterations), str(spec["case_sensitive"])]
    result = subprocess.run(command, check=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    record = json.loads(result.stdout)
    if not record.get("oracle"):
        raise RuntimeError("benchmark correctness oracle failed")
    return record


def calibrated_iterations(binary_specs, spec, target_ms):
    seed = 10000
    suggestions = []
    calibration_records = []
    for label, binary in binary_specs:
        record = run_once(binary, spec, seed)
        record.update({"label": label, "measured": False, "trial": 0})
        calibration_records.append(record)
        elapsed = max(record["elapsed_ns"], 1)
        suggestion = int(seed * target_ms * 1000000 / elapsed)
        suggestions.append(max(1000, min(suggestion, 100000000)))
    return max(suggestions), calibration_records


def atomic_json(path, document):
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile("w", dir=path.parent, delete=False) as stream:
        json.dump(document, stream, indent=2, sort_keys=True)
        stream.write("\n")
        temporary = Path(stream.name)
    temporary.replace(path)


def load_or_create(path, metadata, config):
    if not path.exists():
        return {"schema_version": SCHEMA_VERSION, "metadata": metadata, "config": config,
                "calibrations": [], "records": []}
    document = json.loads(path.read_text())
    if document.get("schema_version") != SCHEMA_VERSION:
        raise SystemExit(f"cannot resume {path}: schema version differs")
    for name in ("revision", "library_source_fingerprint", "library_sha256", "config_status_sha256",
                 "harness_sha256", "architecture", "cpu", "kernel", "python", "clock", "host_exclusive",
                 "compiler", "configure_args", "cflags"):
        if document["metadata"].get(name) != metadata.get(name):
            raise SystemExit(f"cannot resume {path}: metadata field {name} differs")
    if document.get("config") != config:
        raise SystemExit(f"cannot resume {path}: benchmark configuration differs")
    return document


def median_mad(values):
    median = statistics.median(values)
    mad = statistics.median(abs(value - median) for value in values)
    return median, mad


def outlier_trials(records):
    values = [record["ns_per_operation"] for record in records]
    median, mad = median_mad(values)
    if mad == 0:
        return []
    return [record["trial"] for record in records
            if abs(record["ns_per_operation"] - median) / mad > 3.5]


def trial_order(items, trial):
    """Alternate paired execution order without mutating the input list."""
    return list(items) if trial % 2 else list(reversed(items))


def validate_complete(document):
    """Reject partial, duplicate, or unexpected measured records before reporting."""
    expected_keys = {workload_key(spec) for spec in document["config"]["workloads"]}
    expected_trials = set(range(1, document["config"]["trials"] + 1))
    grouped = {}
    for record in document["records"]:
        grouped.setdefault(workload_key(record), []).append(record["trial"])
    if set(grouped) != expected_keys:
        raise SystemExit("benchmark document does not contain the configured workload set")
    for key, trials in grouped.items():
        if len(trials) != len(set(trials)):
            raise SystemExit(f"benchmark document contains duplicate trials for {key}")
        if set(trials) != expected_trials:
            raise SystemExit(f"benchmark document is incomplete for {key}")


def summarize(document):
    validate_complete(document)
    grouped = {}
    for record in document["records"]:
        grouped.setdefault(workload_key(record), []).append(record)
    rows = []
    for key, records in sorted(grouped.items()):
        values = [record["ns_per_operation"] for record in records]
        median, mad = median_mad(values)
        row = dict(zip(("operation", "width", "position", "value_bytes", "case_sensitive"), key))
        row.update({
            "trials": len(records),
            "median_ns_per_operation": median,
            "median_operations_per_second": 1000000000.0 / median,
            "mad_ns_per_operation": mad,
            "relative_mad": mad / median if median else 0,
            "outlier_trials": outlier_trials(records),
        })
        rows.append(row)
    return {"schema_version": SCHEMA_VERSION, "kind": "characterization",
            "metadata": document["metadata"], "config": document["config"], "workloads": rows}


def compare(baseline, candidate):
    validate_complete(baseline)
    validate_complete(candidate)
    base_group = {}
    candidate_group = {}
    for record in baseline["records"]:
        base_group.setdefault(workload_key(record), {})[record["trial"]] = record
    for record in candidate["records"]:
        candidate_group.setdefault(workload_key(record), {})[record["trial"]] = record
    if set(base_group) != set(candidate_group):
        raise SystemExit("paired documents contain different workloads")
    rows = []
    for key in sorted(base_group):
        trials = sorted(base_group[key])
        ratios = [base_group[key][trial]["ns_per_operation"] /
                  candidate_group[key][trial]["ns_per_operation"] for trial in trials]
        median, mad = median_mad(ratios)
        row = dict(zip(("operation", "width", "position", "value_bytes", "case_sensitive"), key))
        row.update({"trials": len(trials), "median_speedup_ratio": median, "mad": mad,
                    "relative_mad": mad / median if median else 0,
                    "improves_at_least_10_percent": median >= 1.10,
                    "regresses_over_5_percent": median < 0.95})
        rows.append(row)
    return {"schema_version": SCHEMA_VERSION, "kind": "comparison",
            "ratio_definition": "baseline ns/op divided by candidate ns/op; above 1 is faster",
            "baseline_metadata": baseline["metadata"], "candidate_metadata": candidate["metadata"],
            "workloads": rows}


def markdown(report):
    if report["kind"] == "characterization":
        lines = [f"# libfastjson characterization: {report['metadata']['label']}", "",
                 f"Revision: `{report['metadata']['revision']}`", "",
                 "| Operation | Width | Position | Bytes | Median ns/op | M operations/s | MAD | Outliers |",
                 "|---|---:|---|---:|---:|---:|---:|---|"]
        for row in report["workloads"]:
            lines.append("| {operation} | {width} | {position} | {value_bytes} | {median_ns_per_operation:.3f} | "
                         "{mops:.3f} | {relative_mad:.4f} | {outliers} |".format(
                             mops=row["median_operations_per_second"] / 1000000,
                             outliers=",".join(map(str, row["outlier_trials"])) or "-", **row))
    else:
        lines = ["# libfastjson paired comparison", "", report["ratio_definition"], "",
                 "| Operation | Width | Position | Bytes | Speedup | MAD | >=10% | >5% regression |",
                 "|---|---:|---|---:|---:|---:|---|---|"]
        for row in report["workloads"]:
            lines.append("| {operation} | {width} | {position} | {value_bytes} | {median_speedup_ratio:.4f} | "
                         "{mad:.4f} | {improves_at_least_10_percent} | {regresses_over_5_percent} |".format(**row))
    return "\n".join(lines) + "\n"


def write_report(path, content):
    if path:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content)


def main(argv=None):
    args = parse_args(argv)
    source = Path(__file__).with_name("benchmark.c")
    harness_hash = hashlib.sha256(source.read_bytes() + Path(__file__).read_bytes()).hexdigest()
    matrix = workloads(args.operation, args.width, args.position, args.value_bytes,
                       args.case_sensitive, args.smoke)
    config = {"trials": args.trials, "calibrations": args.calibrations, "target_ms": args.target_ms,
              "workloads": matrix}

    primary_metadata = build_metadata(args.build_dir, args.label, harness_hash)
    primary = load_or_create(args.output, primary_metadata, config)
    artifact_root = args.output.parent / "artifacts"
    primary_binary = artifact_root / args.label / "benchmark"
    compile_benchmark(args.build_dir, primary_binary, source)
    builds = [(args.label, primary_binary, primary, args.output)]

    if args.pair_build_dir:
        pair_metadata = build_metadata(args.pair_build_dir, args.pair_label, harness_hash)
        pair = load_or_create(args.pair_output, pair_metadata, config)
        pair_binary = artifact_root / args.pair_label / "benchmark"
        compile_benchmark(args.pair_build_dir, pair_binary, source)
        builds.append((args.pair_label, pair_binary, pair, args.pair_output))

    for spec in matrix:
        calibration_matches = []
        for label, binary, document, path in builds:
            matches = [record for record in document["calibrations"]
                       if workload_key(record) == workload_key(spec)]
            if len(matches) > 1:
                raise SystemExit(f"duplicate calibration records for {label} and {workload_key(spec)}")
            calibration_matches.append((label, binary, document, path, matches))
        existing_iterations = {matches[0]["selected_iterations"]
                               for _, _, _, _, matches in calibration_matches if matches}
        if len(existing_iterations) > 1:
            raise SystemExit(f"inconsistent calibrated iterations for {workload_key(spec)}")
        if existing_iterations:
            iterations = existing_iterations.pop()
            for label, binary, document, path, matches in calibration_matches:
                if matches:
                    continue
                record = run_once(binary, spec, 10000)
                record.update({"label": label, "measured": False, "trial": 0,
                               "selected_iterations": iterations})
                document["calibrations"].append(record)
                atomic_json(path, document)
        else:
            binary_specs = [(label, binary) for label, binary, _, _ in builds]
            iterations, calibration_records = calibrated_iterations(binary_specs, spec, args.target_ms)
            for record in calibration_records:
                record["selected_iterations"] = iterations
                for name, value in spec.items():
                    record[name] = value
                for label, _, document, path in builds:
                    if record["label"] == label:
                        document["calibrations"].append(record)
                        atomic_json(path, document)

        for trial in range(1, args.trials + 1):
            order = trial_order(builds, trial)
            for label, binary, document, path in order:
                if any(workload_key(record) == workload_key(spec) and record["trial"] == trial
                       for record in document["records"]):
                    continue
                record = run_once(binary, spec, iterations)
                record.update({"label": label, "measured": True, "trial": trial})
                document["records"].append(record)
                atomic_json(path, document)

    primary_summary = summarize(primary)
    write_report(args.summary_json, json.dumps(primary_summary, indent=2, sort_keys=True) + "\n")
    write_report(args.summary_markdown, markdown(primary_summary))
    if len(builds) == 2:
        comparison = compare(primary, builds[1][2])
        write_report(args.comparison_json, json.dumps(comparison, indent=2, sort_keys=True) + "\n")
        write_report(args.comparison_markdown, markdown(comparison))
    return 0


if __name__ == "__main__":
    sys.exit(main())
