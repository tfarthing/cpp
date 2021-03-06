#include "../../../cpp/platform/windows/WindowsException.h"
#include "../../../cpp/platform/windows/WindowsRegistry.h"
#include "../../../cpp/platform/windows/WindowsShell.h"
#include "../../../cpp/text/Utf16.h"
#include "../../../cpp/data/DataArray.h"

using std::wstring;

namespace cpp
{
    namespace windows
    {
        namespace Shell
        {

            FilePath getPath( REFKNOWNFOLDERID folderId )
            {
                PWSTR path;
                check( SHGetKnownFolderPath( folderId, 0, 0, &path ) );
				FilePath result{ std::filesystem::path{ path }.generic_string( ) };
                CoTaskMemFree( path );

                return result;
            }

            void createShortcut(
                const FilePath & shortcutPath,
                const FilePath & targetPath,
                const Memory & args,
                const Memory & desc,
                const FilePath & iconPath,
                int iconID )
            {
                // Get a pointer to the IShellLink interface.
                IShellLink* psl = NULL;
                IPersistFile* ppf = NULL;
                HRESULT hres = CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, reinterpret_cast<void**>( &psl ) );
                windows::check( hres );

                Utf16::Text wpath = targetPath.toWindows( );
                Utf16::Text working = targetPath.parent( ).toWindows( );
                Utf16::Text wargs = toUtf16( args );
                Utf16::Text wdesc = toUtf16( desc );
                Utf16::Text wiconPath = iconPath.toWindows( );

                // Set the path to the shortcut target
                psl->SetPath( wpath );
                psl->SetWorkingDirectory( working );
                if ( !wargs.isEmpty( ) )
                {
                    psl->SetArguments( wargs );
                }
                if ( !wdesc.isEmpty( ) )
                {
                    psl->SetDescription( wdesc );
                }
                if ( !wiconPath.isEmpty( ) )
                {
                    psl->SetIconLocation( wiconPath, iconID );
                }
                // Query IShellLink for the IPersistFile interface for
                // saving the shortcut in persistent storage.
                hres = psl->QueryInterface( IID_IPersistFile, reinterpret_cast<void**>( &ppf ) );
                if ( SUCCEEDED( hres ) )
                {
                    // Save the link by calling IPersistFile::Save.
                    hres = ppf->Save( shortcutPath.toWindows( ).data( ), TRUE );
                    ppf->Release( );
                }
                psl->Release( );
                check( hres );
            }

            void setUninstall(
                const Memory & appKey,
                const Memory & name,
                const Memory & cmd,
                const Memory & location,
                const Memory & publisher,
                const Memory & version,
                const Memory & icon,
                const Memory & productLink,
                const Memory & helpLink)
            {

                String keyName = cpp::format( "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\%", appKey );
                Registry::Key key = Registry::currentUser( ).create( keyName );

                if ( name.length( ) )
                    { key.set( "DisplayName", name ); }
                if ( cmd.length( ) )
                    { key.set( "UninstallString", cmd ); }
                if ( location.length( ) )
                    { key.set( "InstallLocation", location ); }
                if ( publisher.length( ) )
                    { key.set( "Publisher", publisher ); }
                if ( version.length( ) )
                    { key.set( "DisplayVersion", version ); }
                if ( icon.length( ) )
                    { key.set( "DisplayIcon", icon ); }
                if ( helpLink.length( ) )
                    { key.set( "HelpLink", helpLink ); }
                if ( productLink.length( ) )
                    { key.set( "URLInfoAbout", productLink ); }
                key.set( "NoModify", (DWORD)1 );
                key.set( "NoRepair", (DWORD)1 );
            }

            void removeUninstall( const Memory & key )
            {
                String keyName = cpp::format( "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\%", key );
                if ( Registry::currentUser( ).exists( keyName ) )
                    { Registry::currentUser( ).deleteKey( keyName ); }
            }

            void setAppPath( FilePath appFile )
            {
                FilePath appPath = appFile.parent( );

                String keyName = cpp::format( "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\%", appFile.filename() );
                Registry::Key key = Registry::currentUser( ).create( keyName );
                key.set( "", appFile.toWindows( ) );
                key.set( "Path", appFile.parent( ).toWindows( ) );

                auto env = Registry::currentUser( ).open( "Environment" );
                String path = env.get( "Path", 4 * 1024 );
                if ( path.isEmpty( ) )
                {
					path = appPath.toString( );
                    env.set( "Path", path );
                }
                else if ( path.find( appPath.toString( ) ) == String::npos )
                {
                    path += ";" + appPath.toString( );
                    env.set( "Path", path );
                }
            }

            void removeAppPath( FilePath appFile )
            {
                FilePath appPath = appFile.parent( );

                //  check for app path registry
                String keyName = cpp::format( "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\%", appFile.filename( ) );
                if ( Registry::currentUser( ).exists( keyName ) )
                    { Registry::currentUser( ).deleteKey( keyName ); }

                auto env = Registry::currentUser( ).open( "Environment" );
                String path = env.get( "Path", 4 * 1024 );
                if ( path.find( appPath.toString( ) ) != String::npos )
                {
					path = Memory{ path }.replaceFirst( appPath.toString( ), "" );
					path = Memory{ path }.replaceAll( ";;", ";" );
                    if ( path.length( ) < 3 )
                        { path = ""; }
                    env.set( "Path", path );
                }
            }

            void setOpenWith( FilePath appFile, String appLabel, const StringArray & exts )
            {

                String keyName = cpp::format( "Software\\Classes\\%\\shell\\open\\command", appLabel );
                
                auto key = Registry::currentUser( ).create( keyName );
                key.set( "", appFile.toString( true ) + " \"%1\"" );

                key = Registry::currentUser().create( "Software\\Classes\\.txt\\OpenWithProgids" );
                key.set( appLabel, "" );
                
                key = Registry::currentUser( ).create( "Software\\Classes\\.log\\OpenWithProgids" );
                key.set( appLabel, "" );
            }

            // removeOpenWith( "c:\myapp.exe", "myapp.txt", { ".txt", ".log" } );
            void removeOpenWith( FilePath appFile, String appLabel, const StringArray & exts )
            {
                String keyName = cpp::format( "Software\\Classes\\%", appLabel );
                if ( Registry::currentUser( ).exists( keyName + "\\shell\\open\\command" ) )
                {
                    Registry::currentUser( ).deleteKey( keyName + "\\shell\\open\\command" );
                    Registry::currentUser( ).deleteKey( keyName + "\\shell\\open" );
                    Registry::currentUser( ).deleteKey( keyName + "\\shell" );
                    Registry::currentUser( ).deleteKey( keyName );
                }

                for ( const String & ext : exts.data )
                {
                    auto key = Registry::currentUser( ).open( cpp::format( "Software\\Classes\\%\\OpenWithProgids", ext ) );
                    if ( key.isOpen( ) && key.has( appLabel ) )
                        { key.deleteValue( appLabel ); }
                }
            }

        }
    }
}