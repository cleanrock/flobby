#!/usr/bin/env bash

read -p "Did you checkup .env?  " -n 1 -r
if [[ ! $REPLY =~ ^[Yy]$ ]]
then
  exit 1
fi

printf "\ncreating dockerfile"
./make_dockerfile.sh
printf "\nfiring docker... get some tea"
docker-compose up --build
