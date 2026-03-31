#!/bin/bash

#  args
#  runGlauberFit(inputFileName, histogramName, ancestorMode, freeK, use_dMu_dAnc, f_value, outputFileName)

# root.exe -q -b runGlauberFit.C\(\"AnalysisResults_560123.root\"\,\"hFV0A_BCs\"\,\2\,\20\,\"AR_560123_glauber_hFV0A_BCs.root\"\)&
# root.exe -q -b runGlauberFit.C\(\"AnalysisResults_560123.root\"\,\"hFT0C_BCs\"\,\2\,\20\,\"AR_560123_glauber_hFT0C_BCs.root\"\)&
# root.exe -q -b runGlauberFit.C\(\"AnalysisResults_560123.root\"\,\"hFT0C_BCs\"\,\0\,\20\,\"AR_560123_glauber_hFT0C_BCs_variant.root\"\)&
# root.exe -q -b runGlauberFit.C\(\"AnalysisResults_560123.root\"\,\"hFT0M_BCs\"\,\2\,\20\,\"AR_560123_glauber_hFT0M_BCs.root\"\)&
root.exe -q -b runGlauberFit.C\(\"AnalysisResults_560123.root\"\,\"hNPVContributors\"\,\2\,\1\,\"AR_560123_glauber_hNPVContributors.root\"\)&
root.exe -q -b runGlauberFit.C\(\"AnalysisResults_560123.root\"\,\"hNMFTTracks\"\,\2\,\1\,\"AR_560123_glauber_hNMFTTracks.root\"\)&
root.exe -q -b runGlauberFit.C\(\"AnalysisResults_560123.root\"\,\"hNGlobalTracks\"\,\2\,\1\,\"AR_560123_glauber_hNGlobalTracks.root\"\)&

# root.exe -q -b runGlauberFit.C\(\"AnalysisResults_560090.root\"\,\"hFV0A_BCs\"\,\2\,\20\,\"AR_560090_glauber_hFV0A_BCs.root\"\)&
# root.exe -q -b runGlauberFit.C\(\"AnalysisResults_560090.root\"\,\"hFT0C_BCs\"\,\2\,\20\,\"AR_560090_glauber_hFT0C_BCs.root\"\)&
# root.exe -q -b runGlauberFit.C\(\"AnalysisResults_560090.root\"\,\"hFT0C_BCs\"\,\0\,\20\,\"AR_560090_glauber_hFT0C_BCs_variant.root\"\)&
# root.exe -q -b runGlauberFit.C\(\"AnalysisResults_560090.root\"\,\"hFT0M_BCs\"\,\2\,\20\,\"AR_560090_glauber_hFT0M_BCs.root\"\)&
root.exe -q -b runGlauberFit.C\(\"AnalysisResults_560090.root\"\,\"hNPVContributors\"\,\2\,\1\,\"AR_560090_glauber_hNPVContributors.root\"\)&
root.exe -q -b runGlauberFit.C\(\"AnalysisResults_560090.root\"\,\"hNMFTTracks\"\,\2\,\1\,\"AR_560090_glauber_hNMFTTracks.root\"\)&
root.exe -q -b runGlauberFit.C\(\"AnalysisResults_560090.root\"\,\"hNGlobalTracks\"\,\2\,\1\,\"AR_560090_glauber_hNGlobalTracks.root\"\)&

wait
echo done