
#ifndef NANAOCHANBOT_UPDATE_HPP
#define NANAOCHANBOT_UPDATE_HPP

#include <string>

namespace RickyCorte::Telegram
{

    class Update
    {
    public:
        //Blocca la possibilita' di copiare gli update
        Update(const Update& up)  = delete;

        /**
         * Parsa il json ricevuto da telegram
         * @param data
         */
        Update(const std::string& data);

        ~Update();


        /********************************************
         *
         * Message info
         *
         ********************************************/

        /**
         * True se chat private
         * @return
         */
        bool isPrivateChat() const;

        /**
         * True se chat di gruppo
         * @return
         */
        bool isGroupChat() const;

        /**
         * Messaggio che contiene solo il nome del bot
         * @return
         */
        bool isInvokeMessage() const;

        /**
         * Messaggio che contiene il nome del bot
         * @return
         */
        bool containsBotName() const;

        /**
         * Resituisce se e' entrato una persona nel gruppo (bot compreso)
         * @return
         */
        bool GroupHasNewMember() const;

        /**
         * Restituisce se e' uscito qualcuno dal gruppo (bot compreso)
         * @return
         */
        bool GroupHasMemberQuit() const;

        /**
         * Restituisce se il messaggio supera il limite prestabilito di caratteri
         * @return
         */
        bool isLongMessage() const;

        /**
         * restituisce se questo messaggio e' una risposta a un messaggio del bot
         * @return
         */
        bool isReply() const;

        /**
         * restituisce se il messaggio e' valido e puo' essere processato
         * @return
         */
        bool isValid() const;


        /**
         * Restituisce il testo del messaggio
         * @return
         */
        std::string getMessage() const;


        /********************************************
         *
         * Sender info
         *
         ********************************************/

        /**
         * Restituisce il nome o l'userneme del mittente
         * @return
         */
        std::string getSender() const;


        /**
         * Restituisce l'username del mittente oppure "" in caso non sia impostato
         * @return
         */
        std::string getSenderUsername() const;


        /********************************************
         *
         * Reply
         *
         ********************************************/

        /**
         * Crea la stringa di risposta per il messaggio
         * @param reply messaggio da inviare
         * @param as_reply true se si vuole che il messaggio sia una risposta (citare)
         * @return
         */
        std::string makeTextReply(const std::string reply, bool as_reply = false) const;


    private:

        /*******************************************************************
        *
        * Dati
        *
        ********************************************************************/

        //username di chi ha inviato il messaggio (puo essere "")
        std::string sender_username;

        //nome di chi ha inviato il messaggio
        std::string sender_name;

        //id della chat
        std::int64_t chat_id;

        //tipo di chat da cui proviene il messaggio
        std::string message_type;

        //id del messaggio
        int message_id;

        //messaggio non modificato
        std::string message;

        bool group_member_join ,group_member_quit;

        //questo messaggio e' una risposta a un messaggio del bot
        bool is_reply;

        // messaggio valdo?
        bool is_valid;

        bool isInvoke, has_bot_name, is_long_message;

    };
}


#endif //NANAOCHANBOT_UPDATE_HPP
