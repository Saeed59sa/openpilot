#!/usr/bin/env python3
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SCHEMA = ROOT / "cereal/log.capnp"

enum_re = re.compile(r"^\s*\w+\s*@(?P<num>\d+);")
ordinals = []
inside = False
with SCHEMA.open() as f:
    for line in f:
        line = line.split('#', 1)[0]
        stripped = line.strip()
        if not inside:
            if stripped.startswith("enum EventName"):
                inside = True
            continue
        if stripped.startswith("}"):
            break
        m = enum_re.match(stripped)
        if m:
            ordinals.append(int(m.group('num')))

expected = list(range(len(ordinals)))
if ordinals != expected:
    for i, (actual, exp) in enumerate(zip(ordinals, expected, strict=False)):
        if actual != exp:
            print(f"ordinal mismatch at position {i}: expected {exp}, got {actual}")
    if len(ordinals) != len(set(ordinals)):
        print("duplicate ordinals detected")
    sys.exit(1)
