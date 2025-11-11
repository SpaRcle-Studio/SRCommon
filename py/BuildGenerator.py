import json
import fnmatch
import os
import argparse
from pathlib import Path
from glob import glob
from time import perf_counter


def chunk_list(lst, chunk_size=30):
    return [lst[i : i + chunk_size] for i in range(0, len(lst), chunk_size)]


def generate_unity_build(out_dir: Path, cxx_files: list, cpp_files: list):
    result = ""
    for cxx_file in cxx_files:
        result += f"{str(cxx_file)}\n"

    unity_sources_path = out_dir.joinpath("unity/")

    if not os.path.exists(unity_sources_path):
        os.makedirs(unity_sources_path)

    cpp_files = chunk_list(cpp_files, 30)

    for i in range(len(cpp_files)):
        chunk = cpp_files[i]
        unity_srinc_path = unity_sources_path.joinpath(f"srinc_{i}.cxx")
        unity_srinc_content = ""
        for file in chunk:
            unity_srinc_content += f'#include "{str(file)}"\n'

        with open(unity_srinc_path, "w") as unity_file:
            unity_file.write(unity_srinc_content)

        result += f"{unity_srinc_path}\n"

    with open(out_dir.joinpath("main.srinc"), "w") as output_file:
        output_file.write(result)


def generate_incremental_build(out_dir: Path, cxx_files: list, cpp_files: list):
    files: list = cxx_files + cpp_files
    result = ""
    for file in files:
        result += f"{str(file)}\n"

    with open(out_dir.joinpath("main.srinc"), "w") as output_file:
        output_file.write(result)


def gather_cxx_files(project_root: Path) -> list[str]:
    result = []

    cxx_dir_files = glob(str(project_root.joinpath("*.cxx")))
    if len(cxx_dir_files) > 0:
        result += cxx_dir_files

    result += project_root.joinpath("cxx/").rglob("**.cxx")

    return result


def gather_cpp_files(src_dir: Path, ignore_paths: list) -> list[str]:
    sources = set()
    for file in src_dir.rglob("*.cpp"):
        # normalize path relative to src_dir
        rel_path = file.relative_to(src_dir).as_posix()

        # match against ignore patterns
        ignored = any(
            fnmatch.fnmatchcase(rel_path, pattern)
            or fnmatch.fnmatchcase(f"src/{rel_path}", pattern)
            for pattern in ignore_paths
        )

        if ignored:
            continue

        sources.add(file.resolve())

    return list(sources)


if __name__ == "__main__":
    print("BuildGenerator.py: started")
    timer_start = perf_counter()

    parser = argparse.ArgumentParser(description="SpaRcle Code generator")
    parser.add_argument("--project-path", required=True, help="Path to the project.")
    parser.add_argument(
        "--out", required=True, help="Output directory for the generated files"
    )
    parser.add_argument(
        "--unity",
        required=True,
        choices=["ON", "OFF"],
        help="Generate Unity build files.",
    )

    args = parser.parse_args()

    project_path = Path(args.project_path)
    config_file = project_path.joinpath("srinc.json")

    config = json.load(open(config_file, "r"))
    out_directory = Path(args.out).joinpath(config["module_name"])

    print(
        f"BuildGenerator.py: generating build files for project at:\n\t\t{project_path}"
    )
    print(f"BuildGenerator.py: output directory is:\n\t\t{out_directory}")
    print(f"BuildGenerator.py: unity build is {args.unity}")

    if not os.path.exists(out_directory):
        os.makedirs(out_directory)

    cxx_files = gather_cxx_files(project_path)
    cpp_files = gather_cpp_files(project_path.joinpath("src/"), config["ignore_paths"])

    print(args.unity)

    if args.unity == "ON":
        generate_unity_build(out_directory, cxx_files, cpp_files)
    else:
        generate_incremental_build(out_directory, cxx_files, cpp_files)

    timer_stop = perf_counter()

    print(
        "BuildGenerator.py: finished. Elapsed time: ",
        timer_stop - timer_start,
        " seconds",
    )
