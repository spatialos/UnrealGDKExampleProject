#!/bin/bash

NEW_USER=unrealworker
WORKER_SCRIPT=$1
WORKER_ID=$2
LOG_FILE=$3
shift 1

# Create log file in case it doesn't exist for filebeat to track
touch "${LOG_FILE}"

chmod -R go-w filebeat
chmod +x filebeat/filebeat

trap "exit" INT TERM
trap "kill 0" EXIT

if [ ! -d "filebeat/data" ]
then
    filebeat/filebeat -c filebeat/filebeat.yml setup --pipelines --dashboards --index-management
else
    echo "Data directory already exists. Skipping setup."
fi

filebeat/filebeat --strict.perms=false -c filebeat/filebeat.yml modules enable system

CONFIG_FILE="filebeat/filebeat$WORKER_ID.yml"
cp filebeat/filebeat.yml $CONFIG_FILE

filebeat/filebeat --strict.perms=false -c $CONFIG_FILE -E "filebeat.inputs=[{type:log,paths:['$LOG_FILE']}]" -E "path.data=filebeat/filebeat$WORKER_ID" &

chmod +x "$WORKER_SCRIPT"
"$WORKER_SCRIPT" "$@"

# Sleep to let filebeat finish uploading after the worker has stopped
sleep 30