#!/bin/bash
set -euo pipefail

BUILDKITE_TEMPLATE_FILE=ci/nightly.template.steps.yaml

# Download the unreal-engine.version file from the GDK repo so we can run the example project builds on the same versions the GDK was run against.
# This is not the pinnacle of engineering, as we rely on GitHub's web interface to download the file, but it seems like GitHub disallows git archive
# which would be our other option for downloading a single file.
# Also resolve the GDK branch to run against. The order of priority is:
# GDK_BRANCH envvar > same-name branch as the branch we are currently on > UnrealGDKVersion.txt > "master".

echo --- init-gdk-branch

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
    echo ${GDK_BRANCH_LOCAL}
fi

echo --- number-of-tries
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
        echo --- handle-${COMMIT_HASH}
        if ((STEP_NUMBER > MAXIMUM_ENGINE_VERSION_COUNT_LOCAL)); then
            break
        fi

        export STEP_NUMBER
        echo "STEP_NUMBER:${STEP_NUMBER}"
        export GDK_BRANCH="${GDK_BRANCH_LOCAL}"
        echo "GDK_BRANCH:${GDK_BRANCH}"
       
        if [[ -n "${MAC_BUILD:-}" ]]; then
            REPLACE_STRING="s|BUILKDITE_AGENT_PLACEHOLDER|macos|g;s|ENGINE_COMMIT_HASH_PLACEHOLDER|${COMMIT_HASH}|g"
        else
            REPLACE_STRING="s|BUILKDITE_AGENT_PLACEHOLDER|windows|g;s|ENGINE_COMMIT_HASH_PLACEHOLDER|${COMMIT_HASH}|g"
        fi
        echo --- "REPLACE_STRING:${REPLACE_STRING}"
        sed $REPLACE_STRING "${BUILDKITE_TEMPLATE_FILE}" | buildkite-agent pipeline upload
        STEP_NUMBER=$((STEP_NUMBER+1))
    done
    # We generate one build step for each engine version, which is one line in the unreal-engine.version file.
    # The number of engine versions we are dealing with is therefore the counting variable from the above loop minus one.
    STEP_NUMBER=$((STEP_NUMBER-1))
    buildkite-agent meta-data set "engine-version-count" "${STEP_NUMBER}"

    # firebase auto test steps turn on
    echo --- handle-firebase-steps
    if [ -z ${NIGHTLY_BUILD} ]; then
        # add wait step
        echo --- add-wait-step
        buildkite-agent pipeline upload "ci/nightly.wait.yal"
        
        echo --- add-auto-test-steps
        BUILDKITE_AUTOTEST_TEMPLATE_FILE=ci/nightly.autotest.yaml
        for COMMIT_HASH in $(cat < ci/unreal-engine.version); do
            if [[ -n "${ANDROID_AUTOTEST:-}" ]]; then
                REPLACE_DEVICE_STRING="s|DEVICE_PLACEHOLDER|android|g;s|ENGINE_COMMIT_HASH_PLACEHOLDER|${COMMIT_HASH}|g"
                sed $REPLACE_DEVICE_STRING "${BUILDKITE_AUTOTEST_TEMPLATE_FILE}" | buildkite-agent pipeline upload
            fi
            
            if [[ -n "${MAC_BUILD:-}" ]] && [[ -n "${IOS_AUTOTEST:-}" ]]; then
                REPLACE_DEVICE_STRING="s|DEVICE_PLACEHOLDER|ios|g;s|HASH_PLACEHENGINE_COMMIT_HASH_PLACEHOLDEROLDER|${COMMIT_HASH}|g"
                sed $REPLACE_DEVICE_STRING "${BUILDKITE_AUTOTEST_TEMPLATE_FILE}" | buildkite-agent pipeline upload
            fi
        done
    fi
else
    echo --- "Generating steps for the specified engine version: ${ENGINE_VERSION}"
    export ENGINE_COMMIT_HASH="${ENGINE_VERSION}"
    echo "ENGINE_COMMIT_HASH:${ENGINE_COMMIT_HASH}"
    export GDK_BRANCH="${GDK_BRANCH_LOCAL}"
    echo "GDK_BRANCH:${GDK_BRANCH}"
    
    if [[ -n "${MAC_BUILD:-}" ]]; then
        REPLACE_STRING="s|BUILKDITE_AGENT_PLACEHOLDER|macos|g;s|ENGINE_COMMIT_HASH_PLACEHOLDER|${ENGINE_VERSION}|g"
    else
        REPLACE_STRING="s|BUILKDITE_AGENT_PLACEHOLDER|windows|g;s|ENGINE_COMMIT_HASH_PLACEHOLDER|${ENGINE_VERSION}|g"
    fi
    echo --- "REPLACE_STRING:${REPLACE_STRING}"
    sed $REPLACE_STRING "${BUILDKITE_TEMPLATE_FILE}" | buildkite-agent pipeline upload
    
    # firebase auto test steps turn on
    if [ -z ${NIGHTLY_BUILD} ]; then
        echo --- add-wait-step
        buildkite-agent pipeline upload "ci/nightly.wait.yal"

        echo --- add-auto-test
        BUILDKITE_AUTOTEST_TEMPLATE_FILE=ci/nightly.autotest.yaml
        
        if [[ -n "${ANDROID_AUTOTEST:-}" ]]; then
            REPLACE_DEVICE_STRING="s|DEVICE_PLACEHOLDER|android|g;s|ENGINE_COMMIT_HASH_PLACEHOLDER|${ENGINE_VERSION}|g"
            sed $REPLACE_DEVICE_STRING "${BUILDKITE_AUTOTEST_TEMPLATE_FILE}" | buildkite-agent pipeline upload
        fi
        
        if [[ -n "${MAC_BUILD:-}" ]] && [[ -n "${IOS_AUTOTEST:-}" ]]; then
            REPLACE_DEVICE_STRING="s|DEVICE_PLACEHOLDER|ios|g;s|HASH_PLACEHENGINE_COMMIT_HASH_PLACEHOLDEROLDER|${ENGINE_VERSION}|g"
            sed $REPLACE_DEVICE_STRING "${BUILDKITE_AUTOTEST_TEMPLATE_FILE}" | buildkite-agent pipeline upload
        fi
    fi
fi
