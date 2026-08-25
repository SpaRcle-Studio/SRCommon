//
// Created by Monika on 08.08.2026.
//

#include <Utils/Flux/IR/FluxProgram.h>
#include <Utils/Serialization/JsonSerialization.h>
#include <Utils/Memory/Allocator.h>

namespace SR_FLUX_NS {
    void FluxProgram::SaveToString(String& out) const {
        SR_TRACY_ZONE;
        out.clear();

        out += "registers {}\n\n"_format(requiredRegisters);

        out += "consts\n";
        for (const auto& constant : constants) {
            out += "\t" + constant.type + " = " + constant.value + "\n";
        }
        out += "endconsts\n\n";
        out += "storage\n";
        for (const auto& storageVar : storage) {
            out += "\t" + storageVar.type + " = " + storageVar.value +"\n";
        }
        out += "endstorage\n\n";

        Vector<uint32_t> labelInstructionPointers;
        labelInstructionPointers.resize(labels.size());
        for (const auto& label : labels) {
            labelInstructionPointers.emplace_back(label.instructionPointer);
        }

        uint32_t instructionIndex = 0;
        bool insideLabel = false;
        for (const auto& instruction : instructions) {
            for (const auto& label : labels) {
                if (label.instructionPointer == instructionIndex) {
                    if (insideLabel) {
                        out += "\n";
                    }
                    out += label.name + ":\n";
                    insideLabel = true;
                    break;
                }
            }

            if (insideLabel) {
                out += "\t";
            }

            for (auto&& mapping : OPCODE_MAPPINGS) {
                if (mapping.opcode == instruction.opcode) {
                    out += mapping.name;
                    break;
                }
            }

            if (!instruction.operands.empty() || !instruction.callable.object.empty()) {
                out += " ";
            }

            if (!instruction.callable.object.empty()) {
                out += instruction.callable.object.ToStringView();
                /// у cast имя функции не задаётся - в объекте лежит целевой тип
                if (!instruction.callable.function.empty()) {
                    out += ".";
                    out += instruction.callable.function.ToStringView();
                }
                out += " ";
            }

            uint32_t operandOffset = 0;
            if (instruction.opcode == FluxOpcode::Branch || instruction.opcode == FluxOpcode::Jump) {
                if (!instruction.operands.empty()) {
                    const auto labelIndex = instruction.operands[0];
                    operandOffset++;
                    if (labelIndex < labels.size()) {
                        out += labels[labelIndex].name;
                    }
                    else {
                        out += "error_invalid_label";
                    }
                }
            }

            for (const auto& operand : instruction.operands | std::views::drop(operandOffset)) {
                if (operand < constants.size()) {
                    out += "@" + std::to_string(operand) + " ";
                }
                else if (operand < constants.size() + storage.size()) {
                    out += "$" + std::to_string(operand - constants.size()) + " ";
                }
                else {
                    out += "%" + std::to_string(operand - constants.size() - storage.size()) + " ";
                }
            }

            out += "\n";
            ++instructionIndex;
        }
    }

    void FluxProgram::CloneTo(FluxProgram& clone) const {
        std::exchange(clone.constants, {});
        std::exchange(clone.storage, {});
        std::exchange(clone.instructions, {});
        std::exchange(clone.labels, {});

        clone.allocator = (IAllocator*)new MonotonicAllocator(allocator->GetUsedMemory());
        clone.requiredRegisters = requiredRegisters;

        clone.instructions = Vector<FluxInstruction>(clone.allocator.Get());
        clone.instructions.reserve(instructions.size());

        clone.constants = Vector<FluxVariable>(clone.allocator.Get());
        clone.constants.reserve(constants.size());

        clone.storage = Vector<FluxVariable>(clone.allocator.Get());
        clone.storage.reserve(storage.size());

        clone.labels = Vector<FluxLabel>(clone.allocator.Get());
        clone.labels.reserve(labels.size());

        for (const auto& instruction : instructions) {
            auto&& newInstruction = clone.instructions.emplace_back();
            newInstruction.opcode = instruction.opcode;
            newInstruction.operands = Vector<FluxRegisterId>(clone.allocator.Get(), instruction.operands.begin(), instruction.operands.end());
            newInstruction.callable = instruction.callable;
            newInstruction.debugId = instruction.debugId;
        }

        for (const auto& constant : constants) {
            auto&& newConstant = clone.constants.emplace_back();
            newConstant.type = String(constant.type, clone.allocator.Get());
            newConstant.value = String(constant.value, clone.allocator.Get());
        }

        for (const auto& storageVar : storage) {
            auto&& newStorageVar = clone.storage.emplace_back();
            newStorageVar.type = String(storageVar.type, clone.allocator.Get());
            newStorageVar.value = String(storageVar.value, clone.allocator.Get());
        }

        for (const auto& label : labels) {
            auto&& newLabel = clone.labels.emplace_back();
            newLabel.name = String(label.name, clone.allocator.Get());
            newLabel.instructionPointer = label.instructionPointer;
        }
    }

    FluxProgram::~FluxProgram() {
        std::exchange(instructions, {});
        std::exchange(constants, {});
        std::exchange(storage, {});
        std::exchange(labels, {});
    }

    FluxProgram::FluxProgram(const FluxProgram& other) {
        other.CloneTo(*this);
    }

    FluxProgram& FluxProgram::operator=(const FluxProgram& other) {
        if (this != &other) {
            other.CloneTo(*this);
        }
        return *this;
    }

    FluxProgram::FluxProgram(FluxProgram&& other) noexcept {
        requiredRegisters = std::move(other.requiredRegisters);
        instructions = std::move(other.instructions);
        constants = std::move(other.constants);
        storage = std::move(other.storage);
        labels = std::move(other.labels);
        allocator = std::move(other.allocator);
    }

    FluxProgram& FluxProgram::operator=(FluxProgram&& other) noexcept {
        if (this != &other) {
            requiredRegisters = std::move(other.requiredRegisters);
            instructions = std::move(other.instructions);
            constants = std::move(other.constants);
            storage = std::move(other.storage);
            labels = std::move(other.labels);
            allocator = std::move(other.allocator);
        }
        return *this;
    }
}
