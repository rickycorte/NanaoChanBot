
#ifndef NANAOCHANBOT_MLTRAINER_HPP
#define NANAOCHANBOT_MLTRAINER_HPP


#include <string>
#include <nlohmann/json.hpp>

#include "src/common/GlobalStaticConfig.hpp"

/**
 * Classe helper per trainare il modello di text categorizer
 */
class MLTrainer
{

public:
    /**
     * Traina il modello
     */
    void Train();

private:

    nlohmann::json parseInputFile();
};


#endif //NANAOCHANBOT_MLTRAINER_HPP
