
#ifndef NANAOCHANBOT_UTILITY_H
#define NANAOCHANBOT_UTILITY_H

#include <vector>
#include <string>
#include <sstream>

namespace Utility
{


    /**
     * Divide una stringa al terminatore speficiato, di default lo spazio
     * @param input
     * @param separator
     * @return
     */
    inline std::vector<std::string> split_every(const std::string& input, char separator = ' ')
    {
        std::vector<std::string> res;
        std::istringstream ss(input);
        std::string temp;

        while(std::getline(ss, temp, ',')) {
            res.push_back(temp);
        }
        return  res;
    }


    /**
     * si spiega da se :3
     * @param strRef
     */
    inline void toLower(std::string &strRef)
    {
        for(int i = 0; i < strRef.size(); i++)
        {
            strRef[i] = std::tolower(strRef[i]);
            if(strRef[i] == '&') strRef[i] = ' ';
        }
    }


    /**
     * rinpiazza stringhe qua e la
     * @param target
     * @param itmToReplace
     * @param replacer
     * @return
     */
    inline std::string replace_string(std::string target, const std::string& itmToReplace, const std::string& replacer)
    {
        size_t pos = target.find(itmToReplace);
        // stringa non trovata, non c'e nulla da sostituire
        if(pos == std::string::npos)
        {
            return target;
        }

        target.replace(pos, itmToReplace.size(), replacer);
        return target;

    }


    inline const char * const bool_to_string(bool b)
    {
        return b ? "true" : "false";
    }

}

#endif //NANAOCHANBOT_UTILITY_H
