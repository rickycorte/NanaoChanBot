#!/bin/sh

sh docker-build.sh

cp -rf ./captain-definition ./build

cd build

tar -cf nanao.tar resources Dockerfile captain-definition NanaoChanBot