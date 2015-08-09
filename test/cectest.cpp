//    Copyright (C) 2012 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include <iostream>
#include <cstdlib>

#include "renderer/ceccontrol.h"

using namespace doozy;

int main(int argc, char **argv)
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: " << argv[0] << " devicename command" << std::endl;
            return EXIT_FAILURE;
        }

        if (strcmp(argv[2], "on") == 0)
        {
            CecControl cec(argv[1]);
            cec.turnOn();
            if (!cec.isActiveSource())
            {
                cec.setActiveSource();
            }
        }
        else if (strcmp(argv[2], "off") == 0)
        {
            CecControl cec(argv[1]);
            cec.standBy();
        }
        else
        {
            std::cerr << "Unknown command (use 'on' or 'off')" << std::endl;
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}