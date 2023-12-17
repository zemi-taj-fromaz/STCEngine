#pragma once

#include <iostream>
#include <stdexcept>

#include "Application.h"

int main() {

    try {
        auto app =  CreateApplication();
        app->run();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

  //  std::cin.get();

    return EXIT_SUCCESS;
}