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
        "${ADDITIONAL_UAT_FLAGS}"
}


GDK_REPO="${1:-git@github.com:spatialos/UnrealGDK.git}"
GCS_PUBLISH_BUCKET="${2:-io-internal-infra-unreal-artifacts-production/UnrealEngine}"

pushd "$(dirname "$0")"
    EXAMPLEPROJECT_HOME="$(pwd)/.."
    GDK_BRANCH_NAME="${GDK_BRANCH:-master}"
    GDK_HOME="${EXAMPLEPROJECT_HOME}/Game/Plugins/UnrealGDK"

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
                -map="/Maps/FPS-Start_Small"

            UE4Editor.app/Contents/MacOS/UE4Editor \
                "${EXAMPLEPROJECT_HOME}/Game/GDKShooter.uproject" \
                -run=GenerateSchemaAndSnapshots \
                -MapPaths="/Maps/FPS-Start_Small" \
                -SkipSchema
        popd
    popd

    echo "--- build-mac-client"
    run_uat \
        "${ENGINE_DIRECTORY}" \
        "${EXAMPLEPROJECT_HOME}" \
        "Development" \
        "Mac" \
        "${EXAMPLEPROJECT_HOME}/cooked-mac" \
        "-iterative"

    echo "--- build-ios-client"
    run_uat \
        "${ENGINE_DIRECTORY}" \
        "${EXAMPLEPROJECT_HOME}" \
        "Development" \
        "IOS" \
        "${EXAMPLEPROJECT_HOME}/cooked-ios"
popd
