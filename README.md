# NanaoChanBot

A Telegram chat bot made with :heart: and :coffee:

## Features
* Webhooks
* Integrated web server
* Add custom API
* Built to run inside docker
* Light Docker image ~20MB (no trained model)
* Read to deploy on [Caprover](https://github.com/CapRover/CapRover)
* Machine learning based
* Json Input files easy to read and customize
* Easy config with env vars (list below)
* Reply on: quote, name found in message, tag

## Third parties
* [nlohmann json](https://github.com/nlohmann/json)
* [Mitie lib](https://github.com/mit-nlp/MITIE)

## Requirements
* Docker
* (Recommended) Nginx or Apache as reverse proxy

## Env Variables

| Variable | Type | Default Value | Description |
| :---: | :---: | :---: | :---: |
| BOT_PATH | string | /tgdev | Bot request path (eg `example.com/tgdev`) |
| SERVER_PORT | int | 8080 | Server listen port. Changing this will break builtin Dockerfiles! |
| BOT_NAME | string | NanaoChanBot | Bot username, must match Telegram bot username |
| BOT_NAME_REGEX | string | \b@?nanao(chanbot)?\b | Regex used to find bot name in message |
| BOT_MAX_MESSAGE_SIZE | int | 256 | Max message size, if message is longer appropriate event is triggered | 
| ML_MIN_SCORE | double | 0.5 | Minimum score to show ml result for message |
| SHOW_ML_SCORE | string | false | Add ml score at the end of the message |


## Compile
Run: ```sh docker-build.sh```

Note: this won't train your model!

To build and train the model run:
```sh docker-train.sh```

If you want to create a ready to deploy packet for your caprover node run:
```sh captain-build.sh```

This will build the container and create the .tar to upload!

(Note: remember to train your model before uploading the build!)

