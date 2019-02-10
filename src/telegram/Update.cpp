
#include "Update.hpp"
#include <nlohmann/json.hpp>
#include <regex>

#include "common/GlobalStaticConfig.hpp"
#include "common/Utility.h"

RickyCorte::Telegram::Update::Update(const std::string &data):
        is_valid{true}, chat_id{0}, message_id{0}, is_reply{false},
        group_member_join{false} ,group_member_quit{false}, isInvoke{false}
{
    try
    {
        nlohmann::json json = nlohmann::json::parse(data);

        using namespace Utility;
        message = from_json_nothrow<std::string>(json, "message/text","");
        toLower(message);
        chat_id = json["message/chat/id"_json_pointer];
        message_type = json["message/chat/type"_json_pointer];
        message_id = json["message/message_id"_json_pointer];
        sender_name = json["message/from/first_name"_json_pointer];
        sender_username = from_json_nothrow<std::string>(json, "message/from/username", "");

        std::string reply_sender =  from_json_nothrow<std::string>(json,"message/reply_to_message/from/username", "");
        is_reply = reply_sender == TG_BOT_NAME;

        group_member_join = from_json_nothrow(json,"message/new_chat_member",false);
        group_member_quit = from_json_nothrow(json,"message/left_chat_member",false);

        isInvoke = std::regex_search(message,std::regex(TG_RGX_BOT_NAME));
    }
    catch(...)
    {
        is_valid = false;
    }
}

RickyCorte::Telegram::Update::~Update()
{

}

bool RickyCorte::Telegram::Update::isPrivateChat() const
{
    return message_type == "private";
}

bool RickyCorte::Telegram::Update::isGroupChat() const
{
    return message_type == "group" || message_type == "supergroup";
}

bool RickyCorte::Telegram::Update::isInvokeMessage() const
{
    return isInvoke;
}

bool RickyCorte::Telegram::Update::GroupHasNewMember() const
{
    return group_member_join;
}

bool RickyCorte::Telegram::Update::GroupHasMemberQuit() const
{
    return group_member_quit;
}

bool RickyCorte::Telegram::Update::isLongMessage() const
{
    return message.size() > TG_MAX_MESSAGE_SIZE;
}

bool RickyCorte::Telegram::Update::isReply() const
{
    return is_reply;
}

bool RickyCorte::Telegram::Update::isValid() const
{
    return is_valid;
}

std::string RickyCorte::Telegram::Update::getMessage() const
{
    return message;
}

std::string RickyCorte::Telegram::Update::getSender() const
{
    return (sender_name != "")? sender_name : sender_username;
}

std::string RickyCorte::Telegram::Update::getSenderUsername() const
{
    return sender_username;
}

std::string RickyCorte::Telegram::Update::makeTextReply(const std::string reply, bool as_reply) const
{
    nlohmann::json j;
    j["method"] = "sendMessage";
    j["chat_id"] = chat_id;
    j["text"] = message;
    if(is_reply)
        j["reply_to_message_id"] = message_id;

    return j.dump();
}
