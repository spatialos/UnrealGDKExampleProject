#!/bin/bash
set -euo pipefail

BUILDKITE_TEMPLATE_FILE=ci/nightly.template.steps.yaml

if [[ -n "${MAC_BUILD:-}" ]]; then
    export BUILDKITE_COMMAND="./ci/setup-and-build.sh"
    REPLACE_STRING="s|BUILKDITE_AGENT_PLACEHOLDER|macos|g"
else
    export BUILDKITE_COMMAND="powershell -NoProfile -NonInteractive -InputFormat Text -Command ./ci/setup-and-build.ps1"
    REPLACE_STRING="s|BUILKDITE_AGENT_PLACEHOLDER|windows|g"
fi

# Download the unreal-engine.version file from the GDK repo so we can run the example project builds on the same versions the GDK was run against.
# This is not the pinnacle of engineering, as we rely on GitHub's web interface to download the file, but it seems like GitHub disallows git archive
# which would be our other option for downloading a single file.
# Also resolve the GDK branch to run against. The order of priority is:
# GDK_BRANCH envvar > same-name branch as the branch we are currently on > UnrealGDKVersion.txt > "master".
GDK_BRANCH_LOCAL="${GDK_BRANCH:-}"
if [ -z "${GDK_BRANCH_LOCAL}" ]; then
    GDK_REPO_HEADS=$(git ls-remote --heads "git@github.com:spatialos/UnrealGDK.git" "${BUILDKITE_BRANCH}")
    EXAMPLEPROJECT_REPO_HEAD="refs/heads/${BUILDKITE_BRANCH}"
    if echo "${GDK_REPO_HEADS}" | grep -qF "${EXAMPLEPROJECT_REPO_HEAD}"; then
        GDK_BRANCH_LOCAL="${BUILDKITE_BRANCH}"
    else
        GDK_VERSION=$(cat UnrealGDKVersion.txt)
        if [ -z "${GDK_VERSION}" ]; then
            GDK_BRANCH_LOCAL="master"
        else
            GDK_BRANCH_LOCAL="${GDK_VERSION}"
        fi
    fi
fi

NUMBER_OF_TRIES=0
while [ $NUMBER_OF_TRIES -lt 5 ]; do
    CURL_TIMEOUT=$((10<<NUMBER_OF_TRIES))
    NUMBER_OF_TRIES=$((NUMBER_OF_TRIES+1))
    echo "Trying to download unreal-engine.version from GitHub's UnrealGDK repository, try: $NUMBER_OF_TRIES, timeout: $CURL_TIMEOUT seconds ..."
    curl -L -m $CURL_TIMEOUT https://raw.githubusercontent.com/spatialos/UnrealGDK/$GDK_BRANCH_LOCAL/ci/unreal-engine.version -o ci/unreal-engine.version
    if [ $? -eq 0 ]; then
        break
    fi
done

# This script generates BuildKite steps for each engine version we want to test against.
# We retrieve these engine versions from the unreal-engine.version file in the UnrealGDK repository.
# The steps are based on the template in nightly.template.steps.yaml.

# Default to only testing the first version listed in the unreal-engine.version file
MAXIMUM_ENGINE_VERSION_COUNT_LOCAL="${MAXIMUM_ENGINE_VERSION_COUNT:-1}"
if [ -z "${ENGINE_VERSION}" ]; then 
    echo "Generating build steps for the first ${MAXIMUM_ENGINE_VERSION_COUNT_LOCAL} engine versions listed in unreal-engine.version"
    STEP_NUMBER=1
    IFS=$'\n'
    for COMMIT_HASH in $(cat < ci/unreal-engine.version); do
        if ((STEP_NUMBER > MAXIMUM_ENGINE_VERSION_COUNT_LOCAL)); then
            break
        fi

        export ENGINE_COMMIT_HASH="${COMMIT_HASH}"
        export STEP_NUMBER
        export GDK_BRANCH="${GDK_BRANCH_LOCAL}"
        sed $REPLACE_STRING "${BUILDKITE_TEMPLATE_FILE}" | buildkite-agent pipeline upload
        STEP_NUMBER=$((STEP_NUMBER+1))
    done
    # We generate one build step for each engine version, which is one line in the unreal-engine.version file.
    # The number of engine versions we are dealing with is therefore the counting variable from the above loop minus one.
    STEP_NUMBER=$((STEP_NUMBER-1))
    buildkite-agent meta-data set "engine-version-count" "${STEP_NUMBER}"
else
    echo "Generating steps for the specified engine version: ${ENGINE_VERSION}"
    export ENGINE_COMMIT_HASH="${ENGINE_VERSION}"
    export GDK_BRANCH="${GDK_BRANCH_LOCAL}"
    sed $REPLACE_STRING "${BUILDKITE_TEMPLATE_FILE}" | buildkite-agent pipeline upload
fi
