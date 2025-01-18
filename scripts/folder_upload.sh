#!/usr/bin/env bash

LOG_FOLDER="$1"

TODAY=$(date +%y-%m-%d-%H:%M)
CAR=$(cat /data/params/d/CarName)
ID=$(cat /data/params/d/DongleId)
LOG_FILE="/tmp/upload.log"

exec &> "$LOG_FILE"

echo "$(date) - Starting upload: ${LOG_FOLDER}"

if [ -d "/data/${LOG_FOLDER}" ]; then
  if ping -c 3 8.8.8.8 > /dev/null 2>&1; then
    lftp -u "openpilot:ruF3~Dt8" -e "
      set ftp:list-options -a;
      mirror -R \
        --exclude '*.lock' \
        --exclude-glob '*size 0' \
        /data/${LOG_FOLDER} \
        /tmux_log/${TODAY}_${CAR}_${ID}_${LOG_FOLDER};
      quit
    " ftp://jmtechn.com:8022
    if [ $? -eq 0 ]; then
      echo "$(date) - Folder upload successful"
    else
      echo "$(date) - Folder upload failed (lftp error code: $?)"
      exit 1
    fi
  else
    echo "$(date) - No internet connection"
    exit 1
  fi
else
  echo "$(date) - Folder does not exist"
  exit 1
fi

echo "$(date) - Upload finished"
