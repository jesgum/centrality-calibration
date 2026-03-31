#!/bin/bash

# o2-analysis-centrality-study -b --configuration json://configuration.json --aod-file "@InputData_560123.txt"
o2-analysis-centrality-study -b --configuration json://configuration.json --pipeline centrality-study:16 --shm-segment-size 6442450944 --aod-file "@InputData_560090.txt"
mv  AnalysisResults.root AnalysisResults_560090.root

o2-analysis-centrality-study -b --configuration json://configuration.json --pipeline centrality-study:16 --shm-segment-size 6442450944 --aod-file "@InputData_560123.txt"
mv  AnalysisResults.root AnalysisResults_560123.root
