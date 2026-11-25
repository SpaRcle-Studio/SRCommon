//
// Created by Codex on 25.11.2026.
//

#include <Utils/Tests/SaveStorageProbe.h>
#include <Utils/Game/SaveSystem.h>
#include <Utils/Game/SaveStorage.h>
#include <Utils/FileSystem/VFS.h>

#include <Codegen/SaveStorageProbe.generated.hpp>

namespace SR_UTILS_NS::SaveProbeDetails {
    struct ProbeScope {
        ProbeScope() {
            ResetTransientState();
        }

        ~ProbeScope() {
            ResetTransientState();
        }

    private:
        static void ResetTransientState() {
            SaveSystem::Instance().SetFaultPoint("");
            for (const StringView slot : { "atomic", "values", "files_only", "restart" }) {
                SaveSystem::Instance().CloseSlot("probe", slot);
            }
        }
    };

    bool ResetRunStorage() {
        for (const StringView slot : { "atomic", "values", "files_only" }) {
            if (!SaveSystem::Instance().DeleteSlot("probe", slot)) {
                return false;
            }
        }
        const auto root = SaveSystem::Instance().GetSaveDirectory().Concat("probe");
        for (const StringView filename : { "probe-source.bin", "probe-export.bin" }) {
            const auto path = root.Concat(filename);
            if (path.IsExists()) {
                VFS::Instance().Delete(path.ToStringView());
                if (path.IsExists()) {
                    return false;
                }
            }
        }
        return true;
    }

    JsonValue State(int64_t revision) {
        JsonValue state;
        state.Parse("{\"text\":\"hello\",\"boolean\":true,\"integer\":42,\"nested\":{\"number\":1.25,\"items\":[1,\"two\",false]}}");
        SaveStorage::SetInteger(state, "revision", revision);
        return state;
    }

    bool Write(StringView slotId, int64_t revision) {
        auto pSlot = SaveSystem::Instance().OpenSlot("probe", slotId);
        if (!pSlot || !pSlot->SetState(State(revision))) {
            return false;
        }
        return SaveSystem::Instance().FlushSlot("probe", slotId);
    }

    SaveSlot::Ptr Fresh(StringView id) {
        SaveSystem::Instance().CloseSlot("probe", id);
        return SaveSystem::Instance().OpenSlot("probe", id);
    }

    bool Revision(StringView id, int64_t revision, bool recovered = false) {
        auto pSlot = Fresh(id);
        return pSlot && pSlot->GetState().ToJson() == State(revision).ToJson() && pSlot->WasRecovered() == recovered;
    }

    String CheckGenericStorage() {
        const StringView values[] = {"null", "true", "42", "3.5", "\"plain text\"", "[1,false,\"text\"]", "{\"nested\":[1,2]}"};
        for (const auto value : values) {
            JsonValue state;
            auto pSlot = SaveSystem::Instance().OpenSlot("probe", "values");
            if (!state.Parse(value) || !pSlot || !pSlot->SetState(state) || !SaveSystem::Instance().FlushSlot("probe", "values")) {
                return "FAILED: default metadata/scalar/array state write";
            }
            pSlot = Fresh("values");
            if (!pSlot || !pSlot->HasState() || pSlot->GetState().ToJson() != state.ToJson()) {
                return "FAILED: scalar/array state round-trip";
            }
        }

        String bytes;
        bytes.resize(5 * 1024 * 1024);
        for (uint64_t i = 0; i < bytes.size(); ++i) {
            bytes[i] = static_cast<char>(i % 256);
        }
        const auto root = SaveSystem::Instance().GetSaveDirectory().Concat("probe");
        const auto source = root.Concat("probe-source.bin");
        const auto exported = root.Concat("probe-export.bin");
        if (!SaveStorage::Write(source, bytes)) {
            return "FAILED: binary source preparation";
        }
        auto pSlot = SaveSystem::Instance().OpenSlot("probe", "files_only");
        if (!pSlot) {
            return "FAILED: cannot open files-only probe slot";
        }
        JsonValue metadata;
        metadata.MakeObject();
        SaveStorage::SetInteger(metadata, "schemaVersion", 999);
        pSlot->SetMetadata(metadata);
        if (!pSlot->ImportFile("attachments.test/payload.bin", source) || !pSlot->SetFile("empty.bin", "") ||
            pSlot->SetFile("../escape.bin", bytes) || pSlot->SetFile("attachments.test", "conflict") ||
            !SaveSystem::Instance().FlushSlot("probe", "files_only")) {
            return "FAILED: files-only import/default metadata/path validation";
        }
        pSlot = Fresh("files_only");
        if (!pSlot || pSlot->HasState() || !pSlot->HasFile("empty.bin") || pSlot->EnumerateFiles().GetCount() != 2 ||
            pSlot->GetFile("attachments.test/payload.bin") != bytes || pSlot->GetMetadata().Get("schemaVersion").GetInt() != 999 ||
            !pSlot->ExportFile("attachments.test/payload.bin", exported)) {
            return "FAILED: files-only round-trip, application schema preservation or export";
        }
        String restored;
        if (!SaveStorage::Read(exported, restored) || restored != bytes) {
            return "FAILED: binary export differs from original file";
        }
        const auto slotPath = root.Concat("files_only");
        const auto generationPath = slotPath.Concat("gen_{}"_format(pSlot->GetMetadata().Get("generation").GetInt()));
        if (generationPath.Concat("state.json").IsExists()) {
            return "FAILED: a files-only slot must not require state.json";
        }
        SaveSlot limited;
        limited.Init("probe", "files_only");
        limited.SetReadLimit(1024);
        if (limited.Load(slotPath)) {
            return "FAILED: configured read budget was ignored";
        }

        if (!pSlot->SetFile("attachments.test/payload.bin", "new file revision") || !SaveSystem::Instance().SetFaultPoint("after_files") ||
            SaveSystem::Instance().FlushSlot("probe", "files_only")) {
            return "FAILED: file interruption injection";
        }
        pSlot = Fresh("files_only");
        if (!pSlot || pSlot->GetFile("attachments.test/payload.bin") != bytes ||
            !pSlot->SetFile("attachments.test/payload.bin", "new file revision") ||
            !SaveSystem::Instance().FlushSlot("probe", "files_only")) {
            return "FAILED: file interruption did not preserve previous publication";
        }
        pSlot = Fresh("files_only");
        if (!pSlot) {
            return "FAILED: cannot reopen committed file generation";
        }
        const auto newGeneration = slotPath.Concat("gen_{}"_format(pSlot->GetMetadata().Get("generation").GetInt()));
        if (!SaveStorage::Write(newGeneration.Concat("files/attachments.test/payload.bin"), "damaged")) {
            return "FAILED: cannot inject corrupt file";
        }
        pSlot = Fresh("files_only");
        if (!pSlot || !pSlot->WasRecovered() || pSlot->GetFile("attachments.test/payload.bin") != bytes ||
            !pSlot->RemoveFile("attachments.test/payload.bin") || !SaveSystem::Instance().FlushSlot("probe", "files_only")) {
            return "FAILED: corrupt file fallback or file removal";
        }
        pSlot = Fresh("files_only");
        if (!pSlot || pSlot->HasFile("attachments.test/payload.bin") || !pSlot->HasFile("empty.bin")) {
            return "FAILED: removed file survived publication";
        }
        if (!SaveSystem::Instance().DeleteSlot("probe", "values") || !SaveSystem::Instance().DeleteSlot("probe", "files_only")) {
            return "FAILED: generic probe slot cleanup";
        }
        VFS::Instance().Delete(source.ToStringView());
        VFS::Instance().Delete(exported.ToStringView());
        if (source.IsExists() || exported.IsExists()) {
            return "FAILED: generic probe temporary file cleanup";
        }
        return "generic values + optional metadata + binary import/export + file atomicity/fallback + read budget: PASS\n";
    }
}

namespace SR_UTILS_NS {
    String SaveStorageProbe::Run() {
        SR_TRACY_ZONE;
        SaveProbeDetails::ProbeScope scope;
        if (!SaveProbeDetails::ResetRunStorage()) {
            return "FAILED: cannot remove data left by a previous probe run";
        }
        const String generic = SaveProbeDetails::CheckGenericStorage();
        if (generic.starts_with("FAILED:")) {
            return generic;
        }
        const auto path = SaveSystem::Instance().GetSaveDirectory().Concat("probe").Concat("atomic");
        if (SaveStorage::Digest("") != "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" ||
            SaveStorage::Digest("abc") != "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") {
            return "FAILED: SHA-256 known vectors";
        }
        if (SaveSystem::Instance().OpenSlot("../escape", "slot") || SaveSystem::Instance().OpenSlot("probe", "C:/escape")) {
            return "FAILED: unsafe IDs accepted";
        }
        if (!SaveProbeDetails::Write("atomic", 1) || !SaveProbeDetails::Revision("atomic", 1)) {
            return "FAILED: physical typed-value round-trip: " + SaveSystem::Instance().GetError();
        }
        String trace = generic + "SHA256 + IDs + disk round-trip: PASS\n";
        const StringView points[] = {"after_state", "after_metadata", "before_marker"};
        for (const StringView point : points) {
            SaveSystem::Instance().SetFaultPoint(point);
            if (SaveProbeDetails::Write("atomic", 2) || !SaveProbeDetails::Revision("atomic", 1)) {
                return trace + "FAILED: previous save was not preserved at " + String(point);
            }
            trace += "interruption {} -> current revision 1: PASS\n"_format(point);
        }
        SaveSystem::Instance().SetFaultPoint("after_marker");
        if (!SaveProbeDetails::Write("atomic", 3) || !SaveProbeDetails::Revision("atomic", 3)) {
            return trace + "FAILED: interruption after marker did not commit revision 3";
        }
        trace += "after_marker -> current revision 3: PASS\n";
        if (!SaveProbeDetails::Write("atomic", 4)) {
            return trace + "FAILED: normal overwrite";
        }
        auto pSlot = SaveProbeDetails::Fresh("atomic");
        if (!pSlot) {
            return trace + "FAILED: cannot reopen atomic probe slot";
        }
        const auto currentGeneration = "gen_{}"_format(pSlot->GetMetadata().Get("generation").GetInt());
        if (!SaveStorage::Write(path.Concat("current"), "../invalid") || !SaveProbeDetails::Revision("atomic", 4, true)) {
            return trace + "FAILED: invalid marker fallback";
        }
        trace += "invalid marker -> latest published revision 4, recovered=true: PASS\n";
        String originalState;
        String originalMetadata;
        const auto generationPath = path.Concat(currentGeneration);
        if (!SaveStorage::Read(generationPath.Concat("state.json"), originalState) ||
            !SaveStorage::Read(generationPath.Concat("metadata.json"), originalMetadata) ||
            !SaveStorage::Write(path.Concat("current"), currentGeneration) ||
            !SaveStorage::Write(generationPath.Concat("state.json"), "corrupt") ||
            !SaveProbeDetails::Revision("atomic", 3, true)) {
            return trace + "FAILED: checksum/previous generation fallback";
        }
        trace += "damaged current payload -> previous revision 3, recovered=true: PASS\n";
        JsonValue future;
        future.Parse(originalMetadata);
        SaveStorage::SetInteger(future, "storageVersion", 999);
        if (!SaveStorage::Write(generationPath.Concat("state.json"), originalState) ||
            !SaveStorage::Write(generationPath.Concat("metadata.json"), future.ToJson()) || SaveProbeDetails::Fresh("atomic")) {
            return trace + "FAILED: future schema accepted";
        }
        String unchanged;
        if (!SaveStorage::Read(generationPath.Concat("metadata.json"), unchanged) || unchanged != future.ToJson()) {
            return trace + "FAILED: future schema overwritten";
        }
        trace += "future schema -> rejected, bytes preserved: PASS\n";
        if (!SaveStorage::Write(generationPath.Concat("metadata.json"), originalMetadata)) {
            return trace + "FAILED: restore probe metadata";
        }
        SaveSystem::Instance().CloseSlot("probe", "atomic");
        if (SaveSystem::Instance().EnumerateSlots("probe").GetCount() == 0 || !SaveProbeDetails::Revision("atomic", 4)) {
            return trace + "FAILED: disk enumeration after clearing process slots";
        }
        const auto generations = SaveStorage::Generations(path);
        if (generations.size() != 2) {
            return trace + "FAILED: retention must keep current and previous valid generations";
        }
        if (!SaveSystem::Instance().DeleteSlot("probe", "atomic") || path.IsExists()) {
            return trace + "FAILED: delete";
        }
        return trace + "enumeration + two-generation retention + delete: PASS\nPASSED_IN_THIS_RUN";
    }

    String SaveStorageProbe::PrepareRestart() {
        SR_TRACY_ZONE;
        SaveProbeDetails::ProbeScope scope;
        if (!SaveSystem::Instance().DeleteSlot("probe", "restart")) {
            return "FAILED: cannot reset restart probe slot: " + SaveSystem::Instance().GetError();
        }
        if (!SaveProbeDetails::Write("restart", 42)) {
            return "FAILED: " + SaveSystem::Instance().GetError();
        }
        return "RESTART_REQUIRED: close the entire application, launch again, then VerifyRestart. Path: " +
            SaveSystem::Instance().GetSaveDirectory().ToString();
    }

    String SaveStorageProbe::VerifyRestart() {
        SR_TRACY_ZONE;
        SaveProbeDetails::ProbeScope scope;
        if (!SaveProbeDetails::Revision("restart", 42) || SaveSystem::Instance().EnumerateSlots("probe").GetCount() == 0) {
            return "FAILED: restart payload/enumeration: " + SaveSystem::Instance().GetError();
        }
        return "RESTART_PAYLOAD_VALID: revision 42 and typed tree read from disk; process restart must be confirmed by operator";
    }
}
