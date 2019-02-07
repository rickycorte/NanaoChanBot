

#ifndef NANAOCHANBOT_TELEGRAMBOTAPI_HPP
#define NANAOCHANBOT_TELEGRAMBOTAPI_HPP

#include <mitie/text_categorizer.h>

#include "http/ApiInterface.hpp"
#include "GlobalStaticConfig.hpp"
#include "ReplyContainer.hpp"

namespace RickyCorte
{

    class TelegramBotApi : public Http::ApiInterface
    {
    public:
        TelegramBotApi();
        ~TelegramBotApi();

        Http::Reply onPOST(const Http::Request &req) override;
    };


    mitie::text_categorizer categorizer;
    ReplyContainer* reply_container;
}


#endif //NANAOCHANBOT_TELEGRAMBOTAPI_HPP
