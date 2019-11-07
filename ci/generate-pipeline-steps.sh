#!/bin/bash
set -euo pipefail

# Download the unreal-engine.version file from the GDK repo so we can run the example project builds on the same versions the GDK was run against
GDK_BRANCH_LOCAL="${GDK_BRANCH:-master}"
curl https://raw.githubusercontent.com/spatialos/UnrealGDK/$GDK_BRANCH_LOCAL/ci/unreal-engine.version -o ci/unreal-engine.version

# This script generates steps for each engine version listed in unreal-engine.version in the GDK, and adds those to generated_base.steps.yaml
# The steps are based on the template in nightly.template.steps.yaml

if [ -z "${ENGINE_VERSION}" ]; then 
    echo "Generating build steps for each engine version listed in unreal-engine.version"
    # Only do slack notifies for the first engine version listed in the unreal-engine.version file
    FIRST_VERSION=true
    IFS=$'\n'
    for commit_hash in $(cat < ci/unreal-engine.version); do
        if [ "$FIRST_VERSION" = true ]; then
            FIRST_VERSION=false
            echo "      BUILDKITE_SLACK_NOTIFY: \"$BUILDKITE_SLACK_NOTIFY\"" >> ci/nightly.template.steps.yaml
        else
            echo "      BUILDKITE_SLACK_NOTIFY: \"false\"" >> ci/nightly.template.steps.yaml
        fi
        sed "s/ENGINE_COMMIT_HASH_PLACEHOLDER/$commit_hash/g" ci/nightly.template.steps.yaml | buildkite-agent pipeline upload
    done
else
    echo "Generating steps for the specified engine version: $ENGINE_VERSION" 
    sed "s/ENGINE_COMMIT_HASH_PLACEHOLDER/$ENGINE_VERSION/g" ci/nightly.template.steps.yaml | buildkite-agent pipeline upload
fi
