#!/usr/bin/env python3
"""Deterministic tests for libfastjson benchmark plumbing."""

import contextlib
import io
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).parent))
import runner  # noqa: E402


class BenchmarkTests(unittest.TestCase):
    def test_full_matrix_is_stable_and_covers_boundaries(self):
        matrix = runner.workloads()
        self.assertEqual(len(matrix), 50)
        self.assertIn(runner.workload("lookup", 32, "miss", 0, False), matrix)
        self.assertIn(runner.workload("string", 0, "none", 31, False), matrix)
        self.assertIn(runner.workload("string", 0, "none", 32, False), matrix)
        self.assertIn(runner.workload("object", 9, "none", 0, False), matrix)
        self.assertIn(runner.workload("object-new", 9, "none", 0, False), matrix)

    def test_smoke_matrix_is_reduced(self):
        matrix = runner.workloads(smoke=True)
        self.assertLess(len(matrix), len(runner.workloads()))
        self.assertEqual({entry["operation"] for entry in matrix}, set(runner.OPERATIONS))

    def test_filters(self):
        matrix = runner.workloads(["lookup"], [8], ["last"], None)
        self.assertEqual(matrix, [runner.workload("lookup", 8, "last", 0, False)])

    def test_median_mad_and_outliers(self):
        median, mad = runner.median_mad([10, 10, 11, 12, 100])
        self.assertEqual(median, 11)
        self.assertEqual(mad, 1)
        records = [{"trial": index + 1, "ns_per_operation": value}
                   for index, value in enumerate([10, 10, 11, 12, 100])]
        self.assertEqual(runner.outlier_trials(records), [5])

    def test_alternating_order_does_not_mutate(self):
        builds = ["baseline", "candidate"]
        self.assertEqual(runner.trial_order(builds, 1), builds)
        self.assertEqual(runner.trial_order(builds, 2), list(reversed(builds)))
        self.assertEqual(builds, ["baseline", "candidate"])

    def test_comparison_ratio(self):
        metadata = {"label": "x"}
        spec = runner.workload("lookup", 1, "first", 0, False)
        config = {"trials": 3, "workloads": [spec]}
        baseline_records = []
        candidate_records = []
        for trial, baseline_ns, candidate_ns in ((1, 20, 10), (2, 22, 11), (3, 18, 9)):
            baseline_records.append(dict(spec, trial=trial, ns_per_operation=baseline_ns))
            candidate_records.append(dict(spec, trial=trial, ns_per_operation=candidate_ns))
        baseline = {"metadata": metadata, "config": config, "records": baseline_records}
        candidate = {"metadata": metadata, "config": config, "records": candidate_records}
        row = runner.compare(baseline, candidate)["workloads"][0]
        self.assertEqual(row["median_speedup_ratio"], 2)
        self.assertEqual(row["mad"], 0)

    def test_reports_reject_incomplete_records(self):
        spec = runner.workload("lookup", 1, "first", 0, False)
        document = {"metadata": {}, "config": {"trials": 2, "workloads": [spec]},
                    "records": [dict(spec, trial=1, ns_per_operation=10)]}
        with self.assertRaises(SystemExit):
            runner.summarize(document)

    def test_resume_rejects_changed_host_metadata(self):
        fields = ("revision", "library_source_fingerprint", "library_sha256", "config_status_sha256",
                  "harness_sha256", "architecture", "cpu", "kernel", "python", "clock", "host_exclusive",
                  "compiler", "configure_args", "cflags")
        metadata = {name: name for name in fields}
        config = {"trials": 1}
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory, "raw.json")
            runner.atomic_json(path, {"schema_version": runner.SCHEMA_VERSION,
                                      "metadata": metadata, "config": config,
                                      "calibrations": [], "records": []})
            changed = dict(metadata, kernel="different")
            with self.assertRaises(SystemExit):
                runner.load_or_create(path, changed, config)

    def test_invalid_pair_arguments(self):
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            runner.parse_args(["--build-dir", ".", "--output", "a", "--pair-build-dir", "."])
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            runner.parse_args(["--build-dir", ".", "--output", "a", "--label", "same",
                               "--pair-build-dir", ".", "--pair-output", "b", "--pair-label", "same"])


if __name__ == "__main__":
    unittest.main()
