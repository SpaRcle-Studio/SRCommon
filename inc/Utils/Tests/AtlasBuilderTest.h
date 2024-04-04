#ifndef SRENGINE_ATLASBUILDERTEST_H
#define SRENGINE_ATLASBUILDERTEST_H

#include <Graphics/Utils/AtlasBuilder.h>
#include <Utils/Resources/ResourceManager.h>

namespace SR_UTILS_NS {

    class AtlasBuilderTest {
    public:
        static bool Test() {
    /*        auto&& resourcePath =*/
//

            auto&& spritesFolder = SR_UTILS_NS::ResourceManager::Instance().GetResPath().Concat("Tests/Sprites");
            SR_LOG(spritesFolder.ToString());

            SR_GRAPH_NS::AtlasBuilder builder(spritesFolder);

            return true;
        }
    };
}



#endif //SRENGINE_ATLASBUILDERTEST_H
