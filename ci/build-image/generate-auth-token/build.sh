#!/usr/bin/env bash

set -e -u -o pipefail

COMMIT_SHA=$(git rev-parse HEAD)

TAG="generate-auth-token"
docker build --tag "${TAG}" --file ci/build-image/generate-auth-token/Dockerfile .
echo "## imp-ci group-end Build image"

mkdir -p build
docker save -o ./build/generate-auth-token-image.tar "${TAG}"

echo "## imp-ci group-end Build generate-auth-token Image :runner:"

REMOTE_IMAGE_TAG="eu.gcr.io/windy-oxide-102215/${TAG}"
docker tag "${TAG}" "${REMOTE_IMAGE_TAG}"
docker push "${REMOTE_IMAGE_TAG}"