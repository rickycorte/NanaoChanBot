/*
 * Hikari Backend
 *
 * Copyright (C) 2018 RickyCorte
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HIKARIBACKEND_HIKARICONFIG_H
#define HIKARIBACKEND_HIKARICONFIG_H

#define DEFAULT_SERVER_PORT 8080
#define EPOLL_MAX_EVENTS 64
#define MAX_CONNECTIONS 5
#define READ_BUFFER_SIZE 256

#define ML_INPUT_FILE "resources/dataset.json"
#define ML_MODEL_FILE "resources/model.dat"
#define ML_MIN_SCORE 0.5

#define TG_REPLY_FILE "resources/telegram.json"
#define TG_MAX_MESSAGE_SIZE 256
#define TG_RGX_BOT_NAME "\\b@?nanao(chanbot)?\\b"
#define TG_BOT_NAME "NanaoChanBot"
#define TG_INVOKE_TAG "ev_invoke"
#define TG_USRJOIN_TAG "ev_join"
#define TG_USRLEFT_TAG "ev_left"
#define TG_TOOLONG_TAG "ev_long"
#define TG_LOWSCORE_TAG "ev_lowscore"

/* 2kbytes per request is a really huge limit! */
#define MAX_HTTP_REQUEST_SIZE 16384

#endif //HIKARIBACKEND_HIKARICONFIG_H
