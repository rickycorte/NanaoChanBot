
#ifndef NANAOCHANBOT_TELEGRAMBOTAPI_HPP
#define NANAOCHANBOT_TELEGRAMBOTAPI_HPP

#include <random>

#include <mitie/text_categorizer.h>

#include "http/ApiInterface.hpp"
#include "common/GlobalStaticConfig.hpp"
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

        std::random_device rd;

        double min_ml_score;
        bool show_ml_score;
    };


}


#endif //NANAOCHANBOT_TELEGRAMBOTAPI_HPP
