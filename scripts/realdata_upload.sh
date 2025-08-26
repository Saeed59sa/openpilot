#!/usr/bin/env bash

if [ $# -eq 0 ]; then
    echo "Usage: $0 <LOG_FOLDER1> [LOG_FOLDER2] [LOG_FOLDER3] ..."
    exit 1
fi

TODAY=$(date +%Y-%m-%d)
CAR=$(cat /data/params/d/CarName)
ID=$(cat /data/params/d/DongleId)
FTP_USER="openpilot"
FTP_PASSWORD="ruF3~Dt8"
FTP_HOST="jmtechn.com"
FTP_PORT="8022"

echo "$(date) - Starting route upload with ${#} segments"

if ! ping -c 3 8.8.8.8 > /dev/null 2>&1; then
  echo "$(date) - Network connection failed" >&2
  exit 1
fi

upload_file() {
  local filename="$1"
  local remote_filename="$2"
  local remote_path="$3"

  curl -v -T "$filename" -u "$FTP_USER:$FTP_PASSWORD" "ftp://${FTP_HOST}:${FTP_PORT}${remote_path}"
  if [ $? -ne 0 ]; then
      echo "$(date) - Failed to upload ${remote_filename}" >&2
      return 1
  fi
  return 0
}

TOTAL_SEGMENTS=$#
CURRENT_SEGMENT=0

for LOG_FOLDER in "$@"; do
  CURRENT_SEGMENT=$((CURRENT_SEGMENT + 1))
  LOG_FOLDER_NAME=$(basename "$LOG_FOLDER")

  echo "$(date) - Processing segment ${CURRENT_SEGMENT}/${TOTAL_SEGMENTS}: ${LOG_FOLDER_NAME}"

  if [ ! -d "$LOG_FOLDER" ]; then
    echo "$(date) - Warning: Directory $LOG_FOLDER does not exist, skipping..." >&2
    continue
  fi

  ftp -n << EOF
open $FTP_HOST $FTP_PORT
user $FTP_USER $FTP_PASSWORD
mkdir /tmux_log/${TODAY}_${CAR}_${ID}
mkdir /tmux_log/${TODAY}_${CAR}_${ID}/${LOG_FOLDER_NAME}
bye
EOF

    # qcamera.ts
    if [ -f "${LOG_FOLDER}/qcamera.ts" ]; then
      echo "$(date) - Uploading qcamera.ts from ${LOG_FOLDER_NAME}"
      remote_path="/tmux_log/${TODAY}_${CAR}_${ID}/${LOG_FOLDER_NAME}/qcamera.ts"
      if ! upload_file "${LOG_FOLDER}/qcamera.ts" "qcamera.ts" "$remote_path"; then
        echo "$(date) - Failed to upload qcamera.ts from ${LOG_FOLDER_NAME}" >&2
      fi
    fi

    # rlog
    for rlog_file in "${LOG_FOLDER}"/rlog.*; do
      if [ -f "$rlog_file" ]; then
        filename=$(basename "$rlog_file")
        echo "$(date) - Uploading ${filename} from ${LOG_FOLDER_NAME}"
        remote_path="/tmux_log/${TODAY}_${CAR}_${ID}/${LOG_FOLDER_NAME}/${filename}"
        if ! upload_file "$rlog_file" "$filename" "$remote_path"; then
          echo "$(date) - Failed to upload ${filename} from ${LOG_FOLDER_NAME}" >&2
        fi
      fi
    done

    # qlog
    for qlog_file in "${LOG_FOLDER}"/qlog.*; do
      if [ -f "$qlog_file" ]; then
        filename=$(basename "$qlog_file")
        echo "$(date) - Uploading ${filename} from ${LOG_FOLDER_NAME}"
        remote_path="/tmux_log/${TODAY}_${CAR}_${ID}/${LOG_FOLDER_NAME}/${filename}"
        if ! upload_file "$qlog_file" "$filename" "$remote_path"; then
          echo "$(date) - Failed to upload ${filename} from ${LOG_FOLDER_NAME}" >&2
        fi
      fi
    done

    echo "$(date) - Completed segment ${CURRENT_SEGMENT}/${TOTAL_SEGMENTS}: ${LOG_FOLDER_NAME}"
done

echo "$(date) - Route upload complete (${TOTAL_SEGMENTS} segments processed)"
exit 0
