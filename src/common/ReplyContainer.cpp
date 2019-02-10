#include "ReplyContainer.hpp"

#include <fstream>
#include <sstream>

#include <nlohmann/json.hpp>
#include <Logging.hpp>

RickyCorte::ReplyContainer::ReplyContainer(const std::string &file_name)
{
    parseData(readFromFile(file_name));

    RC_INFO("True random (0 = false): ", rd.entropy());
}

std::string RickyCorte::ReplyContainer::GetReply(const std::string &tag)
{
    if(HasTag(tag))
    {
        std::vector<std::string>* arr = &(reply_container[tag]);

        return (*arr)[rd() % arr->size()];
    }
    else
    {
        return default_reply;
    }
}

bool RickyCorte::ReplyContainer::HasTag(const std::string &tag)
{
    return reply_container.find(tag) != reply_container.end();
}

std::string RickyCorte::ReplyContainer::readFromFile(const std::string &file_name)
{

    std::ifstream is(file_name);
    if (is.is_open())
    {
        //leggi dal json
        std::stringstream buffer;
        buffer << is.rdbuf();
        return buffer.str();
    }

    return "";
}

void RickyCorte::ReplyContainer::parseData(const std::string &data)
{


    try
    {
        nlohmann::json  json;
        json.parse(data);
        json = json["replies"];

        //parsa i singoli blocchi per trovare le frasi da usare come risposte
        for (auto it = json.begin(); it != json.end(); ++it)
        {
            auto pharse_arr = (*it)["phrases"];
            for(auto iit = pharse_arr.begin(); iit != pharse_arr.end(); ++iit )
            {
                std::string phrase = *iit;
                std::string tag = (*it)["tag"];
                reply_container[tag].push_back(phrase);
            }
        }

    } catch (...)
    {
        RC_CRITICAL("Something went wrong loading replies! Check the input file");
        return;
    }

}
