#include <Utils/stdInclude.h>

#include "../src/Utils/Platform/PlatformCommon.cpp"

#ifdef SR_LINUX
    #include "../src/Utils/Platform/PlatformLinux.cpp"
    #include "../src/Utils/Platform/StacktraceLinux.cpp"
#endif

#ifdef SR_WIN32
    #include "../src/Utils/Platform/PlatformWindows.cpp"
    #include "../src/Utils/Platform/StacktraceWindows.cpp"
#endif

#ifdef SR_ANDROID
    #include "../src/Utils/Platform/StacktraceAndroid.cpp"
    #include "../src/Utils/Platform/PlatformAndroid.cpp"
    #include "../src/Utils/Platform/AndroidNativeAppGlue.c"
#endif

#include "../src/Utils/Platform/PlatformSharedVars.cpp"

#ifdef SR_COMMON_USE_TINY_FILE_DIALOGS
    #include "../src/Utils/Platform/MessageBoxDesktop.cpp"
#else
    #ifdef SR_ANDROID
        #include "../src/Utils/Platform/MessageBoxAndroid.cpp"
    #endif
#endif
