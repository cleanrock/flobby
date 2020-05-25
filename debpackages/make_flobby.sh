#!/usr/bin/env bash
source .env

#read -p "Did you checkup .env?  " -n 1 -r
#if [[ ! $REPLY =~ ^[Yy]$ ]]
#then
#  exit 1
#fi

printf "\ncreating dockerfile"
./make_dockerfile.sh
printf "\nfiring docker... for ${DISTRO} get some tea\n"

docker-compose up --build
