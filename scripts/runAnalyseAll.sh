#! /bin/bash
COUNTER=0


INPUT_FILE="../runNumbers.txt"
IFS=',' read -ra RUN_NUMBERS < "$INPUT_FILE"
JOBS=18

mkdir -p ../logs
mkdir -p ../results

for FILE in "${RUN_NUMBERS[@]}"; do
  FILE=$(echo $FILE | tr -d '[:space:]')
  while [ $(ls jobLock_Analysis_* | wc -l) -gt ${JOBS} ]; do
    echo "Sleeping. At counter: ${COUNTER}..."
    sleep 5;  
  done

  echo "[Starting processing run: ${FILE}]"


#  screen -d -m -L -Logfile logs/log_analysis_${FILE}_FT0C_0.txt ./../scripts/analyse.sh ${FILE} hFT0C_BCs 0
 screen -d -m -L -Logfile ../logs/log_analysis_${FILE}_FT0C_2.txt ./../scripts/analyse.sh ${FILE} hFT0C_BCs 2
#  screen -d -m -L -Logfile ../logs/log_analysis_${FILE}hFT0MOuterA_Collisions_2.txt ./../scripts/analyse.sh ${FILE} hFT0MOuterA_Collisions 2
#  screen -d -m -L -Logfile logs/log_analysis_${FILE}_FT0M_2.txt ./../scripts/analyse.sh ${FILE} hFT0M_BCs 2
#  screen -d -m -L -Logfile logs/log_analysis_${FILE}_FV0A_2.txt ./../scripts/analyse.sh ${FILE} hFV0A_BCs 2
#  screen -d -m -L -Logfile logs/log_analysis_${FILE}_NMFTTracks_2.txt ./../scripts/analyse.sh ${FILE} hNMFTTracks 2
#  screen -d -m -L -Logfile logs/log_analysis_${FILE}_NGlobalTracks_2.txt ./../scripts/analyse.sh ${FILE} hNGlobalTracks 2
  sleep 0.1
  let COUNTER=COUNTER+1
done





