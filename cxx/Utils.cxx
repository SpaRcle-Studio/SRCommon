#include <Utils/stdInclude.h>

#include "../src/Utils/SRLM/LogicalMachine.cpp"
#include "../src/Utils/SRLM/DataType.cpp"
#include "../src/Utils/SRLM/DataTypeManager.cpp"
#include "../src/Utils/SRLM/DataOperators.cpp"
#include "../src/Utils/SRLM/LogicalNode.cpp"
#include "../src/Utils/SRLM/LogicalNodes.cpp"
#include "../src/Utils/SRLM/LogicalNodeManager.cpp"
#include "../src/Utils/SRLM/ConvertorNode.cpp"

#include "../src/Utils/Events/EventManager.cpp"
#include "../src/Utils/Events/Event.cpp"
#include "../src/Utils/Events/EventDispatcher.cpp"

#include "../src/Utils/FileSystem/FileSystem.cpp"
#include "../src/Utils/FileSystem/Path.cpp"
#include "../src/Utils/FileSystem/FileDialog.cpp"
#include "../src/Utils/FileSystem/AssimpCache.cpp"

#include "../src/Utils/Input/InputSystem.cpp"
#include "../src/Utils/Input/InputDispatcher.cpp"
#include "../src/Utils/Input/InputDevice.cpp"
#include "../src/Utils/Input/InputHandler.cpp"

#include "../src/Utils/Math/Matrix3x3.cpp"
#include "../src/Utils/Math/Matrix4x4.cpp"
#include "../src/Utils/Math/Quaternion.cpp"
#include "../src/Utils/Math/Vector3.cpp"
#include "../src/Utils/Math/Vector6.cpp"
#include "../src/Utils/Math/Noise.cpp"
#include "../src/Utils/Math/Rect.cpp"

#include "../src/Utils/TaskManager/TaskManager.cpp"
#include "../src/Utils/TaskManager/ThreadWorker.cpp"

#include "../src/Utils/Settings.cpp"
#include "../src/Utils/DebugDraw.cpp"
#include "../src/Utils/Debug.cpp"

#include "../src/Utils/Resources/FileWatcher.cpp"
#include "../src/Utils/Resources/IResource.cpp"
#include "../src/Utils/Resources/ResourceInfo.cpp"
#include "../src/Utils/Resources/ResourcesHolder.cpp"
#include "../src/Utils/Resources/ResourceManager.cpp"
#include "../src/Utils/Resources/ResourceContainer.cpp"
#include "../src/Utils/Resources/IResourceReloader.cpp"
#include "../src/Utils/Resources/ResourceEmbedder.cpp"
#include "../src/Utils/Resources/Xml.cpp"
#include "../src/Utils/Resources/Yaml.cpp"

#include "../src/Utils/CommandManager/ICommand.cpp"
#include "../src/Utils/CommandManager/CmdManager.cpp"
#include "../src/Utils/CommandManager/ReversibleCommand.cpp"

#include "../src/Utils/TypeTraits/PropertyManager.cpp"
#include "../src/Utils/TypeTraits/Property.cpp"
#include "../src/Utils/TypeTraits/Properties.cpp"
#include "../src/Utils/TypeTraits/StandardProperty.cpp"
#include "../src/Utils/TypeTraits/SRClass.cpp"

#include "../src/Utils/Game/LookAtComponent.cpp"
#include "../src/Utils/Game/DebugLogComponent.cpp"

#include "../src/Utils/Localization/LocalizationManager.cpp"

#ifdef SR_TRACY_ENABLE
    #include "../src/Utils/Profile/TracyContext.cpp"
#endif

#include "../libs/xxHash/xxhash.c"