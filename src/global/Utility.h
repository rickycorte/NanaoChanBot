
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

}

#endif //NANAOCHANBOT_UTILITY_H
