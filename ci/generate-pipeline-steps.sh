#!/bin/bash
set -euo pipefail

# Download the unreal-engine.version file from the GDK repo so we can run the example project builds on the same versions the GDK was run against
GDK_BRANCH_LOCAL="${GDK_BRANCH:-master}"
curl https://raw.githubusercontent.com/spatialos/UnrealGDK/$GDK_BRANCH_LOCAL/ci/unreal-engine.version -o ci/unreal-engine.version

# This script generates BuildKite steps for each engine version we want to test against.
# We retrieve these engine versions from the unreal-engine.version file in the UnrealGDK repository.
# The steps are based on the template in nightly.template.steps.yaml.

if [ -z "${ENGINE_VERSION}" ]; then 
    echo "Generating build steps for each engine version listed in unreal-engine.version"
    # Only do slack notifies for the first engine version listed in the unreal-engine.version file
    STEP_NUMBER=1
    IFS=$'\n'
    for commit_hash in $(cat < ci/unreal-engine.version); do
        REPLACE_STRING="s|ENGINE_COMMIT_HASH_PLACEHOLDER|$commit_hash|g; s|STEP_NUMBER_PLACEHOLDER|$STEP_NUMBER|g"
        sed $REPLACE_STRING ci/nightly.template.steps.yaml | buildkite-agent pipeline upload
        STEP_NUMBER=$((STEP_NUMBER+1))
    done
    STEP_NUMBER=$((STEP_NUMBER-1))
    buildkite-agent meta-data set "engine-version-count" "$STEP_NUMBER"
else
    echo "Generating steps for the specified engine version: $ENGINE_VERSION" 
    sed "s|ENGINE_COMMIT_HASH_PLACEHOLDER|$ENGINE_VERSION|g" ci/nightly.template.steps.yaml | buildkite-agent pipeline upload
fi
