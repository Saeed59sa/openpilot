#!/usr/bin/env bash

LOG_FOLDER="$1"
TODAY=$(date +%y-%m-%d-%H:%M)
CAR=$(cat /data/params/d/CarName)
ID=$(cat /data/params/d/DongleId)
LOG_FILE="/tmp/upload.log"
FTP_USER="openpilot"
FTP_PASSWORD="ruF3~Dt8"
FTP_HOST="jmtechn.com"
FTP_PORT="8022"

exec &> "$LOG_FILE"

echo "$(date) - Starting upload: ${LOG_FOLDER}"

if ! ping -c 3 8.8.8.8 > /dev/null 2>&1; then
  echo "$(date) - Network connection failed" >&2
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
    echo "$(date) - Files not found:$MISSING_FILES" >&2
    exit 2
fi

ftp -n << EOF
open $FTP_HOST $FTP_PORT
user $FTP_USER $FTP_PASSWORD
mkdir /tmux_log/${TODAY}_${CAR}_${ID}_${LOG_FOLDER}
bye
EOF

ftp_result=$?

if [ $ftp_result -eq 1 ]; then
    echo "$(date) - FTP connection failed" >&2
    exit 3
elif [ $ftp_result -eq 550 ]; then
    echo "$(date) - FTP directory already exists" >&2
elif [ $ftp_result -ne 0 ]; then
    echo "$(date) - Failed to create directory on FTP (FTP error code: $ftp_result)" >&2
    exit 4
fi


upload_file() {
  local filename="$1"
  local remote_path="/tmux_log/${TODAY}_${CAR}_${ID}_${LOG_FOLDER}/$2"
  curl -v -T "$filename" -u "$FTP_USER:$FTP_PASSWORD" "ftp://${FTP_HOST}:${FTP_PORT}${remote_path}"
    if [ $? -ne 0 ]; then
        echo "$(date) - Failed to upload $2" >&2
        exit 5
    fi
}

upload_file "/data/0/media/realdata/${LOG_FOLDER}/qcamera.ts" "qcamera.ts"
upload_file "/data/0/media/realdata/${LOG_FOLDER}/rlog" "rlog"
upload_file "/data/0/media/realdata/${LOG_FOLDER}/qlog" "qlog"

echo "$(date) - Upload complete"
exit 0
