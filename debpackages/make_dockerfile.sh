#!/usr/bin/env bash
dockerfile="${1:-Dockerfile}"
source .env

echo """FROM $DISTRO:$DISTRO_VERSION
WORKDIR /flobby
ADD flobby_installer.sh flobby_installer.sh
ADD .env .env
RUN chmod +x /flobby/flobby_installer.sh
CMD "/bin/bash flobby_installer.sh"
""" > Dockerfile
