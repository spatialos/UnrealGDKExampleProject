FROM python:3

# Install: 
#   - cloudsmith-cli            for publishing the packages.
#   - gosu                      for handling issues with spatial writing files which buildkite-agent cannot delete.
RUN pip install cloudsmith-cli \
    && apt-get update \
    && curl -LSs -o /usr/local/bin/gosu -SL "https://github.com/tianon/gosu/releases/download/1.4/gosu-$(dpkg --print-architecture)" \
    && chmod +x /usr/local/bin/gosu \
    && apt-get clean

# Download spatial CLI into container
ARG TOOLBELT_VERSION="20190827.085359.7e083741fd"
WORKDIR /build/tools/
ADD "https://console.improbable.io/toolbelt/download/${TOOLBELT_VERSION}/linux" ./spatial
RUN ["chmod", "+x", "./spatial"]
ENV PATH "${PATH}:/build/tools/"

# Setup volumes to mount our secrets into.
VOLUME /var/spatial_oauth
VOLUME /var/cloudsmith_credentials
VOLUME /var/npmrc
ENV IMPROBABLE_CONFIG_DIR "/var/spatial_oauth/"
ENV CLOUDSMITH_CONFIG_DIR "/var/cloudsmith_credentials"

# Copy our entrypoint script into the container.
WORKDIR /usr/src/
COPY ./ci/docker/publish-packages-entrypoint.sh ./entrypoint.sh
RUN ["chmod", "+x", "./entrypoint.sh"]

ENTRYPOINT ["./entrypoint.sh"]