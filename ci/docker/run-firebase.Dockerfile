ARG BASE_REPO=eu.gcr.io/windy-oxide-102215
FROM ${BASE_REPO}/base_ci:20200924-1002-7225-feature-go-upgrade-1015

SHELL ["/bin/bash", "-e", "-u", "-o", "pipefail", "-c"]

ENV CLOUD_SDK_VERSION="302.0.0" 

# Install cloud sdk for firebase tests
RUN mkdir --parents /usr/local/google \
    && curl --location --show-error --silent "https://dl.google.com/dl/cloudsdk/channels/rapid/downloads/google-cloud-sdk-${CLOUD_SDK_VERSION}-linux-x86_64.tar.gz" | tar --extract --gzip --directory=/usr/local/google \
    && ln --symbolic /usr/local/google/google-cloud-sdk/bin/gcloud /usr/bin/gcloud \
    && ln --symbolic /usr/local/google/google-cloud-sdk/bin/gsutil /usr/bin/gsutil \
    && gcloud components install beta \
    && gcloud config set project chlorodize-bipennated-8024348

COPY entrypoint.sh /usr/local/bin/entrypoint.sh
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
