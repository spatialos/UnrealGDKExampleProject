#!/bin/bash

set -e -u -o pipefail
if [[ -n "${DEBUG-}" ]]; then
    set -x
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

insert_setup_build_step(){
    VERSION="${1}"
    AGENT="${2}"
    COMMAND="${3}"
    FILENAME="ci/nightly.template.steps.yaml"
    # ENGINE_COMMIT_FORMATTED_HASH is the same as ENGINE_COMMIT_HASH, but replace ' ','.','-' with '_' to use it as a buildkite key.
    # So as to make the steps indentify for different engine_version
    # For more information: https://buildkite.com/docs/pipelines/block-step#text-input-attributes
    ENGINE_COMMIT_FORMATTED_HASH=$(sed "s/ /_/g" <<< ${VERSION} | sed "s/-/_/g" | sed "s/\./_/g")
    REPLACE_ENGINE_COMMIT_HASH="s|ENGINE_COMMIT_HASH_PLACEHOLDER|${VERSION}|g"
    REPLACE_ENGINE_COMMIT_FORMATTED_HASH="s|ENGINE_COMMIT_FORMATTED_HASH_PLACEHOLDER|${ENGINE_COMMIT_FORMATTED_HASH}|g"
    REPLACE_AGENT="s|AGENT_PLACEHOLDER|${AGENT}|g"
    REPLACE_COMMAND="s|COMMAND_PLACEHOLDER|${COMMAND}|g"
    sed "${REPLACE_ENGINE_COMMIT_HASH}" "${FILENAME}" | sed "${REPLACE_ENGINE_COMMIT_FORMATTED_HASH}" | sed "${REPLACE_AGENT}" | sed "${REPLACE_COMMAND}" | buildkite-agent pipeline upload
}

insert_firebase_test_step(){
    VERSION="${1}"
    DEVICE="${2}"
    FILENAME="ci/nightly.${DEVICE}.firebase.test.yaml"
    ENGINE_COMMIT_FORMATTED_HASH=$(sed "s/ /_/g" <<< ${VERSION} | sed "s/-/_/g" | sed "s/\./_/g")
    REPLACE_ENGINE_COMMIT_HASH="s|ENGINE_COMMIT_HASH_PLACEHOLDER|${VERSION}|g"
    REPLACE_ENGINE_COMMIT_FORMATTED_HASH="s|ENGINE_COMMIT_FORMATTED_HASH_PLACEHOLDER|${ENGINE_COMMIT_FORMATTED_HASH}|g"
    sed "${REPLACE_ENGINE_COMMIT_HASH}" "${FILENAME}" | sed "${REPLACE_ENGINE_COMMIT_FORMATTED_HASH}" | buildkite-agent pipeline upload
}

insert_firebase_test_steps(){
    VERSION="${1}"
    if [[ -n "${FIREBASE_TEST:-}" ]]; then
        insert_firebase_test_step "${VERSION}" android
        
        if [[ -n "${MAC_BUILD:-}" ]]; then
            insert_firebase_test_step "${VERSION}" ios
        fi
    fi    
}

insert_setup_build_steps(){
    VERSION="${1}"
    # CI steps run on MacOS agent
    SETUP_BUILD_COMMAND_BASH="./ci/setup-and-build.sh"

    # CI steps run on Windows agent
    SETUP_BUILD_COMMAND_PS="powershell -NoProfile -NonInteractive -InputFormat Text -Command ./ci/setup-and-build.ps1"

    if [[ -n "${FIREBASE_TEST:-}" ]]; then
        if [[ -n "${MAC_BUILD:-}" ]]; then
            echo "--- insert-setup-and-build-step-on-mac"
            insert_setup_build_step "${VERSION}" macos "${SETUP_BUILD_COMMAND_BASH}"
        fi
        
        echo "--- insert-setup-and-build-step-on-windows"
        insert_setup_build_step "${VERSION}" windows "${SETUP_BUILD_COMMAND_PS}"
    else
        if [[ -n "${MAC_BUILD:-}" ]]; then
            echo "--- insert-setup-and-build-step-on-mac"
            insert_setup_build_step "${VERSION}" macos "${SETUP_BUILD_COMMAND_BASH}"
        else
            echo "--- insert-setup-and-build-step-on-windows"
            insert_setup_build_step "${VERSION}" windows "${SETUP_BUILD_COMMAND_PS}"        
        fi
    fi
}

# This script generates BuildKite steps for each engine version we want to test against.
# We retrieve these engine versions from the unreal-engine.version file in the UnrealGDK repository.
# The steps are based on the template in nightly.template.steps.yaml.

# Default to only testing the first version listed in the unreal-engine.version file
MAXIMUM_ENGINE_VERSION_COUNT_LOCAL="${MAXIMUM_ENGINE_VERSION_COUNT:-1}"
if [ -z "${ENGINE_VERSION}" ]; then 
    echo "Generating build steps for the first ${MAXIMUM_ENGINE_VERSION_COUNT_LOCAL} engine versions listed in unreal-engine.version"
    
    IFS=$'\n'
    VERSIONS=$(cat < ci/unreal-engine.version | tr -d '\r')

    # Turn on Firebase test steps
    echo "--- handle-firebase-steps"
    if [[ -n "${FIREBASE_TEST:-}" ]]; then        
        echo "--- insert-firebase-test-steps"
        COUNT=1
        for VERSION in ${VERSIONS}; do
            TRIMED_VERSION=$(sed 's/ *$//g' <<< ${VERSION})
            echo --- handle-firebase-:${TRIMED_VERSION}-COUNT:${COUNT}
            if ((COUNT > MAXIMUM_ENGINE_VERSION_COUNT_LOCAL)); then
                break
            fi

            insert_firebase_test_steps "${TRIMED_VERSION}"
            COUNT=$((COUNT+1))
        done
    fi

    STEP_NUMBER=1
    for VERSION in ${VERSIONS}; do
        TRIMED_VERSION=$(sed 's/ *$//g' <<< ${VERSION})
        echo "--- handle-setup-and-build-:${TRIMED_VERSION}-STEP_NUMBER:${STEP_NUMBER}"
        if ((STEP_NUMBER > MAXIMUM_ENGINE_VERSION_COUNT_LOCAL)); then
            break
        fi

        export ENGINE_COMMIT_HASH="${TRIMED_VERSION}"
        export STEP_NUMBER
        export GDK_BRANCH="${GDK_BRANCH_LOCAL}"
            
        insert_setup_build_steps "${TRIMED_VERSION}"

        STEP_NUMBER=$((STEP_NUMBER+1))
    done
  
    # We generate one build step for each engine version, which is one line in the unreal-engine.version file.
    # The number of engine versions we are dealing with is therefore the counting variable from the above loop minus one.
    STEP_NUMBER=$((STEP_NUMBER-1))
    buildkite-agent meta-data set "engine-version-count" "${STEP_NUMBER}"

else
    echo "--- Generating steps for the specified engine version: ${ENGINE_VERSION}"
    TRIMED_VERSION=$(sed 's/ *$//g' <<< ${ENGINE_VERSION})
    export ENGINE_COMMIT_HASH="${TRIMED_VERSION}"
    export GDK_BRANCH="${GDK_BRANCH_LOCAL}"    
    # If the specified version is set, the STEP_NUMBER should be 1
    export STEP_NUMBER=1
    
    # Turn on Firebase test steps
    echo "--- insert-firebase-test-steps"
    insert_firebase_test_steps "${TRIMED_VERSION}"

    echo "--- insert-setup-and-build-steps"
    insert_setup_build_steps "${TRIMED_VERSION}"

    # If the specified version is set, the engine-version-count should be 1
    buildkite-agent meta-data set "engine-version-count" "1"
fi

# Generate auth token for both android and ios firebase test
export GDK_BRANCH="${GDK_BRANCH_LOCAL}"
buildkite-agent pipeline upload "ci/nightly.gen.auth.token.yaml"
