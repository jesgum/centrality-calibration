#!/bin/bash

root.exe -q -b drawGlauberPbPb.C\(\"AR_560123_calibration_hFV0A_BCs.root\"\,\"AR_560123_hFV0A_BCs.pdf\"\,\180000\)&
root.exe -q -b drawGlauberPbPb.C\(\"AR_560123_calibration_hFT0C_BCs.root\"\,\"AR_560123_hFT0C_BCs.pdf\"\,\60000\)&
root.exe -q -b drawGlauberPbPb.C\(\"AR_560123_calibration_hFT0C_BCs_variants1.root\"\,\"AR_560123_hFT0C_BCs_variants1.pdf\"\,\60000\)&
root.exe -q -b drawGlauberPbPb.C\(\"AR_560123_calibration_hFT0C_BCs_variants2.root\"\,\"AR_560123_hFT0C_BCs_variants2.pdf\"\,\60000\)&
root.exe -q -b drawGlauberPbPb.C\(\"AR_560123_calibration_hFT0M_BCs.root\"\,\"AR_560123_hFT0M_BCs.pdf\"\,\180000\)&
root.exe -q -b drawGlauberPbPb.C\(\"AR_560123_calibration_hNPVContributors.root\"\,\"AR_560123_hNPVContributors.pdf\"\,\5000\)&
root.exe -q -b drawGlauberPbPb.C\(\"AR_560123_calibration_hNMFTTracks.root\"\,\"AR_560123_hNMFTTracks.pdf\"\,\5000\)&
root.exe -q -b drawGlauberPbPb.C\(\"AR_560123_calibration_hNGlobalTracks.root\"\,\"AR_560123_hNGlobalTracks.pdf\"\,\5000\)&
root.exe -q -b drawGlauberPbPb.C\(\"AR_560090_calibration_hFV0A_BCs.root\"\,\"AR_560090_hFV0A_BCs.pdf\"\,\180000\)&
root.exe -q -b drawGlauberPbPb.C\(\"AR_560090_calibration_hFT0C_BCs.root\"\,\"AR_560090_hFT0C_BCs.pdf\"\,\60000\)&
root.exe -q -b drawGlauberPbPb.C\(\"AR_560090_calibration_hFT0C_BCs_variants1.root\"\,\"AR_560090_hFT0C_BCs_variants1.pdf\"\,\60000\)&
root.exe -q -b drawGlauberPbPb.C\(\"AR_560090_calibration_hFT0C_BCs_variants2.root\"\,\"AR_560090_hFT0C_BCs_variants2.pdf\"\,\60000\)&
root.exe -q -b drawGlauberPbPb.C\(\"AR_560090_calibration_hFT0M_BCs.root\"\,\"AR_560090_hFT0M_BCs.pdf\"\,\180000\)&
root.exe -q -b drawGlauberPbPb.C\(\"AR_560090_calibration_hNPVContributors.root\"\,\"AR_560090_hNPVContributors.pdf\"\,\5000\)&
root.exe -q -b drawGlauberPbPb.C\(\"AR_560090_calibration_hNMFTTracks.root\"\,\"AR_560090_hNMFTTracks.pdf\"\,\5000\)&
root.exe -q -b drawGlauberPbPb.C\(\"AR_560090_calibration_hNGlobalTracks.root\"\,\"AR_560090_hNGlobalTracks.pdf\"\,\5000\)&

wait
echo "done"