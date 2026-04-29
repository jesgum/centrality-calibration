#!/bin/bash

# jobLock_Analysis_559843_hNMFTTracks_2 jobLock_Analysis_560031_hNMFTTracks_2 jobLock_Analysis_560049_hNMFTTracks_2 jobLock_Analysis_560066_hNMFTTracks_2 jobLock_Analysis_560123_hNMFTTracks_2 jobLock_Analysis_560184_hNMFTTracks_2

echo "Will now run all relevant calibrations"

# FILES=$(ls results/AR_560169_glauberNBD_ancestorMode*_fixedK_fixedMu_hFT0C_BCs.root)
FILES=$(ls results/*.root)


RUN="111111"
EST="FT0A Amplitude"

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

  root.exe -q -b runCalibration.cc\(\"${FILE}\"\)
  root.exe -q -b "drawSummaryPlots.cc(\"${FILE}\",\"${RUN}\",\"${EST}\")"
done