#!/bin/sh
echo Building NanaoChanBot...

mkdir build

docker build -t nanao:build . -f Dockerfile.build
docker container create --name extract nanao:build
docker container cp extract:/NanaoChanBot/NanaoChanBot ./build/
docker container rm -f extract

echo copying required files..
cp -rf ./resources ./build
cp -rf ./Dockerfile ./build


chown -R ricky ./build

echo DONE :3


