#!/usr/bin/env python3
"""Coverage ratchet for the wildebeest test suite.

Compares a gcovr --json-summary file against tests/coverage/thresholds.json.

  * Fails (exit 1) if line or branch coverage is below the stored floor.
  * Warns if coverage is more than SLACK points above the floor, as a
    reminder to raise the floor in the same change that added the tests.

Usage: check_ratchet.py <gcovr-summary.json> [thresholds.json]
"""

import json
import os
import sys

SLACK = 1.0


def main(argv):
    if len(argv) not in (2, 3):
        print(__doc__, file=sys.stderr)
        return 2
    summary_path = argv[1]
    thresholds_path = argv[2] if len(argv) == 3 else os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "thresholds.json")

    with open(summary_path) as f:
        summary = json.load(f)
    with open(thresholds_path) as f:
        floor = json.load(f)

    in_gha = os.environ.get("GITHUB_ACTIONS") == "true"
    failed = False
    for metric in ("line", "branch"):
        actual = float(summary[f"{metric}_percent"])
        minimum = float(floor[metric])
        print(f"{metric:6}: {actual:5.1f}%  (floor {minimum:5.1f}%)")
        if actual < minimum:
            failed = True
            msg = (f"{metric} coverage {actual:.1f}% is below the floor "
                   f"{minimum:.1f}% in tests/coverage/thresholds.json")
            print(f"::error::{msg}" if in_gha else f"ERROR: {msg}")
        elif actual > minimum + SLACK:
            msg = (f"{metric} coverage {actual:.1f}% is more than {SLACK} "
                   f"points above the floor {minimum:.1f}%; raise it in "
                   f"tests/coverage/thresholds.json")
            print(f"::warning::{msg}" if in_gha else f"WARNING: {msg}")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
