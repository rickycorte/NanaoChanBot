#!/bin/sh
echo Building NanaoChanBot...

mkdir build

sudo docker build -t nanao:build . -f Dockerfile.build
sudo docker container create --name extract nanao:build
sudo docker container cp extract:/NanaoChanBot/NanaoChanBot ./build/
sudo docker container rm -f extract

echo copying required files..
sudo cp -rf ./resources ./build
sudo cp -rf ./Dockerfile ./build


sudo chown -R $USER:$USER ./build

echo DONE :3


