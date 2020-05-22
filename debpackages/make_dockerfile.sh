#!/usr/bin/env bash
dockerfile="${1:-Dockerfile}"
source .env

echo """FROM $DISTRO:$DISTRO_VERSION
#COPY 19-sources.list /etc/apt/sources.list
WORKDIR /flobby
ADD flobby_installer.sh flobby_installer.sh
RUN chmod +x /flobby/flobby_installer.sh
#ENTRYPOINT ["ls", "flobby_installer.sh"]
#ENTRYPOINT [ls]
""" > Dockerfile
