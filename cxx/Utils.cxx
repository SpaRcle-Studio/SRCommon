#include <Utils/stdInclude.h>

#include "../libs/cssparser/cssparser/CSSParser.cpp"
#include "../libs/cssparser/cssparser/CSSProperties.cpp"
#include "../libs/cssparser/cssparser/CSSUtils.cpp"

#ifdef SR_COMMON_EMBED_RESOURCES
    #include <EmbedResources.cxx>
#endif

#ifdef SR_COMMON_ASIO
    #include "../src/Utils/Network/Asio/AsioContext.cpp"
    #include "../src/Utils/Network/Asio/AsioTCPSocket.cpp"
    #include "../src/Utils/Network/Asio/AsioTCPAcceptor.cpp"
    #include "../src/Utils/Network/Asio/AsioPinger.cpp"
    #include "../src/Utils/Network/Asio/AsioICMPSocket.cpp"
#endif

#ifdef SR_LINUX
    #include "../src/Utils/Platform/PlatformLinux.cpp"
    #include "../src/Utils/Platform/StacktraceLinux.cpp"

    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

#ifdef SR_EMSCRIPTEN
    #include "../src/Utils/Platform/PlatformEmscripten.cpp"
    #include "../src/Utils/Platform/StacktraceEmscripten.cpp"
#endif

#ifdef SR_WIN32
    #include "../src/Utils/Platform/PlatformWindows.cpp"
    #include "../src/Utils/Platform/StacktraceWindows.cpp"
#endif

#ifdef SR_ANDROID
    #include "../src/Utils/Platform/StacktraceAndroid.cpp"
    #include "../src/Utils/Platform/PlatformAndroid.cpp"
    #include "../src/Utils/Platform/AndroidNativeAppGlue.c"
    #include "../src/Utils/Platform/AndroidEvent.cpp"
#endif

#ifdef SR_COMMON_USE_TINY_FILE_DIALOGS
    #include "../src/Utils/Platform/MessageBoxDesktop.cpp"
#else
    #ifdef SR_ANDROID
        #include "../src/Utils/Platform/MessageBoxAndroid.cpp"
    #elif defined(SR_EMSCRIPTEN)
        #include "../src/Utils/Platform/MessageBoxEmscripten.cpp"
    #endif
#endif

#include "../src/Utils/FileSystem/MappedFile.cpp"

#include "../src/Utils/Profile/TracyContext.cpp"

#include "../libs/xxHash/xxhash.c"
