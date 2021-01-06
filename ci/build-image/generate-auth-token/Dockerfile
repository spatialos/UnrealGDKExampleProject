ARG BASE_REPO=eu.gcr.io/windy-oxide-102215
FROM ${BASE_REPO}/base_ci:20200924-1002-7225-feature-go-upgrade-1015

SHELL ["/bin/bash", "-e", "-u", "-o", "pipefail", "-c"]

# Set environment variables
# Keep these alpha-sorted
ENV CLOUD_SDK_VERSION="319.0.0" \
    PACKER_VERSION="1.6.6" \
    PIP_VERSION="20.2.4" \
    # because pyenv virtualenv-init fails without PROMPT_COMMAND being set
    PROMPT_COMMAND="" \
    PYENV_ROOT="/usr/local/pyenv" \
    PYENV_INSTALLER_VERSION="dd3f7d0914c5b4a416ca71ffabdf2954f2021596"

# Add the git ppa for an update version and install build dependency packages.
# Keep these alpha-sorted.
RUN add-apt-repository --yes --update ppa:git-core/ppa \
    && apt-get --quiet --quiet update \
    && apt-get --quiet --quiet install --yes --no-install-recommends \
        build-essential \
        bzip2 \
        ca-certificates \
        curl \
        git \
        jq \
        libbz2-dev \
        libffi-dev \
        liblzma-dev \
        libncurses5-dev \
        libncursesw5-dev \
        libreadline-dev \
        libsqlite3-dev \
        libssl-dev \
        libxmlsec1-dev \
        llvm \
        make \
        parallel \
        python-openssl \
        tk-dev \
        wget \
        xz-utils \
        zip \
        zlib1g-dev \
        && apt-get --quiet --quiet clean \
    && rm --recursive --force /var/lib/apt/lists/*

# Install 'go', it is required to find out tool versions.
RUN curl --location --show-error --silent "https://storage.googleapis.com/golang/go${GO_VERSION}.linux-amd64.tar.gz" | tar --extract --gzip --directory=/usr/local \
    && ln --symbolic /usr/local/go/bin/go /usr/local/bin/go

# Install the 'gcloud' toolchain.
RUN mkdir -p /usr/local/google \
    && curl --location --show-error --silent "https://dl.google.com/dl/cloudsdk/channels/rapid/downloads/google-cloud-sdk-${CLOUD_SDK_VERSION}-linux-x86_64.tar.gz" | tar --extract --gzip --directory=/usr/local/google \
    && ln --symbolic /usr/local/google/google-cloud-sdk/bin/gcloud /usr/bin/gcloud \
    && ln --symbolic /usr/local/google/google-cloud-sdk/bin/gsutil /usr/bin/gsutil \
    && gcloud components install beta

# Install the 'pyenv' toolchain for ansible.
RUN curl --location --show-error --silent "https://github.com/pyenv/pyenv-installer/raw/${PYENV_INSTALLER_VERSION}/bin/pyenv-installer" --output "/tmp/pyenv-installer" \
    && chmod +x "/tmp/pyenv-installer" \
    && "/tmp/pyenv-installer" \
    && export PATH="${PYENV_ROOT}/bin:${PATH}" \
    && "${PYENV_ROOT}/bin/pyenv" update \
    && "${PYENV_ROOT}/bin/pyenv" install "$(cat /.python-version | sed 's/ .*//')" \
    && "${PYENV_ROOT}/bin/pyenv" rehash \
    && hash -r \
    && "${PYENV_ROOT}/shims/pip3" install --upgrade "pip==${PIP_VERSION}" \
    && chmod --recursive 777 "${PYENV_ROOT}/shims" "${PYENV_ROOT}/plugins/pyenv-virtualenv/shims"

# We need to git tag and push, so need to ID ourselves.
RUN git config --global user.email "robot@improbable.io" \
    && git config --global user.name "Buildkite robot"

