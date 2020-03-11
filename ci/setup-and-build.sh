#!/usr/bin/env bash

GDK_REPO="${1:-git@github.com:spatialos/UnrealGDK.git}"
GCS_PUBLISH_BUCKET="${2:-io-internal-infra-unreal-artifacts-production/UnrealEngine}"

EXAMPLEPROJECT_HOME="$(pwd)"
GDK_BRANCH_NAME=${GDK_BRANCH:-master}
GDK_HOME="${EXAMPLEPROJECT_HOME}/Game/Plugins/UnrealGDK"

echo "--- clone-gdk-plugin"
mkdir -p "${EXAMPLEPROJECT_HOME}/Game/Plugins"
pushd "${EXAMPLEPROJECT_HOME}/Game/Plugins/"
	git clone ${GDK_REPO} --depth 1 -b ${GDK_BRANCH_NAME}
popd

echo "--- get-head-gdk-commit"
pushd ${GDK_HOME}
	# Get the short commit hash of this gdk build for later use in assembly name
	GDK_COMMIT_HASH=$(git rev-parse HEAD | cut -c1-6)
    echo "GDK at commit: ${GDK_COMMIT_HASH} on branch ${GDK_BRANCH_NAME}"
popd

echo "--- set-up-gdk-plugin"
${GDK_HOME}/Setup.sh --mobile
# TODO something is missing, the gdk plugin hasn't been properly set up. cooking doesn't succeed completelys
# Use the cached engine version or set it up if it has not been cached yet.
echo "--- set-up-engine"
ENGINE_DIRECTORY="${EXAMPLEPROJECT_HOME}/UnrealEngine"
pushd ${GDK_HOME}
	ci/get-engine.sh "${ENGINE_DIRECTORY}"
popd

pushd ${ENGINE_DIRECTORY}
	echo "--- create-xcode-project"
	${ENGINE_DIRECTORY}/Engine/Build/BatchFiles/Mac/Build.sh -projectfiles -project="${EXAMPLEPROJECT_HOME}/Game/GDKShooter.uproject" -game -engine -progress

	echo "--- build-unreal-editor"
	${ENGINE_DIRECTORY}/Engine/Build/BatchFiles/Mac/XcodeBuild.sh GDKShooterEditor Mac Development "${EXAMPLEPROJECT_HOME}/Game/GDKShooter.uproject"

	if [[ $? -ne 0 ]]; then
	  echo "Failed to build the Unreal Editor."
	  exit 1
	fi

	echo "--- generate-schema"
	pushd "Engine/Binaries/Mac"
		UE4Editor.app/Contents/MacOS/UE4Editor "${EXAMPLEPROJECT_HOME}/Game/GDKShooter.uproject" -run=GenerateSchemaAndSnapshots -MapPaths="/Maps/FPS-Start_Small"
		if [[ $? -ne 0 ]]; then
		  echo "Failed to generate schema."
		  exit 1
		fi
	popd
popd

echo "--- Build MacOS Client"
${ENGINE_DIRECTORY}/Engine/Build/BatchFiles/RunUAT.sh \
	-ScriptsForProject=${EXAMPLEPROJECT_HOME}/Game/GDKShooter.uproject \
	BuildCookRun \
	-nocompileeditor \
	-nop4 \
	-project=${EXAMPLEPROJECT_HOME}/Game/GDKShooter.uproject \
	-cook \
	-stage \
	-archive \
	-archivedirectory=${EXAMPLEPROJECT_HOME}/cooked-mac \
	-package \
	-clientconfig=Development \
	-ue4exe=${EXAMPLEPROJECT_HOME}/UnrealEngine/Engine/Binaries/Mac/UE4Editor-Cmd \
    -pak \
    -prereqs \
    -nodebuginfo \
    -targetplatform=Mac \
    -build \
    -utf8output \
    -compile

if [[ $? -ne 0 ]]; then
	echo "Failed to build the MacOS client."
	exit 1
fi
echo "--- Build iOS Client"
${ENGINE_DIRECTORY}/Engine/Build/BatchFiles/RunUAT.sh \
	-ScriptsForProject=${EXAMPLEPROJECT_HOME}/Game/GDKShooter.uproject \
	BuildCookRun \
	-nocompileeditor \
	-nop4 \
	-project=${EXAMPLEPROJECT_HOME}/Game/GDKShooter.uproject \
	-cook \
	-stage \
	-archive \
	-archivedirectory=${EXAMPLEPROJECT_HOME}/cooked-ios \
	-package \
	-clientconfig=Development \
	-ue4exe=${EXAMPLEPROJECT_HOME}/UnrealEngine/Engine/Binaries/Mac/UE4Editor-Cmd \
    -pak \
    -prereqs \
    -nodebuginfo \
    -targetplatform=IOS \
    -build \
    -utf8output \
    -compile 

if [[ $? -ne 0 ]]; then
  echo "Failed to build the iOS client."
  exit 1
fi

