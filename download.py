import subprocess
import os
import shutil
from concurrent.futures import ThreadPoolExecutor, as_completed

# --- Number of jobs ---
MAX_WORKERS = 8

# --- Which files to download ---
DOWNLOAD_ANALYSIS_RESULTS = True   # AnalysisResults.root
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
    result = subprocess.run(["alien_ls", alien_path], capture_output=True, text=True, timeout=60)
    if result.returncode != 0:
        raise RuntimeError(f"alien_ls failed on {alien_path}: {result.stderr.strip()}")
    return [e.strip() for e in result.stdout.splitlines() if e.strip()]


def download_single(alien_path, local_path):
    """Download one file from AliEn. Returns (success, error_message)."""
    result = subprocess.run(
        ["alien_cp", "-q", alien_path, f"file:{local_path}"],
        capture_output=True, text=True, timeout=300,
    )
    if result.returncode == 0:
        return True, None
    return False, result.stderr.strip()


def download_file(run_path, run_num, filename, tag, folder):
    """
    Primary download attempt: grab `filename` directly from the run path.
    Returns (success, error).
    """
    alien_path = f"{run_path}/{filename}"
    local_file  = f"{folder}/{tag}_{run_num}.root"
    try:
        result = subprocess.run(
            ["alien_cp", "-q", alien_path, f"file:{local_file}"],
            capture_output=True, text=True, timeout=300,
        )
        if result.returncode == 0:
            return True, None
        return False, result.stderr.strip()
    except subprocess.TimeoutExpired:
        return False, "Timeout after 300s"
    except Exception as e:
        return False, str(e)


def download_subjob(run_path, subdir, run_num, filename, tag, tmp_dir):
    """
    Download one sub-job file into tmp_dir.
    Returns (local_path, error) — local_path is None on failure.
    """
    alien_file = f"{run_path}/{subdir}/{filename}"
    safe_name  = subdir.replace("/", "_").strip("_") or "job"
    local_file = os.path.join(tmp_dir, f"{tag}_{run_num}_{safe_name}.root")
    try:
        ok, err = download_single(alien_file, local_file)
        return (local_file, None) if ok else (None, f"{subdir}: {err}")
    except subprocess.TimeoutExpired:
        return None, f"{subdir}: timeout"
    except Exception as e:
        return None, f"{subdir}: {e}"


def run_hadd(output_file, input_files, run_num, tag, folder):
    """
    Merge input_files into output_file with hadd, then clean up the tmp dir.
    Returns (run_num, tag, folder, success, error).
    """
    tmp_dir = os.path.dirname(input_files[0])
    try:
        try:
            result = subprocess.run(
                ["hadd", "-f", output_file] + input_files,
                capture_output=True, text=True, timeout=600,
            )
            if result.returncode != 0:
                return run_num, tag, folder, False, f"hadd failed: {result.stderr.strip()}"
        except subprocess.TimeoutExpired:
            return run_num, tag, folder, False, "hadd timed out after 600s"
        except FileNotFoundError:
            return run_num, tag, folder, False, "hadd not found — is ROOT available in PATH?"
        return run_num, tag, folder, True, None
    finally:
        shutil.rmtree(tmp_dir, ignore_errors=True)


if __name__ == "__main__":
    if not _TARGETS:
        print("Error: both DOWNLOAD_ANALYSIS_RESULTS and DOWNLOAD_AO2D are False. "
              "Nothing to do.")
        exit(1)

    runlist     = parse_file(DIRECTORIES_FILE)
    run_numbers = parse_file(RUN_NUMBERS_FILE)

    if DUMMY_DIR in runlist:
        print(f"Error: {DIRECTORIES_FILE} still contains dummy value '{DUMMY_DIR}'. "
              "Please replace with real directories.")
        exit(1)
    if DUMMY_RUN in run_numbers:
        print(f"Error: {RUN_NUMBERS_FILE} still contains dummy value '{DUMMY_RUN}'. "
              "Please replace with real run numbers.")
        exit(1)
    if len(runlist) != len(run_numbers):
        print(f"Error: {len(runlist)} directories but {len(run_numbers)} run numbers — must match.")
        exit(1)

    for _, _, folder in _TARGETS:
        os.makedirs(folder, exist_ok=True)

    target_desc = " + ".join(f for f, _, _ in _TARGETS)
    total_jobs  = len(runlist) * len(_TARGETS)
    print(f"Downloading {target_desc} for {len(runlist)} runs ({total_jobs} total files).\n")

    failed = []

    with ThreadPoolExecutor(max_workers=MAX_WORKERS) as executor:

        # ── Phase 1: submit all direct-download jobs ──────────────────────────
        # future → (run_path, run_num, filename, tag, folder)
        direct_futures = {}
        for run_path, run_num in zip(runlist, run_numbers):
            for filename, tag, folder in _TARGETS:
                f = executor.submit(download_file, run_path, run_num, filename, tag, folder)
                direct_futures[f] = (run_path, run_num, filename, tag, folder)

        # ── Phase 2: process completions; fan out sub-job downloads on failure ─
        # For each failed run that enters the merge path we track its sub-job
        # futures so we can collect them and submit hadd once they all finish.
        #
        # pending_merges: (run_num, tag, folder) → {
        #     "output":   str,          # final output path
        #     "futures":  set[Future],  # outstanding subjob futures
        #     "downloaded": [str],      # paths collected so far
        #     "errors":   [str],        # subjob error messages
        # }
        pending_merges = {}

        # future → (run_num, tag, folder)  for sub-job download futures
        subjob_futures = {}

        # future → (run_num, tag, folder)  for hadd futures
        hadd_futures = {}

        # Collect all futures we still need to wait on
        active = set(direct_futures)

        while active:
            done, active = set(), active  # we'll rebuild active below
            # as_completed on a snapshot; re-add unfinished ones each loop
            done_futures = []
            still_waiting = []
            for f in list(active):
                if f.done():
                    done_futures.append(f)
                else:
                    still_waiting.append(f)

            # If nothing finished yet, block until at least one does
            if not done_futures:
                for f in as_completed(still_waiting):
                    done_futures.append(f)
                    break
                still_waiting = [f for f in still_waiting if f not in done_futures]

            active = set(still_waiting)

            for f in done_futures:
                # ── direct download result ────────────────────────────────────
                if f in direct_futures:
                    run_path, run_num, filename, tag, folder = direct_futures.pop(f)
                    success, error = f.result()

                    if success:
                        print(f" ✓ {folder}/{tag}_{run_num}.root")
                        continue

                    if not TRY_MANUAL_MERGE_ON_FAILURE:
                        print(f" ✗ {folder}/{tag}_{run_num}.root  —  {error}")
                        failed.append(f"{folder}/{tag}_{run_num}.root")
                        continue

                    # Direct download failed — fan out sub-job downloads
                    print(f"   [fallback] Direct download failed for "
                          f"{tag}_{run_num}: {error}. Trying manual merge...")

                    try:
                        subdirs = alien_ls(run_path)
                    except Exception as e:
                        print(f" ✗ {folder}/{tag}_{run_num}.root  —  alien_ls failed: {e}")
                        failed.append(f"{folder}/{tag}_{run_num}.root")
                        continue

                    if not subdirs:
                        print(f" ✗ {folder}/{tag}_{run_num}.root  —  "
                              "alien_ls returned no subdirectories")
                        failed.append(f"{folder}/{tag}_{run_num}.root")
                        continue

                    tmp_dir     = f"{folder}/tmp_{tag}_{run_num}"
                    output_file = f"{folder}/{tag}_{run_num}.root"
                    os.makedirs(tmp_dir, exist_ok=True)

                    key = (run_num, tag, folder)
                    pending_merges[key] = {
                        "output":     output_file,
                        "futures":    set(),
                        "downloaded": [],
                        "errors":     [],
                        "tmp_dir":    tmp_dir,
                    }

                    for subdir in subdirs:
                        sf = executor.submit(
                            download_subjob,
                            run_path, subdir, run_num, filename, tag, tmp_dir,
                        )
                        subjob_futures[sf] = key
                        pending_merges[key]["futures"].add(sf)
                        active.add(sf)

                # ── sub-job download result ───────────────────────────────────
                elif f in subjob_futures:
                    key = subjob_futures.pop(f)
                    local_path, error = f.result()
                    merge = pending_merges[key]
                    merge["futures"].discard(f)

                    if local_path:
                        merge["downloaded"].append(local_path)
                    else:
                        merge["errors"].append(error)

                    # All sub-jobs for this run are done — submit hadd
                    if not merge["futures"]:
                        run_num, tag, folder = key
                        downloaded = merge["downloaded"]
                        errors     = merge["errors"]

                        if not downloaded:
                            print(f" ✗ {folder}/{tag}_{run_num}.root  —  "
                                  f"No sub-job files downloaded. "
                                  f"Errors: {'; '.join(errors)}")
                            failed.append(f"{folder}/{tag}_{run_num}.root")
                            shutil.rmtree(merge["tmp_dir"], ignore_errors=True)
                            del pending_merges[key]
                            continue

                        if errors:
                            print(f"   [merge {tag} {run_num}] "
                                  f"{len(downloaded)} files downloaded, "
                                  f"{len(errors)} sub-jobs failed: {'; '.join(errors)}")

                        hf = executor.submit(
                            run_hadd,
                            merge["output"], downloaded, run_num, tag, folder,
                        )
                        hadd_futures[hf] = key
                        active.add(hf)
                        del pending_merges[key]

                # ── hadd result ───────────────────────────────────────────────
                elif f in hadd_futures:
                    key = hadd_futures.pop(f)
                    run_num, tag, folder, success, error = f.result()
                    local_name = f"{folder}/{tag}_{run_num}.root"
                    if success:
                        print(f" ✓ {local_name}  (merged)")
                    else:
                        print(f" ✗ {local_name}  —  {error}")
                        failed.append(local_name)

    print(f"\nDone. {total_jobs - len(failed)}/{total_jobs} succeeded.")
    if failed:
        print(f"Failed files: {failed}")