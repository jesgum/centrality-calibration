import subprocess
import os
from concurrent.futures import ThreadPoolExecutor, as_completed

DIRECTORIES_FILE = "directories.txt"
RUN_NUMBERS_FILE = "runNumbers.txt"

DUMMY_DIR = "RAW_COPY_FROM_HY_SUBMITTED_JOBS_OUTPUT_DIRECTORY"
DUMMY_RUN = "RAW_COPY_FROM_HY_SUBMITTED_JOBS_RUN_NO"

def parse_file(filepath):
    with open(filepath, "r") as f:
        content = f.read()
    return [x.strip() for x in content.split(",") if x.strip()]

def download_file(run_path, run_num):
    alien_path = f"{run_path}/AnalysisResults.root"
    local_file = f"results/AR_{run_num}.root"

    cmd = ["alien_cp", "-q", alien_path, f"file:{local_file}"]

    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
        if result.returncode == 0:
            return (run_num, True, None)
        else:
            return (run_num, False, result.stderr.strip())
    except subprocess.TimeoutExpired:
        return (run_num, False, "Timeout after 300s")
    except Exception as e:
        return (run_num, False, str(e))

runlist = parse_file(DIRECTORIES_FILE)
run_numbers = parse_file(RUN_NUMBERS_FILE)

if DUMMY_DIR in runlist:
    print(f"Error: {DIRECTORIES_FILE} still contains dummy value '{DUMMY_DIR}'. Please replace with real directories.")
    exit(1)

if DUMMY_RUN in run_numbers:
    print(f"Error: {RUN_NUMBERS_FILE} still contains dummy value '{DUMMY_RUN}'. Please replace with real run numbers.")
    exit(1)

if len(runlist) != len(run_numbers):
    print(f"Error: {len(runlist)} directories but {len(run_numbers)} run numbers - must match.")
    exit(1)

os.makedirs("results", exist_ok=True)
print(f"Found {len(runlist)} runs to download.\n")

failed = []
with ThreadPoolExecutor(max_workers=8) as executor:
    futures = {
        executor.submit(download_file, path, run_num): run_num
        for path, run_num in zip(runlist, run_numbers)
    }

    for future in as_completed(futures):
        run_num, success, error = future.result()
        if success:
            print(f" ✓ AR_{run_num}.root")
        else:
            print(f" ✗ AR_{run_num}.root  —  {error}")
            failed.append(run_num)

print(f"\nDone. {len(runlist) - len(failed)}/{len(runlist)} succeeded.")
if failed:
    print(f"Failed runs: {failed}")