import subprocess
import os
import shutil
from concurrent.futures import ThreadPoolExecutor, as_completed

# --- Which files to download ---
DOWNLOAD_ANALYSIS_RESULTS = False   # AnalysisResults.root
DOWNLOAD_AO2D             = True   # AO2D.root

DIRECTORIES_FILE = "directories.txt"
RUN_NUMBERS_FILE = "runNumbers.txt"
DUMMY_DIR = "RAW_COPY_FROM_HY_SUBMITTED_JOBS_OUTPUT_DIRECTORY"
DUMMY_RUN = "RAW_COPY_FROM_HY_SUBMITTED_JOBS_RUN_NO"
TRY_MANUAL_MERGE_ON_FAILURE = True

# Derived list of (filename, tag, output_folder) triples for the enabled targets.
#   filename      — name on AliEn
#   tag           — prefix for local files, e.g. AR_<run>.root / AO2D_<run>.root
#   output_folder — local directory to download into
_TARGETS: list[tuple[str, str, str]] = []
if DOWNLOAD_ANALYSIS_RESULTS:
    _TARGETS.append(("AnalysisResults.root", "AR",   "ARs"))

if DOWNLOAD_AO2D:
    _TARGETS.append(("AO2D.root",            "AO2D", "AO2Ds"))


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


def download_file(run_path, run_num, filename, tag, folder):
    """
    Primary download attempt: grab `filename` directly from the run path.
    Returns (run_num, tag, folder, success, error).
    """
    alien_path = f"{run_path}/{filename}"
    local_file = f"{folder}/{tag}_{run_num}.root"
    try:
        result = subprocess.run(
            ["alien_cp", "-q", alien_path, f"file:{local_file}"],
            capture_output=True, text=True, timeout=300,
        )
        if result.returncode == 0:
            return (run_num, tag, folder, True, None)
        return (run_num, tag, folder, False, result.stderr.strip())
    except subprocess.TimeoutExpired:
        return (run_num, tag, folder, False, "Timeout after 300s")
    except Exception as e:
        return (run_num, tag, folder, False, str(e))


def manual_merge(run_path, run_num, filename, tag, folder):
    """
    Fallback merge strategy:
      1. alien_ls the run path to find sub-job directories.
      2. For each sub-job directory, download its `filename` into a
         temporary local folder with a unique name.
      3. Merge all downloaded files with `hadd` into the final output file.

    Returns (run_num, tag, folder, success, error).
    """
    tmp_dir = f"{folder}/tmp_{tag}_{run_num}"
    os.makedirs(tmp_dir, exist_ok=True)

    try:
        # --- Step 1: list sub-job directories ---
        try:
            subdirs = alien_ls(run_path)
        except Exception as e:
            return (run_num, tag, folder, False, f"alien_ls failed: {e}")

        if not subdirs:
            return (run_num, tag, folder, False, "alien_ls returned no subdirectories")

        # --- Step 2: download each sub-job's file ---
        downloaded = []
        download_errors = []

        for subdir in subdirs:
            alien_file = f"{run_path}/{subdir}/{filename}"
            safe_name = subdir.replace("/", "_").strip("_") or "job"
            local_file = os.path.join(tmp_dir, f"{tag}_{run_num}_{safe_name}.root")

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
                run_num, tag, folder, False,
                f"No sub-job files downloaded. Errors: {'; '.join(download_errors)}",
            )

        if download_errors:
            print(
                f"   [merge {tag} {run_num}] {len(downloaded)} files downloaded, "
                f"{len(download_errors)} sub-jobs failed: {'; '.join(download_errors)}"
            )

        # --- Step 3: merge with hadd ---
        output_file = f"{folder}/{tag}_{run_num}.root"
        hadd_cmd = ["hadd", "-f", output_file] + downloaded
        try:
            hadd_result = subprocess.run(
                hadd_cmd, capture_output=True, text=True, timeout=600
            )
            if hadd_result.returncode != 0:
                return (run_num, tag, folder, False, f"hadd failed: {hadd_result.stderr.strip()}")
        except subprocess.TimeoutExpired:
            return (run_num, tag, folder, False, "hadd timed out after 600s")
        except FileNotFoundError:
            return (run_num, tag, folder, False, "hadd not found — is ROOT available in PATH?")

        return (run_num, tag, folder, True, None)

    finally:
        shutil.rmtree(tmp_dir, ignore_errors=True)


def download_or_merge(run_path, run_num, filename, tag, folder):
    """
    Try the direct download first. If it fails and TRY_MANUAL_MERGE_ON_FAILURE
    is set, fall back to the sub-job merge strategy.
    """
    run_num, tag, folder, success, error = download_file(run_path, run_num, filename, tag, folder)

    if success or not TRY_MANUAL_MERGE_ON_FAILURE:
        return run_num, tag, folder, success, error

    print(
        f"   [fallback] Direct download failed for {tag}_{run_num}: {error}. "
        "Trying manual merge..."
    )
    return manual_merge(run_path, run_num, filename, tag, folder)


if __name__ == "__main__":
    if not _TARGETS:
        print("Error: both DOWNLOAD_ANALYSIS_RESULTS and DOWNLOAD_AO2D are False. "
              "Nothing to do.")
        exit(1)

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

    for _, _, folder in _TARGETS:
        os.makedirs(folder, exist_ok=True)

    target_desc = " + ".join(f for f, _, _ in _TARGETS)
    total_jobs = len(runlist) * len(_TARGETS)
    print(f"Downloading {target_desc} for {len(runlist)} runs "
          f"({total_jobs} total files).\n")

    failed = []
    with ThreadPoolExecutor(max_workers=8) as executor:
        futures = {
            executor.submit(download_or_merge, path, run_num, filename, tag, folder): (run_num, tag)
            for path, run_num in zip(runlist, run_numbers)
            for filename, tag, folder in _TARGETS
        }

        for future in as_completed(futures):
            run_num, tag, folder, success, error = future.result()
            local_name = f"{folder}/{tag}_{run_num}.root"
            if success:
                print(f" ✓ {local_name}")
            else:
                print(f" ✗ {local_name}  —  {error}")
                failed.append(local_name)

    print(f"\nDone. {total_jobs - len(failed)}/{total_jobs} succeeded.")
    if failed:
        print(f"Failed files: {failed}")