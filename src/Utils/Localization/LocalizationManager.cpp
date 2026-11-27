//
// Created by innerviewer on 2024-03-17.
//

#include <Utils/Localization/LocalizationManager.h>
#include <Utils/FileSystem/FileSystem.h>
#include <Utils/Resources/ResourceManager.h>

#include <rapidyaml/src/ryml.hpp>

namespace SR_UTILS_NS::Localization {

    LocalizationFile::LocalizationFile(const std::unordered_map<Locale, Path>& localePaths, const Locale& languageToLoad) {
        m_localePaths = localePaths;
        LoadLocalizationStrings(m_localePaths[languageToLoad]);

        //localizationStrings.insert();
    }

    LocalizationFile::~LocalizationFile() {
        //m_watchedFile ///TODO: Что делать с FileWatcher
    }

    void LocalizationFile::LoadLocalizationStrings(const Path& filePath) {
        auto&& pFileContents = FileSystem::ReadFileAsBlob(filePath);
        if (!pFileContents) {
            SR_ERROR("LocalizationFile::LoadLocalizationStrings() : failed to read localization file! Path: {}", filePath);
            return;
        }

        ryml::Tree tree = ryml::parse_in_arena(ryml::to_csubstr(*pFileContents));
        ryml::ConstNodeRef root = tree.crootref();
        for (auto&& n : root.children()) {
            auto&& key = n.key();
            auto&& value = n.val();
            m_locStrings[StringAtom(key.str, key.len)] = String(std::string_view(value.str, value.len));
        }
    }

    void LocalizationFile::SwitchFileByLocale(const Locale& newLocale) {
        if (m_localePaths.contains(newLocale)) {
            LoadLocalizationStrings(m_localePaths[newLocale]);
        }
        else {
            SR_WARN("LocalizationFile::SwitchFileByLocale() : Chosen locale is not supported by file! Locale code: %i"); ///TODO: SR_FORMAT locale code
        }
    }

    UnicodeString LocalizationFile::GetStringById(const StringAtom &id) {
        return m_locStrings[id];
    }

    /*LocalizationGroup::LocalizationGroup(const std::list<LocalizationFile>& locFiles) :
        m_locFiles(locFiles)
    { }

    void LocalizationGroup::AddLocFile(const LocalizationFile& locFile) {
        m_locFiles.emplace_back(locFile);
    }

    void LocalizationGroup::ChangeLocale(Locale newLocale) {
        for (auto&& locFile : m_locFiles) {
            locFile.SwitchFileByLocale(newLocale);
        }
    }

    LocalizationGroup::~LocalizationGroup() {
        ///
    } */

    void LocalizationManager::MakeLocGroup(std::unordered_map<StringAtom, LocalizationFile>& locFiles,
                                           const StringAtom& groupId) {
        Vector<LocalizationFile*> localizationGroup;
        localizationGroup.reserve(locFiles.size());
        for (auto&& file : locFiles) {
            localizationGroup.emplace_back(&file.second);
            m_localizationFiles.insert(std::move(file));
        }
        m_localizationGroups[groupId] = std::move(localizationGroup);
    }

    void LocalizationManager::RegisterLocGroup(const Vector<StringAtom> &locFileIds, const StringAtom &groupId) {
        Vector<LocalizationFile*> locFiles;
        locFiles.reserve(locFileIds.size());
        for (auto&& id : locFileIds) {
            locFiles.emplace_back(&m_localizationFiles[id]);
        }
        m_localizationGroups[groupId] = std::move(locFiles);
    }

    void LocalizationManager::RemoveLocGroup(const StringAtom& id) {
        m_localizationGroups.erase(id);
        ///TODO: что делать с файлами в m_localizationGroups
    }

    void LocalizationManager::ChangeLocGroupLocale(const StringAtom& id, const Locale& newLocale) {
        for (auto&& file : m_localizationGroups[id]) {
            file->SwitchFileByLocale(newLocale);
        }
    }

    void LocalizationManager::AddLocFileIntoGroup(const StringAtom &fileId, const StringAtom &groupId) {

    }

    void LocalizationManager::RemoveLocFileFromGroup(const StringAtom &fileId, const StringAtom &groupId) {

    }

    StringAtom LocalizationManager::MakeLocFile(const std::unordered_map<Locale, Path>& localePaths,
                                                const Locale& languageToLoad,
                                                StringAtom fileId) {
        LocalizationFile locFile = LocalizationFile(localePaths, languageToLoad);
        m_localizationFiles[fileId] = locFile;
        return fileId;
    }

    void LocalizationManager::LoadInfoAsConfigFile(const SpaRcle::Utils::Path &filePath) {
        Path resourcesFolder = SR_UTILS_NS::ResourceManager::Instance().GetResPath();
        auto&& pFileContents = FileSystem::ReadFileAsBlob(filePath.ToString());
        if (!pFileContents) {
            SR_ERROR("LocalizationManager::LoadInfoAsConfigFile() : failed to read localization config file! Path: {}", filePath);
            return;
        }

        ryml::Tree tree = ryml::parse_in_place(pFileContents->data());
        ryml::ConstNodeRef root = tree.crootref();

        for (auto&& locGroup : root.children()) {
            Vector<StringAtom> locFileIds;
            locFileIds.reserve(locGroup.num_children());

            for (auto&& locFile : locGroup.children()) {
                auto&& languageKey = locFile[0].key();
                Locale languageToLoad = std::string_view(languageKey.str, languageKey.len);
                std::unordered_map<Locale, Path> localePaths;

                for (auto&& localePath : locFile) {
                    auto&& pathValue = localePath.val();
                    auto&& localeKey = localePath.key();
                    localePaths[Locale(std::string_view(localeKey.str, localeKey.len))] = resourcesFolder.Concat(std::string_view(pathValue.str, pathValue.len));
                }

                auto&& fileKey = locFile.key();
                locFileIds.emplace_back(MakeLocFile(localePaths, languageToLoad, StringAtom(fileKey.str, fileKey.len)));
            }

            auto&& groupKey = locGroup.key();
            RegisterLocGroup(locFileIds, StringAtom(groupKey.str, groupKey.len));
        }
    }

    //void LocalizationManager::RegisterTextComponent(std::string &id,
    //                                                SR_GTYPES_NS::ITextComponent *textComponent) {
    //    m_textComponents.insert(textComponent);
    //}
}
