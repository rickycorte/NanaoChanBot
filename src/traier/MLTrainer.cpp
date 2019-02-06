
#include "MLTrainer.hpp"

#include <fstream>
#include <sstream>

#include <mitie/text_categorizer_trainer.h>
#include <Logging.hpp>

#include "Utility.h"

void MLTrainer::Train()
{
    mitie::text_categorizer_trainer trainer{};
    trainer.set_num_threads(4);

    RC_INFO("Starting training...");

    try
    {
        auto json = parseInputFile();
        json = json["train_data"];

        //parsa i singoli blocchi per trovare le frasi e i tag su cui trainare il modello
        for (auto it = json.begin(); it != json.end(); ++it)
        {
            auto pharse_arr = (*it)["phrases"];
            for(auto iit = pharse_arr.begin(); iit != pharse_arr.end(); ++iit )
            {
                std::string phrase = *iit;
                std::string tag = (*it)["tag"];
                trainer.add(Utility::split_every(phrase), tag);

            }
        }


        mitie::text_categorizer categorizer = trainer.train();
        dlib::serialize(ML_MODEL_FILE) << "mitie::text_categorizer" << categorizer;


    } catch (...)
    {
        RC_CRITICAL("Something went wrong training the new model! Check the input file");
        return;
    }

    RC_INFO("Training complete!");
}



nlohmann::json MLTrainer::parseInputFile()
{
    nlohmann::json json{};

    std::ifstream is(ML_INPUT_FILE);
    if (is.is_open())
    {
        //leggi dal jon
        std::stringstream buffer;
        buffer << is.rdbuf();

        json = json.parse(buffer.str());

    }

    return json;
}
