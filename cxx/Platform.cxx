#include <Utils/stdInclude.h>

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
