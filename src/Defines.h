/************************************************************************************
*
*    NanaoChanBot
*    Copyright (C) 2017  RickyCorte
*    https://github.com/rickycorte
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*
************************************************************************************/

#ifndef __DEFINES__
#define __DEFINES__

namespace Bot
{
  const char *const StartUpHeaderLine = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";;
  const char *const ProgramName = "NanaoChan Bot";
  const char *const ProgramVersion = "a30.0.0";


  const char *const TelegramName = "@NanaoChanBot";
  const char *const TelegramNameLower = "@nanaochanbot";
  const char *const TelegramToken = "bot token";

  const char *const HTTP_Get_Template = "GET {path} HTTP/1.1\nHost: {host}\nAccept-Language: en-us\nUser-agent: nanao\nConnection: keep-alive\n\n";
  const char *const Telegram_Host = "api.telegram.org";
  const char *const Telegram_Base_Path = "/bot token";

  const char *const webHookPort = "9799";
  const short maxConnections = 1;
  const long socketTimeOutSec = 2;
  const long socketTimeOutMilliSec = 0;

  const char *const sendMessageUrl = "sendMessage";
  const char *const sendPhotoUrl = "sendPhoto";
  const char *const sendStickerUrl = "sendSticker";
  const char *const sendAudioUrl = "sendAudio";

  const char *const AudioFile = "data/audios.ncb";
  const char *const StickerFile = "data/stickers.ncb";
  const char *const PhotosFile = "data/photos.ncb";

  const int rebellionRatePercent = 20;
  const int maxTagsPerMessage = 3;

  const int maxConcurrentRequestHandlerThreads = 10;
  const int maxMessageCharacters = 256;

  const short MaxRequestWords = 10;
  const short WorkerThreadUpdateDelay = 5;

  const short AvgRequestNumber = 100;

  const char *const QuotePicCategory = "quote";

  const int avoid_post_id = -1;

  const int connection_timeout_in_minutes = 2;

  const int rate_limit_time = 1;
  const int rate_limit_message_count = 3;

}

#endif
