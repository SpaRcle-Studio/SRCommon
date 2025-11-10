import sys, os, argparse
from pathlib import Path
from glob import glob
from time import perf_counter
from collections import defaultdict


def gather_cxx_includes(cxx_dir: Path) -> set:
    includes = set()
    for file in cxx_dir.rglob("**.cxx"):
        with open(file, "r") as f:
            for line in f:
                line = line.strip()
                if line.startswith("#include"):
                    # Extract the included file name
                    parts = line.split()
                    if len(parts) >= 2:
                        include_file = parts[1].strip('"<>')
                        include_file = cxx_dir.joinpath(include_file)
                        include_file = Path(include_file).absolute().resolve()
                        includes.add(include_file)

    return includes


def gather_sources(src_dir: Path) -> set:
    sources = set()
    for file in src_dir.rglob("**.cpp"):
        sources.add(file.absolute())

    return sources


def remove_duplicate_sources(raw_sources: set, cxx_includes: set) -> set:
    filtered_sources = set()
    for source in raw_sources:
        if source not in cxx_includes:
            filtered_sources.add(source)
        else:
            print("CompileCommands.py: excluding source included in cxx files:", source)
    return filtered_sources


if __name__ == "__main__":
    # print("CompileCommands.py: started")
    #
    # parser = argparse.ArgumentParser(description="SpaRcle Code generator")
    # parser.add_argument("--sources", required=True)
    # parser.add_argument(
    #     "--out", required=True, help="Output directory for the generated files"
    # )
    # args = parser.parse_args()
    #
    # if not os.path.exists(args.out):
    #     os.makedirs(args.out)
    #

    print("CompileCommands.py: testing mode.")

    utils_root = Path(os.path.abspath(__file__)).parent.parent

    cxx_includes = gather_cxx_includes(utils_root.joinpath("cxx/"))
    raw_sources = gather_sources(utils_root.joinpath("src/"))

    sources = remove_duplicate_sources(raw_sources, cxx_includes)

    print("CompileCommands.py: finished")
