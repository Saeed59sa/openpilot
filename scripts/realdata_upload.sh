#!/usr/bin/env bash

LOG_FOLDER="$1"
TODAY=$(date +%y-%m-%d-%H:%M)
CAR=$(cat /data/params/d/CarName)
ID=$(cat /data/params/d/DongleId)
LOG_FILE="/tmp/upload.log"
FTP_CREDENTIALS="openpilot:ruF3~Dt8"
FTP_HOST="jmtechn.com:8022"

exec &> "$LOG_FILE"

echo "$(date) - Starting upload: ${LOG_FOLDER}"

if ! ping -c 3 8.8.8.8 > /dev/null 2>&1; then
  echo "$(date) - No internet connection"
  exit 1
fi

MISSING_FILES=""
if [ ! -f "/data/0/media/realdata/${LOG_FOLDER}/qcamera.ts" ]; then
    MISSING_FILES+=" qcamera.ts"
fi
if [ ! -f "/data/0/media/realdata/${LOG_FOLDER}/rlog" ]; then
    MISSING_FILES+=" rlog"
fi
if [ ! -f "/data/0/media/realdata/${LOG_FOLDER}/qlog" ]; then
    MISSING_FILES+=" qlog"
fi

if [ -n "$MISSING_FILES" ]; then
    echo "$(date) - Following files not found:$MISSING_FILES"
    exit 1
fi

curl -v -u "${FTP_CREDENTIALS}" "ftp://${FTP_HOST}/tmux_log/${TODAY}_${CAR}_${ID}_${LOG_FOLDER}" -X "MKD"
if [ $? -ne 0 ]; then
    echo "$(date) - Failed to create directory on FTP"
    exit 1
fi

upload_file() {
  local filename="$1"
  local remote_path="/tmux_log/${TODAY}_${CAR}_${ID}_${LOG_FOLDER}/$2"
  curl -v -T "$filename" -u "${FTP_CREDENTIALS}" "ftp://${FTP_HOST}${remote_path}"
    if [ $? -ne 0 ]; then
        echo "$(date) - Failed to upload $2"
        exit 1
    fi
}

upload_file "/data/0/media/realdata/${LOG_FOLDER}/qcamera.ts" "qcamera.ts"
upload_file "/data/0/media/realdata/${LOG_FOLDER}/rlog" "rlog"
upload_file "/data/0/media/realdata/${LOG_FOLDER}/qlog" "qlog"

echo "$(date) - Upload finished"
