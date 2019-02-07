

#ifndef NANAOCHANBOT_REPLYCONTAINER_HPP
#define NANAOCHANBOT_REPLYCONTAINER_HPP

#include <string>
#include <map>
#include <vector>
#include <random>


namespace RickyCorte
{
    /**
     * Classe che serve per recuperare le risposte da un file json
     */
    class ReplyContainer
    {
    public:
        ReplyContainer(const std::string& file_name);

        /**
        * Restituisce una riposta a caso del tag se presente, altrimenti restituisce una riposta standard
        * @param tag
        * @return
        */
        std::string GetReply(const std::string& tag);

        /**
         * Controlla se ha o meno risposte per il tag
         * @param tag
         * @return
         */
        bool HasTag(const std::string& tag);

    private:
        /**
         * Legge tutto un file
         * @param file_name
         * @return
         */
        std::string readFromFile(const std::string& file_name);

        /**
         * Parsa il file
         * @param data
         */
        void parseData(const std::string& data);


        std::map<std::string, std::vector<std::string>> reply_container;
        const std::string default_reply = "...";

        std::random_device rd;

    };
}


#endif //NANAOCHANBOT_REPLYCONTAINER_HPP
