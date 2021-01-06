#!/usr/bin/env bash

set -e -u -o pipefail

COMMIT_SHA=$(git rev-parse HEAD)

if [[ -n "${BUILDKITE:-}" ]]; then
    declare -a BUILDKITE_ARGS=(
    "-e=BUILDKITE=${BUILDKITE}"
    "-e=BUILD_EVENT_CACHE_ROOT_PATH=/build-event-data"
    "-e=BUILDKITE_AGENT_ACCESS_TOKEN=${BUILDKITE_AGENT_ACCESS_TOKEN}"
    "-e=BUILDKITE_AGENT_ENDPOINT=${BUILDKITE_AGENT_ENDPOINT}"
    "-e=BUILDKITE_AGENT_META_DATA_CAPABLE_OF_BUILDING=${BUILDKITE_AGENT_META_DATA_CAPABLE_OF_BUILDING}"
    "-e=BUILDKITE_AGENT_META_DATA_ENVIRONMENT=${BUILDKITE_AGENT_META_DATA_ENVIRONMENT}"
    "-e=BUILDKITE_AGENT_META_DATA_PERMISSION_SET=${BUILDKITE_AGENT_META_DATA_PERMISSION_SET}"
    "-e=BUILDKITE_AGENT_META_DATA_PLATFORM=${BUILDKITE_AGENT_META_DATA_PLATFORM}"
    "-e=BUILDKITE_AGENT_META_DATA_SCALER_VERSION=${BUILDKITE_AGENT_META_DATA_SCALER_VERSION}"
    "-e=BUILDKITE_AGENT_META_DATA_AGENT_COUNT=${BUILDKITE_AGENT_META_DATA_AGENT_COUNT}"
    "-e=BUILDKITE_AGENT_META_DATA_MACHINE_TYPE=${BUILDKITE_AGENT_META_DATA_MACHINE_TYPE}"
    "-e=BUILDKITE_AGENT_META_DATA_QUEUE=${BUILDKITE_AGENT_META_DATA_QUEUE}"
    "-e=BUILDKITE_TIMEOUT=${BUILDKITE_TIMEOUT}"
    "-e=BUILDKITE_ARTIFACT_UPLOAD_DESTINATION=${BUILDKITE_ARTIFACT_UPLOAD_DESTINATION}"
    "-e=BUILDKITE_BRANCH=${BUILDKITE_BRANCH}"
    "-e=BUILDKITE_BUILD_CREATOR_EMAIL=${BUILDKITE_BUILD_CREATOR_EMAIL}"
    "-e=BUILDKITE_BUILD_CREATOR=${BUILDKITE_BUILD_CREATOR}"
    "-e=BUILDKITE_BUILD_ID=${BUILDKITE_BUILD_ID}"
    "-e=BUILDKITE_BUILD_URL=${BUILDKITE_BUILD_URL}"
    "-e=BUILDKITE_COMMIT=${BUILDKITE_COMMIT}"
    "-e=BUILDKITE_JOB_ID=${BUILDKITE_JOB_ID}"
    "-e=BUILDKITE_LABEL=${BUILDKITE_LABEL}"
    "-e=BUILDKITE_MESSAGE=${BUILDKITE_MESSAGE}"
    "-e=BUILDKITE_ORGANIZATION_SLUG=${BUILDKITE_ORGANIZATION_SLUG}"
    "-e=BUILDKITE_LAST_STEP_BUILDID=${BUILDKITE_LAST_STEP_BUILDID}"
    "-e=PYTHONHASHSEED=0"
    "-e=BUILDKITE_PIPELINE_SLUG=${BUILDKITE_PIPELINE_SLUG}"
    "--volume=/usr/bin/buildkite-agent:/usr/bin/buildkite-agent"
    "--volume=/usr/local/bin/imp-tool:/usr/local/bin/imp-tool"
    "--volume=${BUILDKITE_BUILD_CHECKOUT_PATH}/app/results:/app/results"
    )
fi

if [[ -n "${SPATIAL_SECRET:-}" ]]; then
    BUILDKITE_ARGS+=("-e=SPATIAL_SECRET=${SPATIAL_SECRET}")
fi

if [[ -n "${SPATIAL_SECRET_STAGING:-}" ]]; then
    BUILDKITE_ARGS+=("-e=SPATIAL_SECRET_STAGING=${SPATIAL_SECRET_STAGING}")
fi

mkdir -p logs
mkdir -p app/results
USER_ID=$(id -u)

SERVICES_REGION="WESTERN"
# Grab spatial service account secret in order to use package_client, make sure it is removed after script execution
SPATIALOS_CREDENTIALS_PATH="$HOME/.improbable/oauth2/oauth2_refresh_token"

imp-ci secrets read --environment=production --buildkite-org=improbable \
    --secret-type=spatialos-service-account --secret-name=prod-res-benchmarks-production \
    --write-to="${SPATIALOS_CREDENTIALS_PATH}" \
    --field=token

IMAGE="eu.gcr.io/windy-oxide-102215/generate-auth-token"
docker run "${BUILDKITE_ARGS[@]}" -e "LOCAL_USER_ID=${USER_ID}" "${IMAGE}"
