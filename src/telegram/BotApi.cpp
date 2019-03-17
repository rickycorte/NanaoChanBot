#include "BotApi.hpp"

#include <Logging.hpp>

#include "common/Utility.h"
#include "Update.hpp"

RickyCorte::Telegram::BotApi::BotApi()
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

    min_ml_score = Utility::getEnvDouble("ML_MIN_SCORE", ML_MIN_SCORE);
    show_ml_score = Utility::getEnvStr("SHOW_ML_SCORE", "false") == "true";

}

RickyCorte::Http::Reply RickyCorte::Telegram::BotApi::onPOST(const RickyCorte::Http::Request &req)
{
    std::string result = "";
    bool is_reply = false;
    bool error = false;
    bool _vivid_reply = !(rd()% 5); // 1/5 of random actions


    Update *up = new Update(req.GetBody());
    if(up)
    {
        if(up->isValid())
        {
            std::string text_tag = "no_reply";
            double text_score = 1;

            if (up->isInvokeMessage())
            {
                text_tag = TG_INVOKE_TAG;
                is_reply = true;
            }
            else if (up->GroupHasNewMember())
            {
                text_tag = TG_USRJOIN_TAG;
            }
            else if (up->GroupHasMemberQuit())
            {
                text_tag = TG_USRLEFT_TAG;
            }
            else if (up->isPrivateChat() || up->isReply() || up->containsBotName() || _vivid_reply)
            {
                if (up->isLongMessage())
                {
                    text_tag = TG_TOOLONG_TAG;
                    is_reply = true;
                    if(_vivid_reply) text_score = 0; // evita di triggerare il testo lungo se non citata
                }
                else
                {
                    //ml
                    RC_DEBUG("ML: " + up->getMessage());

                    if (up->getMessage().size() < 1)
                        text_tag = TG_LOWSCORE_TAG;
                    else
                        categorizer.predict(Utility::split_every(up->getMessage()), text_tag, text_score);
                }
            }

            if(text_tag != "no_reply")
            {
                is_reply = !(rd() % 4);

                if (text_score < min_ml_score)
                    result = reply_container->GetReply(TG_LOWSCORE_TAG);
                else
                    result = reply_container->GetReply(text_tag);

                result = Utility::replace_string(result, "{u}", up->getSender());

                if(show_ml_score)
                    result += "\n\nTag: " + text_tag + " Score: " + std::to_string(text_score);


                result = up->makeTextReply(result, is_reply);
            }
            else result = "ok";

            //elimina le risposte senza senso nel caso di azione casuale
            if(text_score < min_ml_score && _vivid_reply)
                result = "ok";

        }
        else error = true;


        delete up;
    } else error = true;

    if(error)
        return Http::Reply(400, "{\"result\":\"error\", \"message\":\"Bad request\"}");

    return Http::Reply(200, result);

}

RickyCorte::Telegram::BotApi::~BotApi()
{
    if(reply_container != nullptr) delete reply_container;
}
