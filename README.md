# centrality
Repository to do centrality calibrations

## 1.
- Copy HY run and output directories (run by run) from the 'submitted jobs' tab and paste the output in `runNumbers.txt` and `directories.txt`
- Todo: Do local merging if the HY run merge failed

## 2.
- Download the HY output with `download.py`
- No additional configuration needed

## 3.
- Run all the glauber fits using `runMassAnalysis.sh`
- Set number of jobs (default 18)
- Very time consuming (~hrs)

## 4.