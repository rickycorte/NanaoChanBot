#!/bin/sh
echo Training NanaoChanBot...

sh ./docker-build.sh

sudo docker build -t nanao:trainer . -f Dockerfile.train
sudo docker container create --name extract nanao:trainer
sudo docker container cp extract:/resources/model.dat ./build/resources
sudo docker container rm -f extract

sudo chown -R $USER:$USER ./build
cp -rf ./build/resources/model.dat ./resources/model.dat

echo DONE :3