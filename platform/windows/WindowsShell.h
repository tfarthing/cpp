#pragma once

#include "../../../cpp/process/Platform.h"
#include "../../../cpp/file/Files.h"
#include "../../../cpp/file/FilePath.h"
#include "../../../cpp/data/DataArray.h"

#include <Shellapi.h>
#include <Shlobj.h>

namespace cpp
{
    namespace windows
    {
        namespace Shell
        {

            FilePath getPath( REFKNOWNFOLDERID folderId );

            void createShortcut(
                const FilePath & shortcutPath,
                const FilePath & targetPath,
                const Memory & args,
                const Memory & desc,
                const FilePath & iconPath,
                int iconID );

            void setUninstall(
                const Memory & appKey, 
                const Memory & name, 
                const Memory & cmd, 
                const Memory & location, 
                const Memory & publisher, 
                const Memory & version, 
                const Memory & icon,
                const Memory & productLink,
                const Memory & helpLink );
            void removeUninstall( const Memory & key );

            void setAppPath( FilePath appFile );
            void removeAppPath( FilePath appFile );

            void setOpenWith( FilePath appFile, String appLabel, const StringArray & exts );
            void removeOpenWith( FilePath appFile, String appLabel, const StringArray & exts );

        }
    }
}