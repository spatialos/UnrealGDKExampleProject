#!/bin/bash
set -euo pipefail

BUILDKITE_TEMPLATE_FILE=ci/nightly.template.steps.yaml
SETUP_BUILD_COMMAND_BASH="./ci/setup-and-build.sh"
SETUP_BUILD_COMMAND_PS="powershell -NoProfile -NonInteractive -InputFormat Text -Command ./ci/setup-and-build.ps1"

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

buildkite-agent meta-data set "android-autotest" "0"
buildkite-agent meta-data set "ios-autotest" "0"

if [[ -n "${FIREBASE_AUTOTEST:-}" ]]; then
    buildkite-agent meta-data set "android-autotest" "1"
    ANDROID_AUTOTEST=true
    echo --- "ANDROID_AUTOTEST:${ANDROID_AUTOTEST}"

    if [[ -n "${MAC_BUILD:-}" ]]; then
        buildkite-agent meta-data set "ios-autotest" "1"
        IOS_AUTOTEST=true
        echo --- "IOS_AUTOTEST:${IOS_AUTOTEST}"
    fi
fi

insert_file_step() {
    filename="${1}"
    buildkite-agent pipeline upload ${filename}
}

insert_wait_step() {
    insert_file_step "ci/nightly.wait.yaml"
}

insert_setup_build_step(){
    version="${1}"
    agent="${2}"
    command="${3}"
    filename=ci/nightly.template.steps.yaml
    ENGINE_COMMIT_FORMATED_HASH=$(sed "s/ /_/g" <<< ${version} | sed "s/-/_/g" | sed "s/\./_/g")
    REPLACE_ENGINE_COMMIT_HASH="s|ENGINE_COMMIT_HASH_PLACEHOLDER|${version}|g"
    REPLACE_ENGINE_COMMIT_FORMATED_HASH="s|ENGINE_COMMIT_FORMATED_HASH_PLACEHOLDER|${ENGINE_COMMIT_FORMATED_HASH}|g"
    REPLACE_AGENT="s|AGENT_PLACEHOLDER|${agent}|g"
    REPLACE_COMMAND="s|COMMAND_PLACEHOLDER|${command}|g"
    sed ${REPLACE_ENGINE_COMMIT_HASH} "${filename}" | sed ${REPLACE_ENGINE_COMMIT_FORMATED_HASH} | sed ${REPLACE_AGENT} | sed ${REPLACE_COMMAND} | buildkite-agent pipeline upload
}

insert_auto_test_step(){
    version="${1}"
    device="${2}"
    filename=ci/nightly.${device}.autotest.yaml
    ENGINE_COMMIT_FORMATED_HASH=$(sed "s/ /_/g" <<< ${version} | sed "s/-/_/g" | sed "s/\./_/g")
    REPLACE_ENGINE_COMMIT_HASH="s|ENGINE_COMMIT_HASH_PLACEHOLDER|${version}|g"
    REPLACE_ENGINE_COMMIT_FORMATED_HASH="s|ENGINE_COMMIT_FORMATED_HASH_PLACEHOLDER|${ENGINE_COMMIT_FORMATED_HASH}|g"
    sed ${REPLACE_ENGINE_COMMIT_HASH} "${filename}" | sed ${REPLACE_ENGINE_COMMIT_FORMATED_HASH} | buildkite-agent pipeline upload
}

insert_auto_test_steps(){
    version="${1}"
    if [ ANDROID_AUTOTEST ]; then
        insert_auto_test_step ${version} android
    fi
    
    if [[ -n "${MAC_BUILD:-}" ]] && [ IOS_AUTOTEST ]; then
        insert_auto_test_step ${version} ios
    fi
}

insert_setup_build_steps(){
    version="${1}"
    if [[ -n "${FIREBASE_AUTOTEST:-}" ]]; then
        #if nightly build, we should build if MAC_BUILD setted
        if [[ -n "${MAC_BUILD:-}" ]]; then
            echo --- insert-setup-and-build-step-on-mac
            insert_setup_build_step ${version} macos ${SETUP_BUILD_COMMAND_BASH}
        fi
        
        #if nightly build, we should build on windows allways
        echo --- insert-setup-and-build-step-on-windows
        insert_setup_build_step ${version} windows ${SETUP_BUILD_COMMAND_PS}
    else
        # if normal build just build Mac or Windows
        if [[ -n "${MAC_BUILD:-}" ]]; then
            echo --- insert-setup-and-build-step-on-mac
            insert_setup_build_step ${version} macos ${SETUP_BUILD_COMMAND_BASH}
        else
            echo --- insert-setup-and-build-step-on-windows
            insert_setup_build_step ${version} windows ${SETUP_BUILD_COMMAND_PS}        
        fi
    fi
}

insert_generate_auth_token_step(){
    echo --- insert-wait-generate-auth-token-step
    insert_wait_step
    insert_file_step "ci/nightly.gen.auth.token.yaml"
}

# This script generates BuildKite steps for each engine version we want to test against.
# We retrieve these engine versions from the unreal-engine.version file in the UnrealGDK repository.
# The steps are based on the template in nightly.template.steps.yaml.

# Default to only testing the first version listed in the unreal-engine.version file
MAXIMUM_ENGINE_VERSION_COUNT_LOCAL="${MAXIMUM_ENGINE_VERSION_COUNT:-1}"
if [ -z "${ENGINE_VERSION}" ]; then 
    echo "Generating build steps for the first ${MAXIMUM_ENGINE_VERSION_COUNT_LOCAL} engine versions listed in unreal-engine.version"
    
    IFS=$'\n'
    VERSIONS=$(cat < ci/unreal-engine.version)

    #  turn on firebase auto test steps
    echo --- handle-firebase-steps
    if [[ -n "${FIREBASE_AUTOTEST:-}" ]]; then        
        echo --- add-auto-test-steps
        COUNT=1
        for VERSION in ${VERSIONS}; do
            echo --- handle-autotest-:${VERSION}-COUNT:${COUNT}
            if ((COUNT > MAXIMUM_ENGINE_VERSION_COUNT_LOCAL)); then
                break
            fi

            insert_auto_test_steps ${VERSION}
            COUNT=$((COUNT+1))
        done
    fi

    STEP_NUMBER=1
    for VERSION in ${VERSIONS}; do
        echo --- handle-setup-and-build-:${VERSION}-STEP_NUMBER:${STEP_NUMBER}
        if ((STEP_NUMBER > MAXIMUM_ENGINE_VERSION_COUNT_LOCAL)); then
            break
        fi

        export ENGINE_COMMIT_HASH="${VERSION}"
        echo "ENGINE_COMMIT_HASH:${ENGINE_COMMIT_HASH}"
        export STEP_NUMBER
        echo "STEP_NUMBER:${STEP_NUMBER}"
        export GDK_BRANCH="${GDK_BRANCH_LOCAL}"
        echo "GDK_BRANCH:${GDK_BRANCH}"
            
        insert_setup_build_steps ${VERSION}

        STEP_NUMBER=$((STEP_NUMBER+1))
    done
  
    # We generate one build step for each engine version, which is one line in the unreal-engine.version file.
    # The number of engine versions we are dealing with is therefore the counting variable from the above loop minus one.
    STEP_NUMBER=$((STEP_NUMBER-1))
    buildkite-agent meta-data set "engine-version-count" "${STEP_NUMBER}"

    # generate auth token for both android and ios autotest
    insert_generate_auth_token_step
else
    echo --- "Generating steps for the specified engine version: ${ENGINE_VERSION}"
    export ENGINE_COMMIT_HASH="${ENGINE_VERSION}"
    echo "ENGINE_COMMIT_HASH:${ENGINE_COMMIT_HASH}"
    export GDK_BRANCH="${GDK_BRANCH_LOCAL}"
    echo "GDK_BRANCH:${GDK_BRANCH}"
    
    #  turn on firebase auto test steps
    echo --- insert-auto-test-steps
    insert_auto_test_steps ${ENGINE_VERSION}

    echo --- insert-setup-and-build-steps
    insert_setup_build_steps ${ENGINE_VERSION}

    # generate auth token for both android and ios autotest
    insert_generate_auth_token_step
fi
