#!/bin/sh

echo Deploying NanaoChanBot on local machine ...
echo this does not train the model!

sh ./docker-build.sh

cd build
sudo docker build -t nanao:production .
sudo docker run -d -p 5000:8080 nanao:production

echo Running NanaoChanBot on port 5000

