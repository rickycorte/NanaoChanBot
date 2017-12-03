# NanaoChanBot

A Telegram chat bot

## Features
* Webhooks
* Trigger search in message
* Send images
* Search Anime data
* Bot commands
* Message filter
* Rate Limiter
* Thread pool to process requests
* Connection pool
* Simple usage stats
* Logs on file
* Internal timer

## Third parties
* OpenSSL: https://www.openssl.org/
* RapidJSON: https://github.com/Tencent/rapidjson
* Mongodb
* Mongodb C++ driver: http://mongodb.github.io/mongo-cxx-driver/

## Requirements
* Linux distros (tested on Ubuntu 16.04 lts)
* Reverse proxy for incoming connections (example: nginx)

## Compile
Install Mongodb c++ driver and then run

```make```
