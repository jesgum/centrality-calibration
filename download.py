import subprocess
import os
import shutil
from concurrent.futures import ThreadPoolExecutor, as_completed

DIRECTORIES_FILE = "directories.txt"
RUN_NUMBERS_FILE = "runNumbers.txt"
DUMMY_DIR = "RAW_COPY_FROM_HY_SUBMITTED_JOBS_OUTPUT_DIRECTORY"
DUMMY_RUN = "RAW_COPY_FROM_HY_SUBMITTED_JOBS_RUN_NO"
TRY_MANUAL_MERGE_ON_FAILURE = True


def parse_file(filepath):
    with open(filepath, "r") as f:
        content = f.read()
    return [x.strip() for x in content.split(",") if x.strip()]


def alien_ls(alien_path):
    """List contents of an AliEn directory. Returns list of entries or raises on failure."""
    cmd = ["alien_ls", alien_path]
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
    if result.returncode != 0:
        raise RuntimeError(f"alien_ls failed on {alien_path}: {result.stderr.strip()}")
    entries = [e.strip() for e in result.stdout.splitlines() if e.strip()]
    return entries


def download_single(alien_path, local_path):
    """Download one file from AliEn. Returns (success, error_message)."""
    cmd = ["alien_cp", "-q", alien_path, f"file:{local_path}"]
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
    if result.returncode == 0:
        return True, None
    return False, result.stderr.strip()


def download_file(run_path, run_num):
    """
    Primary download attempt: grab AnalysisResults.root directly from the run path.
    Returns (run_num, success, error).
    """
    alien_path = f"{run_path}/AnalysisResults.root"
    local_file = f"ARs/AR_{run_num}.root"
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


def manual_merge(run_path, run_num):
    """
    Fallback merge strategy:
      1. alien_ls the run path to find sub-job directories.
      2. For each sub-job directory, download its AnalysisResults.root into a
         temporary local folder with a unique filename.
      3. Merge all downloaded files with `hadd` into the final output file.

    Returns (run_num, success, error).
    """
    tmp_dir = f"ARs/tmp_{run_num}"
    os.makedirs(tmp_dir, exist_ok=True)

    try:
        # --- Step 1: list sub-job directories ---
        try:
            subdirs = alien_ls(run_path)
        except Exception as e:
            return (run_num, False, f"alien_ls failed: {e}")

        if not subdirs:
            return (run_num, False, "alien_ls returned no subdirectories")

        # --- Step 2: download each sub-job's AnalysisResults.root ---
        downloaded = []
        download_errors = []

        for subdir in subdirs:
            alien_file = f"{run_path}/{subdir}/AnalysisResults.root"
            # Sanitise the subdir name so it's safe as a filename component
            safe_name = subdir.replace("/", "_").strip("_") or "job"
            local_file = os.path.join(tmp_dir, f"AR_{run_num}_{safe_name}.root")

            try:
                ok, err = download_single(alien_file, local_file)
                if ok:
                    downloaded.append(local_file)
                else:
                    download_errors.append(f"{subdir}: {err}")
            except subprocess.TimeoutExpired:
                download_errors.append(f"{subdir}: timeout")
            except Exception as e:
                download_errors.append(f"{subdir}: {e}")

        if not downloaded:
            return (
                run_num,
                False,
                f"No sub-job files downloaded. Errors: {'; '.join(download_errors)}",
            )

        if download_errors:
            print(
                f"   [merge {run_num}] {len(downloaded)} files downloaded, "
                f"{len(download_errors)} sub-jobs failed: {'; '.join(download_errors)}"
            )

        # --- Step 3: merge with hadd ---
        output_file = f"ARs/AR_{run_num}.root"
        hadd_cmd = ["hadd", "-f", output_file] + downloaded
        try:
            hadd_result = subprocess.run(
                hadd_cmd, capture_output=True, text=True, timeout=600
            )
            if hadd_result.returncode != 0:
                return (
                    run_num,
                    False,
                    f"hadd failed: {hadd_result.stderr.strip()}",
                )
        except subprocess.TimeoutExpired:
            return (run_num, False, "hadd timed out after 600s")
        except FileNotFoundError:
            return (run_num, False, "hadd not found — is ROOT available in PATH?")

        return (run_num, True, None)

    finally:
        # Always clean up the temporary directory
        shutil.rmtree(tmp_dir, ignore_errors=True)


def download_or_merge(run_path, run_num):
    """
    Try the direct download first. If it fails and TRY_MANUAL_MERGE_ON_FAILURE
    is set, fall back to the sub-job merge strategy.
    """
    run_num, success, error = download_file(run_path, run_num)

    if success or not TRY_MANUAL_MERGE_ON_FAILURE:
        return run_num, success, error

    print(f"   [fallback] Direct download failed for {run_num}: {error}. Trying manual merge...")
    return manual_merge(run_path, run_num)


if __name__ == "__main__":
    runlist = parse_file(DIRECTORIES_FILE)
    run_numbers = parse_file(RUN_NUMBERS_FILE)

    if DUMMY_DIR in runlist:
        print(
            f"Error: {DIRECTORIES_FILE} still contains dummy value '{DUMMY_DIR}'. "
            "Please replace with real directories."
        )
        exit(1)

    if DUMMY_RUN in run_numbers:
        print(
            f"Error: {RUN_NUMBERS_FILE} still contains dummy value '{DUMMY_RUN}'. "
            "Please replace with real run numbers."
        )
        exit(1)

    if len(runlist) != len(run_numbers):
        print(
            f"Error: {len(runlist)} directories but {len(run_numbers)} run numbers — must match."
        )
        exit(1)

    os.makedirs("results", exist_ok=True)
    print(f"Found {len(runlist)} runs to download.\n")

    failed = []
    with ThreadPoolExecutor(max_workers=8) as executor:
        futures = {
            executor.submit(download_or_merge, path, run_num): run_num
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