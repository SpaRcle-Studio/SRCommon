//
// Created by Igor on 04/11/2022.
//

#include <Utils/CommandManager/ReversibleCommand.h>
#include <Utils/Resources/Xml.h>

namespace SR_UTILS_NS {
    bool ReversibleCommand::Load(const Xml::Node&) {
        return false;
    }

    Xml::Node ReversibleCommand::Save() const {
        return Xml::Node();
    }

    bool GroupCommand::Load(const Xml::Node&) {
        return false;
    }

    Xml::Node GroupCommand::Save() const {
        return Xml::Node();
    }

    GroupCommand::GroupCommand(Vector<ReversibleCommand*>&& commands)
        : m_commands(std::move(commands))
    { }

    GroupCommand::~GroupCommand() {
        for (ReversibleCommand* command : m_commands) {
            SR_SAFE_DELETE_PTR(command);
        }
    }

    bool GroupCommand::Redo() {
        for (ReversibleCommand* command : m_commands) {
            command->Redo();
        }

        return true;
    }

    bool GroupCommand::Undo() {
        for (int64_t i = m_commands.size() - 1; i >= 0; --i) {
            m_commands[i]->Undo();
        }

        return true;
    }


}