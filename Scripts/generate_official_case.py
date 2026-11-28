#!/usr/bin/env python3
# SPDX-License-Identifier: CC0-1.0
"""Call the pinned Library Checker generator for one case at a time."""

from pathlib import Path
import runpy
import sys


def main() -> None:
    checkout = Path(sys.argv[1])
    directory = Path(sys.argv[2])
    action = sys.argv[3]
    sys.path.insert(0, str(checkout))
    generator = runpy.run_path(str(checkout / "generate.py"))
    problem = generator["Problem"](checkout, directory)
    if action == "prepare":
        problem.health_check()
        problem.generate_params_h()
        problem.compile_checker()
        problem.compile_correct()
        problem.compile_gens()
        return

    if action not in ("input", "case"):
        raise ValueError("unknown generation action")
    name = sys.argv[4]
    for test in problem.config["tests"]:
        filename = test["name"]
        for seed in range(test["number"]):
            if generator["casename"](filename, seed) != name:
                continue
            source = directory / "in" / (name + ".in")
            data = generator["execcmd"](directory / "gen" / filename, [str(seed)])
            generator["check_call_to_file"](data, source)
            if action == "case":
                expected = directory / "out" / (name + ".out")
                solution = generator["execcmd"](directory / "sol" / "correct.cpp")
                with source.open("rb") as stream:
                    generator["check_call_to_file"](solution, expected, stdin=stream)
            return
    raise ValueError("unknown official case")


if __name__ == "__main__":
    main()
