#!/bin/sh
echo Training NanaoChanBot...

sh ./nanao-build.sh

docker build -t nanao:trainer . -f Dockerfile.train
docker container create --name extract nanao:trainer
docker container cp extract:/NanaoChanBot/resources/model.dat ./build/resources
docker container rm -f extract

chown -R ricky ./build

echo DONE :3