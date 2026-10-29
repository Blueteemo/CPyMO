#!/usr/bin/env python3
"""Compare CPyMO NDS experimental loadbench logs; stdlib only.

Usage:
  python3 cpymo-backends/nds/tools/compare-loadbench.py cold.txt cached.txt
  python3 cpymo-backends/nds/tools/compare-loadbench.py --self-test

No measured results are fabricated. --self-test uses deliberately synthetic
arrays to exercise the parser and comparison guards, not performance claims.
"""
import argparse
import tempfile
from pathlib import Path

HEADER = "CPYMO_NDS_LOADBENCH_V1"
COUNTS = {
    "load_bench": 40, "load_state": 32, "compressed_state": 8,
    "engine_state": 46, "perf_state": 9, "background_cache_state": 5,
    "stream_diag": 8, "audio_bench": 12,
}
ASSETS = ("image 1 (bg)", "image 2 (chara)", "image 3 (bg)", "image 4 (chara)")


def read_log(path):
    lines = Path(path).read_text(encoding="utf-8").splitlines()
    if not lines or lines[0] != HEADER:
        raise ValueError(f"{path}: expected {HEADER}")
    values = {}
    for line in lines[1:]:
        if "=" not in line:
            raise ValueError(f"{path}: malformed row {line!r}")
        k, v = line.split("=", 1)
        if k in values:
            raise ValueError(f"{path}: duplicate key {k}")
        if k in COUNTS:
            words = v.split(",")
            if len(words) != COUNTS[k]:
                raise ValueError(f"{path}: {k} has {len(words)} words, expected {COUNTS[k]}")
            values[k] = [int(word) for word in words]
        elif k in ("actual_twl", "recent_limit", "image_buffer_policy", "bg_cache_enabled"):
            values[k] = int(v)
        else:
            raise ValueError(f"{path}: unrecognized field {k}")
    mandatory = set(COUNTS) | {"actual_twl", "recent_limit", "image_buffer_policy", "bg_cache_enabled"}
    if set(values) != mandatory:
        raise ValueError(f"{path}: missing fields {sorted(mandatory - set(values))}")
    if values["load_bench"][0] != 0x4c424e43:
        raise ValueError(f"{path}: wrong load_bench signature")
    if values["load_bench"][1] != 1 or values["load_bench"][2] != 0:
        raise ValueError(f"{path}: benchmark did not complete successfully (code={values['load_bench'][2]})")
    if values["load_bench"][3] != 32:
        raise ValueError(f"{path}: benchmark completed {values['load_bench'][3]}/32 loads")
    if values["actual_twl"] not in (0, 1) or values["image_buffer_policy"] not in range(4):
        raise ValueError(f"{path}: invalid runtime or buffer policy")
    return values


def format_us(v):
    return f"{v / 1000:,.3f} ms"


def compare(a, b, label_a, label_b):
    if a["actual_twl"] != b["actual_twl"]:
        raise ValueError("Refusing to compare different NTR/TWL runtime modes")
    if a["load_bench"][12:16] != b["load_bench"][12:16]:
        raise ValueError("Final frame hashes differ: rendering output changed")
    if a["load_bench"][28:32] != b["load_bench"][28:32]:
        raise ValueError("Across-eight-pass frame hashes differ: rendering output changed")
    differing_cache = (a["recent_limit"] == 0) != (b["recent_limit"] == 0)
    differing_buffer = a["image_buffer_policy"] != b["image_buffer_policy"]
    if differing_cache and differing_buffer:
        raise ValueError("Cache and buffer changed together: confounded A/B, split the experiments")
    if differing_cache:
        kind = "cache on/off"
        if a["bg_cache_enabled"] != b["bg_cache_enabled"]:
            raise ValueError("Background-cache policy differed: confounded cache A/B")
    elif differing_buffer and a["recent_limit"] == b["recent_limit"] and a["bg_cache_enabled"] == b["bg_cache_enabled"]:
        kind = "file buffering"
    else:
        raise ValueError("No isolated cache/buffer policy difference")
    lines = [
        f"CPyMO NDS loadbench: {kind} A/B (actual runtime: {'TWL' if a['actual_twl'] else 'NTR'})",
        f"A: {label_a} | buffer={a['image_buffer_policy']} cache_budget={a['recent_limit']} bytes",
        f"B: {label_b} | buffer={b['image_buffer_policy']} cache_budget={b['recent_limit']} bytes",
    ]
    for i, name in enumerate(ASSETS):
        avg_a, avg_b = a["load_bench"][4+i] / 8, b["load_bench"][4+i] / 8
        delta = (avg_b / avg_a - 1) * 100 if avg_a else float("nan")
        lines.append(
            f"  {name}: avg {format_us(avg_a)} -> {format_us(avg_b)} "
            f"({delta:+.1f}%); first {format_us(a['load_bench'][32+i])} -> "
            f"{format_us(b['load_bench'][32+i])}; warm max "
            f"{format_us(a['load_bench'][36+i])} -> {format_us(b['load_bench'][36+i])}"
        )
    lines.extend([
        f"LRU cache hits/misses/evictions: {a['load_state'][9:12]} -> {b['load_state'][9:12]}",
        f"Max audio service gap: {format_us(a['load_bench'][16])} -> {format_us(b['load_bench'][16])}",
        f"Heap peak (lifetime, not benchmark-only): {a['engine_state'][8]} -> {b['engine_state'][8]} bytes",
        f"Observed allocation failures: {a['engine_state'][9]} -> {b['engine_state'][9]}",
        "These numbers include load+draw, not loading alone. Audio gap is not underrun proof.",
        "The screenshot hashes matched. Still check visible screens/audio on actual DS/DSi.",
    ])
    return "\n".join(lines)


def mock_log(cache_limit, policy, speed):
    arr = {key: [0] * n for key, n in COUNTS.items()}
    t = arr["load_bench"]
    t[0], t[1], t[2], t[3] = 0x4c424e43, 1, 0, 32
    for i in range(4):
        t[4+i] = speed * (i+1) * 8
        t[8+i] = speed * (i+1)
        t[12+i] = 0x12340000 + i
        t[28+i] = 0x56780000 + i
        t[32+i] = speed * (i+1)
        t[36+i] = speed * (i+1)
    t[16] = 21000
    arr["load_state"][9:12] = [10 if cache_limit else 0, 12, 0]
    arr["engine_state"][8] = 320000
    meta = dict(actual_twl=0, recent_limit=cache_limit, image_buffer_policy=policy, bg_cache_enabled=0)
    return HEADER + "\n" + "\n".join(
        f"{k}={v}" for k, v in meta.items()
    ) + "\n" + "\n".join(
        f"{k}={','.join(map(str, v))}" for k, v in arr.items()
    ) + "\n"


def self_test():
    with tempfile.TemporaryDirectory() as temp:
        a, b = Path(temp)/"a.txt", Path(temp)/"b.txt"
        a.write_text(mock_log(0, 2, 20000), encoding="utf-8")
        b.write_text(mock_log(384*1024, 2, 10000), encoding="utf-8")
        ra, rb = read_log(a), read_log(b)
        assert "cache on/off" in compare(ra, rb, "a", "b")
        rb["load_bench"][28] ^= 1
        try:
            compare(ra, rb, "a", "b")
        except ValueError as e:
            assert "frame hashes" in str(e)
        else:
            raise AssertionError("Mismatch did not fail")
        rb["load_bench"][28] ^= 1
        rb["image_buffer_policy"] = 1
        try:
            compare(ra, rb, "a", "b")
        except ValueError as e:
            assert "confounded" in str(e)
        else:
            raise AssertionError("Confounded A/B did not fail")
    print("self-test passed (synthetic log validation only)")


if __name__ == "__main__":
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("a", nargs="?")
    p.add_argument("b", nargs="?")
    p.add_argument("--self-test", action="store_true")
    args = p.parse_args()
    if args.self_test:
        self_test()
    elif args.a and args.b:
        try:
            print(compare(read_log(args.a), read_log(args.b), args.a, args.b))
        except (ValueError, OSError) as error:
            p.exit(1, f"Comparison rejected: {error}\n")
    else:
        p.error("provide two log paths or --self-test")
