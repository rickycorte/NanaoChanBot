/*
 * Hikari Backend
 *
 * Copyright (C) 2018 RickyCorte
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <iostream>
#include <cstring>

#include <rickycorte/Logging.hpp>

#include "global/GlobalStaticConfig.hpp"
#include "Server.hpp"
#include "EchoApi.hpp"
#include "telegram/BotApi.hpp"

#include "traier/MLTrainer.hpp"


void printHelp()
{
    std::cout<<"NanaoChanBot usage:\n"
               "Run without parameters to start bot\n\t"
               "'-t' to train model\n\t"
               "'-h' to show this menu\n";
}

void trainDataset()
{
    MLTrainer trainer;
    trainer.Train();
}


void startBot()
{
    using namespace RickyCorte;


    Server server;
    server.AddApiInterface("/", new EchoApi());
    server.AddApiInterface("/tgdev", new Telegram::BotApi());
    server.Run();
}


int main(int argc, char *argv[])
{
    if(argc == 1) // solo nome programma
    {
        startBot();
        return  0;
    }
    else if(argc == 2) //cerca -h e -t
    {
        if(strcmp(argv[1], "-h") == 0)
        {
            printHelp();
            return 0;
        }
        if(strcmp(argv[1], "-t") == 0)
        {
            trainDataset();
            return  0;
        }
    }

    std::cout<<"Type NanaoChanBot '-h' to show help\n";

    return 0;
}