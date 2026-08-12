#!/bin/bash

# jobLock_Analysis_559843_hNMFTTracks_2 jobLock_Analysis_560031_hNMFTTracks_2 jobLock_Analysis_560049_hNMFTTracks_2 jobLock_Analysis_560066_hNMFTTracks_2 jobLock_Analysis_560123_hNMFTTracks_2 jobLock_Analysis_560184_hNMFTTracks_2

echo "Will now run all relevant calibrations"

FILES=$(ls ../results/*glauberNBD_ancestorMode2_fixedK_fixedMu_hFT0C_BCs.root)
# FILES=$(ls ../results/*glauberNBD*.root)


RUN="111111"
EST="FT0A Amplitude"
ANCHOR=90
GLOBAL_NORM=kTRUE

for FILE in ${FILES}; do
  echo "Processing file ${FILE}"
  BASE=$(basename "$FILE" .root)
  RUN=$(echo "$BASE" | sed 's/AR_\([0-9]*\)_.*/\1/')
  if echo "$BASE" | grep -q '_BCs$'; then
    HISTTYPE=$(echo "$BASE" | sed 's/.*_fixedMu_h\(.*\)_BCs/\1/')
    EST="${HISTTYPE} Amplitude"
  else
    HISTTYPE=$(echo "$BASE" | sed 's/.*_fixedMu_hN\(.*\)/\1/')
    EST="$HISTTYPE"
  fi

  root.exe -q -b "../macros/runCalibration.cc(\"${FILE}\",${ANCHOR},${GLOBAL_NORM})"
  root.exe -q -b "../macros/drawSummaryPlots.cc(\"${FILE}\",\"${RUN}\",\"${EST}\",${GLOBAL_NORM})"
done
