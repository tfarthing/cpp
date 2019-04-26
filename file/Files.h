#pragma once

#include <filesystem>

#include "../../cpp/data/String.h"

namespace cpp
{

	namespace Files = std::filesystem;


    struct File
    {
        enum class                          Access               
                                                { Create, Write, Read };
        enum class                          Share                
                                                { AllowNone, AllowRead, AllowWrite, AllowAll };
    };

}