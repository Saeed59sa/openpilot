#!/usr/bin/env bash

LOG_FOLDER="$1"
LOG_FOLDER_NAME=$(basename "$LOG_FOLDER")

TODAY=$(date +%Y-%m-%d)
CAR=$(cat /data/params/d/CarName)
ID=$(cat /data/params/d/DongleId)
FTP_USER="openpilot"
FTP_PASSWORD="ruF3~Dt8"
FTP_HOST="jmtechn.com"
FTP_PORT="8022"

echo "$(date) - Starting upload: ${LOG_FOLDER}"

if ! ping -c 3 8.8.8.8 > /dev/null 2>&1; then
  echo "$(date) - Network connection failed" >&2
  exit 1
fi

ftp -n << EOF
open $FTP_HOST $FTP_PORT
user $FTP_USER $FTP_PASSWORD
mkdir /tmux_log/${TODAY}_${CAR}_${ID}
mkdir /tmux_log/${TODAY}_${CAR}_${ID}/${LOG_FOLDER_NAME}
bye
EOF

upload_file() {
  local filename="$1"
  local remote_filename="$2"
  local remote_path="/tmux_log/${TODAY}_${CAR}_${ID}/${LOG_FOLDER_NAME}/${remote_filename}"
  curl -v -T "$filename" -u "$FTP_USER:$FTP_PASSWORD" "ftp://${FTP_HOST}:${FTP_PORT}${remote_path}"
    if [ $? -ne 0 ]; then
        echo "$(date) - Failed to upload ${remote_filename}" >&2
        exit 2
    fi
}

upload_file "${LOG_FOLDER}/qcamera.ts" "qcamera.ts"

for rlog_file in "${LOG_FOLDER}"/rlog.*; do
    if [ -f "$rlog_file" ]; then
        filename=$(basename "$rlog_file")
        echo "$(date) - Uploading ${filename}"
        upload_file "$rlog_file" "$filename"
    fi
done

for qlog_file in "${LOG_FOLDER}"/qlog.*; do
    if [ -f "$qlog_file" ]; then
        filename=$(basename "$qlog_file")
        echo "$(date) - Uploading ${filename}"
        upload_file "$qlog_file" "$filename"
    fi
done

echo "$(date) - Upload complete"
exit 0
