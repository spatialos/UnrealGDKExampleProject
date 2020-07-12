#!/usr/bin/env bash

set -e -u -o pipefail
if [[ -n "${DEBUG-}" ]]; then
    set -x
fi

source /opt/improbable/environment

run_uat() {
    ENGINE_DIRECTORY="${1}"
    EXAMPLEPROJECT_HOME="${2}"
    CLIENT_CONFIG="${3}"
    TARGET_PLATFORM="${4}"
    ARCHIVE_DIRECTORY="${5}"
    ADDITIONAL_UAT_FLAGS="${6:-}"
    COMMAND_LINE="${7:-}"
    echo "RunUAT.sh=${ENGINE_DIRECTORY}/Engine/Build/BatchFiles/RunUAT.sh"

    ${ENGINE_DIRECTORY}/Engine/Build/BatchFiles/RunUAT.sh \
        -ScriptsForProject="${EXAMPLEPROJECT_HOME}/Game/GDKShooter.uproject" \
        BuildCookRun \
        -nocompileeditor \
        -nop4 \
        -project="${EXAMPLEPROJECT_HOME}/Game/GDKShooter.uproject" \
        -cook \
        -stage \
        -archive \
        -archivedirectory="${ARCHIVE_DIRECTORY}" \
        -package \
        -clientconfig="${CLIENT_CONFIG}" \
        -ue4exe="${EXAMPLEPROJECT_HOME}/UnrealEngine/Engine/Binaries/Mac/UE4Editor-Cmd" \
        -pak \
        -prereqs \
        -nodebuginfo \
        -targetplatform="${TARGET_PLATFORM}" \
        -build \
        -utf8output \
        -compile \
        -cmdline="${COMMAND_LINE}" \
        "${ADDITIONAL_UAT_FLAGS}"
}


GDK_REPO="${1:-git@github.com:spatialos/UnrealGDK.git}"
GCS_PUBLISH_BUCKET="${2:-io-internal-infra-unreal-artifacts-production/UnrealEngine}"

pushd "$(dirname "$0")"
    EXAMPLEPROJECT_HOME="$(pwd)/.."
    GDK_BRANCH_NAME="${GDK_BRANCH:-master}"
    GDK_HOME="${EXAMPLEPROJECT_HOME}/Game/Plugins/UnrealGDK"
    ENGINE_COMMIT_HASH="${ENGINE_COMMIT_HASH:-0}"

    echo "--- clone-gdk-plugin"
    mkdir -p "${EXAMPLEPROJECT_HOME}/Game/Plugins"
    pushd "${EXAMPLEPROJECT_HOME}/Game/Plugins/"
        git clone ${GDK_REPO} \
            --branch ${GDK_BRANCH_NAME}  \
            --single-branch \
            --depth 1
    popd

    echo "--- print-head-gdk-commit"
    pushd "${GDK_HOME}"
        GDK_COMMIT_HASH=$(git rev-parse HEAD | cut -c1-6)
        echo "GDK at commit: ${GDK_COMMIT_HASH} on branch ${GDK_BRANCH_NAME}"
    popd

    echo "--- set-up-gdk-plugin"
    "${GDK_HOME}/Setup.sh" --mobile

    echo "--- set-up-engine"
    ENGINE_DIRECTORY="${EXAMPLEPROJECT_HOME}/UnrealEngine"
    #set meta-data engine-home-mac 
    buildkite-agent meta-data set "engine-home-mac" "$ENGINE_DIRECTORY"
    buildkite-agent meta-data set "exampleproject-home-mac" "$EXAMPLEPROJECT_HOME"
    
    echo "ENGINE_DIRECTORY:$ENGINE_DIRECTORY"

    "${GDK_HOME}/ci/get-engine.sh" \
        "${ENGINE_DIRECTORY}" \
        "${GCS_PUBLISH_BUCKET}"

    pushd "${ENGINE_DIRECTORY}"
        echo "--- create-xcode-project"
        Engine/Build/BatchFiles/Mac/Build.sh \
            -projectfiles \
            -project="${EXAMPLEPROJECT_HOME}/Game/GDKShooter.uproject" \
            -game \
            -engine \
            -progress

        echo "--- build-unreal-editor"
        Engine/Build/BatchFiles/Mac/XcodeBuild.sh \
            GDKShooterEditor \
            Mac \
            Development \
            "${EXAMPLEPROJECT_HOME}/Game/GDKShooter.uproject"

        echo "--- generate-schema"
        pushd "Engine/Binaries/Mac"
            UE4Editor.app/Contents/MacOS/UE4Editor \
                "${EXAMPLEPROJECT_HOME}/Game/GDKShooter.uproject" \
                -run=CookAndGenerateSchema \
                -targetplatform=MacNoEditor \
                -SkipShaderCompile \
                -unversioned \
                -map="/Maps/Control_Small"

            UE4Editor.app/Contents/MacOS/UE4Editor \
                "${EXAMPLEPROJECT_HOME}/Game/GDKShooter.uproject" \
                -run=GenerateSchemaAndSnapshots \
                -MapPaths="/Maps/Control_Small" \
                -SkipSchema
        popd
    popd

    echo "--- build-mac-client"
    run_uat \
        "${ENGINE_DIRECTORY}" \
        "${EXAMPLEPROJECT_HOME}" \
        "Development" \
        "Mac" \
        "${EXAMPLEPROJECT_HOME}/cooked-mac-${ENGINE_COMMIT_HASH}" \
        "-iterative"
        ""

    if [[ -n "${IOS_AUTOTEST:-}" ]]; then
        echo "--- change-runtime-settings"
        python "${EXAMPLEPROJECT_HOME}/ci/change-runtime-settings.py" "${EXAMPLEPROJECT_HOME}"

        echo "--- build-ios-client-for-autotest"
        run_uat \
            "${ENGINE_DIRECTORY}" \
            "${EXAMPLEPROJECT_HOME}" \
            "Development" \
            "IOS" \
            "${EXAMPLEPROJECT_HOME}/cooked-ios-${ENGINE_COMMIT_HASH}"
            "" \
            "connect.to.spatialos -workerType UnrealClient -OverrideSpatialNetworking +devauthToken ${AUTH_TOKEN} +deployment ${DEPLOYMENT_NAME} +linkProtocol Tcp" 
            
        echo "--- set-build-ios-job-id:$BUILDKITE_JOB_ID"
        buildkite-agent meta-data set "${ENGINE_COMMIT_HASH}-build-ios-job-id" "$BUILDKITE_JOB_ID" 
    else
        echo "--- build-ios-client"
        run_uat \
            "${ENGINE_DIRECTORY}" \
            "${EXAMPLEPROJECT_HOME}" \
            "Development" \
            "IOS" \
            "${EXAMPLEPROJECT_HOME}/cooked-ios-${ENGINE_COMMIT_HASH}"
            "" \
            ""  
    fi

popd
