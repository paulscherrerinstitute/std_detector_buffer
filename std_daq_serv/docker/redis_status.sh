#!/bin/bash
set -e

# How long the status record should be kept in the database.
EXPIRE_SECONDS=15
# At which interval should records be refreshed in the database.
STATUS_INTERVAL_SECONDS=10

if [[ -z "${REDIS_STATUS_KEY}" ]]; then
  echo "Environment variable REDIS_STATUS_KEY not defined."
  exit 1;
fi

STATUS="$(redis-cli -h "${REDIS_HOST}" -p "${REDIS_PORT}" -x hset "${REDIS_STATUS_KEY}" config < config.json)"
if [ "${STATUS}" != 0 ] && [ "${STATUS}" != 1 ]; then
  echo "Cound not set service status in Redis: ${STATUS}"
  exit 1;
fi

STATUS="$(redis-cli -h "${REDIS_HOST}" -p "${REDIS_PORT}" --raw hset "${REDIS_STATUS_KEY}" service_name "${SERVICE_NAME}")"
if [ "${STATUS}" != 0 ] && [ "${STATUS}" != 1 ]; then
  echo "Cound not set service status in Redis: ${STATUS}"
  exit 1;
fi

STATUS="$(redis-cli -h "${REDIS_HOST}" -p "${REDIS_PORT}" --raw hset "${REDIS_STATUS_KEY}" start_timestamp "$(date +%s)")"
if [ "${STATUS}" != 0 ] && [ "${STATUS}" != 1 ]; then
  echo "Cound not set service status in Redis: ${STATUS}"
  exit 1;
fi

STATUS="$(redis-cli -h "${REDIS_HOST}" -p "${REDIS_PORT}" expire "${REDIS_STATUS_KEY}" ${EXPIRE_SECONDS})"
if [ "${STATUS}" != 1 ]; then
  echo "Could not set status expire: ${STATUS}"
  exit 1;
fi

while true; do
  TIMESTAMP="$(date +%s%N)"

  STATUS="$(redis-cli -h "${REDIS_HOST}" -p "${REDIS_PORT}" expire "${REDIS_STATUS_KEY}" ${EXPIRE_SECONDS})"
  if [ "${STATUS}" != 1 ]; then
    echo "Could not set status expire: ${STATUS}"
    exit 1;
  fi

  # Update expire every 10 seconds.
  sleep "${STATUS_INTERVAL_SECONDS}"
done
