
#ifndef NANAOCHANBOT_TELEGRAMBOTAPI_HPP
#define NANAOCHANBOT_TELEGRAMBOTAPI_HPP

#include <mitie/text_categorizer.h>

#include "http/ApiInterface.hpp"
#include "GlobalStaticConfig.hpp"
#include "common/ReplyContainer.hpp"

namespace RickyCorte::Telegram
{

    class BotApi : public Http::ApiInterface
    {
    public:
        BotApi();
        ~BotApi();

        Http::Reply onPOST(const Http::Request &req) override;

    private:
        mitie::text_categorizer categorizer;
        ReplyContainer* reply_container;
    };


}


#endif //NANAOCHANBOT_TELEGRAMBOTAPI_HPP
