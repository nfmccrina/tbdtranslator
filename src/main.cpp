#include "languid_config.h"
#include "languid_controller.h"
#include <iostream>

using namespace languid;

int main(int argc, char *argv[])
{
    try
    {
        LanguidConfig config = LanguidConfig::init(argc, argv);
        LanguidController controller(config);

        std::cout << "Languid has started. Press <enter> to exit...";

        controller.run();

        std::cout << "Languid has stopped." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "error handled: " << e.what() << '\n';
    }

    return 0;
}