#pragma once

#include <cpp/Integer.h>
#include <cpp/String.h>

namespace cpp
{

    struct Size
    {
        static constexpr uint64_t toKilos( size_t bytes )
            { return bytes / 1000LL; }
        static constexpr uint64_t toMegas( size_t bytes )
            { return bytes / ( 1000LL * 1000 ); }
        static constexpr uint64_t toGigas( size_t bytes )
            { return bytes / ( 1000LL * 1000 * 1000 ); }
        static constexpr uint64_t toTeras( size_t bytes )
            { return bytes / ( 1000LL * 1000 * 1000 * 1000 ); }

        static constexpr uint64_t ofKilos( size_t kilobytes )
            { return kilobytes * 1000LL; }
        static constexpr uint64_t ofKibis( size_t kibibytes )
            { return kibibytes * 1024LL; }
        static constexpr uint64_t ofMegas( size_t megabytes )
            { return megabytes * 1000LL * 1000; }
        static constexpr uint64_t ofMebis( size_t mebibytes )
            { return mebibytes * 1024LL * 1024; }
        static constexpr uint64_t ofGigas( size_t gigabytes )
            { return gigabytes * 1000LL * 1000 * 1000; }
        static constexpr uint64_t ofGibis( size_t gibibytes )
            { return gibibytes * 1024LL * 1024 * 1024; }
        static constexpr uint64_t ofTeras( size_t terabytes )
            { return terabytes * 1000LL * 1000 * 1000 * 1000; }

    };

}