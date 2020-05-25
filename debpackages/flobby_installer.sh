#!/usr/bin/env bash
create_debpkg="${1:-"False"}"
source .env

apt-get update
apt-get install -y apt-utils \
git \
cmake \
build-essential \
pkg-config \
libboost-system1.67.0 \
libboost1.67-dev \
libboost-filesystem-dev \
libboost-chrono1.67-dev \
libboost-regex-dev \
libboost-thread1.67-dev \
libjsoncpp-dev libjsoncpp1 \
libgraphicsmagick++1-dev \
libcurl4-gnutls-dev \
libminizip-dev \
libxpm-dev \
libxcb-screensaver0-dev \
libxss-dev \
checkinstall


# clone flobby repo and make it 
git clone https://github.com/cleanrock/flobby
cd flobby
CMAKE_INSTALL_PREFIX="/usr/local/bin/"
git submodule update --init
cmake .
make -j 12
make install

checkinstall --pakdir="/debpackages" --pkgname="flobby-${DISTRO}-${DISTRO_VERSION}-" -y
