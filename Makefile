CC = g++
SRC_PATH = src
BUILD_PATH = build
CFLAGS = -std=c++11 -g -rdynamic
INCLUDE = -I/usr/include/mongocxx/v_noabi -I/usr/include/libmongoc-1.0 -I/usr/include/bsoncxx/v_noabi -I/usr/include/libbson-1.0
LIBS = -Ilib -pthread -static-libgcc -static-libstdc++ -L/usr/lib -lmongocxx -lbsoncxx -lrt -lssl -lcrypto
FILES = Defines.h Helper.h Helper.cpp Log.h Log.cpp Bot/Update.h Bot/Update.cpp Bot/Stats.h Bot/Stats.cpp DatabaseManager.h DatabaseManager.cpp Bot/TelegramBot.h Bot/TelegramBot.cpp Bot/AnimeResultParse.h Bot/AnimeResultParse.cpp Bot/Connection.h Bot/Connection.cpp Bot/RateLimiter.h Bot/RateLimiter.cpp DataMap.h DataMap.cpp ExecutionTimer.h ExecutionTimer.cpp Source.cpp
SOURCES = $(FILES:%=$(SRC_PATH)/%)
NAME = nanaoBot
LANGUAGE_PATH = language
DATA_PATH = Data
MALPARSER_PATH = MalParser

all:
	echo Compiling:
	mkdir -p $(BUILD_PATH)
	$(CC) $(CFLAGS) $(INCLUDE) $(SOURCES) $(LIBS) -o $(BUILD_PATH)/$(NAME)

	rm -rf $(BUILD_PATH)/$(LANGUAGE_PATH)
	cp -R $(LANGUAGE_PATH) $(BUILD_PATH)/$(LANGUAGE_PATH)

	rm -rf $(BUILD_PATH)/$(MALPARSER_PATH)
	cp -R $(MALPARSER_PATH) $(BUILD_PATH)/$(MALPARSER_PATH)

	rm -rf $(BUILD_PATH)/$(DATA_PATH)
	cp -R $(DATA_PATH) $(BUILD_PATH)/$(DATA_PATH)

	echo Build Complete.

run: all
	echo Running:
	./$(BUILD_PATH)/$(NAME)
