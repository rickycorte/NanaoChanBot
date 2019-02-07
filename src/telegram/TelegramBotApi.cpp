
#include "TelegramBotApi.hpp"

#include <Logging.hpp>

#include "Utility.h"

RickyCorte::TelegramBotApi::TelegramBotApi()
{

    reply_container = new ReplyContainer{TG_REPLY_FILE};
    if(!reply_container)
    {
        RC_CRITICAL("Unable to load replies");
        exit(1);
    }

    //carica modello
    std::string classname;
    dlib::deserialize(ML_MODEL_FILE) >> classname >> categorizer;

    //printa i tag del modello e se hanno risposte
    const std::vector<string> tagstr = categorizer.get_tag_name_strings();
    RC_INFO("The tagger supports ", tagstr.size(), " tags:");
    for (unsigned int i = 0; i < tagstr.size(); ++i)
    {
        std::cout << "   " << tagstr[i]
            << " found replies: "<< Utility::bool_to_string(reply_container->HasTag(tagstr[i])) << std::endl;
    }
}

RickyCorte::Http::Reply RickyCorte::TelegramBotApi::onPOST(const RickyCorte::Http::Request &req)
{
    return ApiInterface::onPOST(req);
}

RickyCorte::TelegramBotApi::~TelegramBotApi()
{
    if(reply_container != nullptr) delete reply_container;
}
