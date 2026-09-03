//
// Created by Monika on 03.09.2026.
//

#include <Utils/Flux/Graph/FluxGraphCompiler.h>
#include <Utils/Flux/Graph/FluxGraphCompileContext.h>
#include <Utils/Flux/Graph/FluxGraph.h>
#include <Utils/Flux/Graph/FluxCompileUtils.h>
#include <Utils/Flux/IR/FluxProgram.h>
#include <Utils/Flux/IR/FluxProgramOptimizer.h>
#include <Utils/Memory/Allocator.h>
#include <Utils/Memory/MemoryLiterals.h>

namespace SR_FLUX_NS {
    FluxGraphCompiler::FluxGraphCompiler(const FluxGraph& graph)
        : m_graph(graph)
    {
        m_arithmeticObject = StringAtom("MathUtils");
        m_addFunction = StringAtom("Add");
        m_lessFunction = StringAtom("Less");
    }

    /// =================================================== Поток ======================================================

    void FluxGraphCompiler::CompileEventNode(FluxGraphCompileContext& context) const {
        auto&& program = *context.program;
        const uint32_t nodeIndex = context.nodeIndex;
        auto&& node = m_graph.GetNodes()[nodeIndex];

        {
            /// имя метки является именем события: среда исполнения ищет точку входа именно по нему
            FluxLabel& label = program.labels.emplace_back();
            label.instructionPointer = static_cast<uint32_t>(program.instructions.size());
            label.name = String(program.allocator.Get());
            if (!node.GetName().empty()) {
                label.name += node.GetName().ToStringView();
            }
            else {
                FormatTo(label.name, "UnnamedEvent_Label_{}", program.labels.size() - 1);
            }
        }

        /// аргументы события лежат в стеке значений в порядке передачи, поэтому снимаются с конца
        const uint32_t argumentCount = m_graph.GetMaxOutputPin(nodeIndex);

        for (uint32_t pin = argumentCount; pin >= 1; --pin) {
            const uint64_t key = MakeFluxValueKey(nodeIndex, pin);
            const uint32_t useCount = GetUseCount(context, key);

            /// невостребованный аргумент просто снимается со стека в нулевой регистр
            FluxRegisterId destination = context.ToOperand(0);

            if (useCount > 0) {
                FluxValueRef value;
                value.sourceNode = nodeIndex;
                value.sourcePin = pin;
                value.registerIndex = context.AllocateRegister();
                value.operand = context.ToOperand(value.registerIndex);
                value.isRegister = true;
                value.loopDepth = context.loopDepth;

                context.materialized.emplace(key, value);
                context.pendingUses.emplace(key, useCount);

                destination = value.operand;
            }

            auto&& instruction = EmitInstruction(context, FluxOpcode::Pop, nodeIndex);
            instruction.operands.emplace_back(destination);
        }

        CompileFlow(context, GetFlowTarget(nodeIndex, 0), FluxInvalidLabel);
    }

    void FluxGraphCompiler::CompileFlow(FluxGraphCompileContext& context, uint32_t nodeIndex, const uint32_t terminatorLabel) const {
        const uint32_t previousTerminator = context.terminatorLabel;
        context.terminatorLabel = terminatorLabel;
        context.flowTerminated = false;

        while (nodeIndex != FluxInvalidNode && !context.hasErrors) {
            /// узел, в который сходится несколько ветвей, компилируется один раз, а повторные
            /// приходы в него превращаются в переход. Это же снимает проблему циклов в потоке
            if (GetFlowInputCount(context, nodeIndex) > 1) {
                auto&& pIt = context.emittedLabels.find(nodeIndex);
                if (pIt != context.emittedLabels.end()) {
                    EmitJump(context, FluxOpcode::Jump, pIt->second, nodeIndex);
                    context.flowTerminated = true;
                    context.terminatorLabel = previousTerminator;
                    return;
                }
                /// код после метки исполняется всеми входящими путями, поэтому он не имеет права
                /// опираться на значения, вычисленные внутри текущей ветви - остальные пути
                /// приходят сюда переходом и соответствующие регистры не заполняли
                context.PruneToFlowSplitScope();
                const uint32_t labelIndex = CreateLabel(context, "merge");
                BindLabel(context, labelIndex);
                context.emittedLabels.emplace(nodeIndex, labelIndex);
            }

            context.nodeIndex = nodeIndex;
            nodeIndex = CompileNode(context);
        }

        if (!context.hasErrors && !context.flowTerminated) {
            if (terminatorLabel == FluxInvalidLabel) {
                EmitInstruction(context, FluxOpcode::Return, context.nodeIndex);
            }
            else {
                EmitJump(context, FluxOpcode::Jump, terminatorLabel, context.nodeIndex);
            }
            context.flowTerminated = true;
        }

        context.terminatorLabel = previousTerminator;
    }

    uint32_t FluxGraphCompiler::CompileNode(FluxGraphCompileContext& context) const {
        const uint32_t nodeIndex = context.nodeIndex;
        auto&& node = m_graph.GetNodes()[nodeIndex];

        switch (node.GetType()) {
            case FluxGraphNodeType::Invoke:
                return CompileInvokeNode(context, nodeIndex);
            case FluxGraphNodeType::WriteVariable:
                return CompileWriteVariableNode(context, nodeIndex);
            case FluxGraphNodeType::Branch:
                return CompileBranchNode(context, nodeIndex);
            case FluxGraphNodeType::For:
                return CompileForNode(context, nodeIndex);
            case FluxGraphNodeType::While:
                return CompileWhileNode(context, nodeIndex);
            case FluxGraphNodeType::Cast:
                return CompileCastNode(context, nodeIndex);
            case FluxGraphNodeType::Sequence:
                return CompileSequenceNode(context, nodeIndex);
            case FluxGraphNodeType::ParallelSequence:
                return CompileParallelSequenceNode(context, nodeIndex);
            case FluxGraphNodeType::Evaluate:
            case FluxGraphNodeType::Constant:
            case FluxGraphNodeType::ReadVariable:
                SR_ERROR("FluxGraphCompiler::CompileNode() : pure node {} cannot be a part of the execution flow!", nodeIndex);
                context.hasErrors = true;
                return FluxInvalidNode;
            default:
                SRHalt("FluxGraphCompiler::CompileNode() : unhandled node type!");
                context.hasErrors = true;
                return FluxInvalidNode;
        }
    }

    uint32_t FluxGraphCompiler::CompileInvokeNode(FluxGraphCompileContext& context, const uint32_t nodeIndex) const {
        if (!CompileCall(context, nodeIndex, GetCallObjectPin(FluxGraphNodeType::Invoke))) {
            return FluxInvalidNode;
        }

        /// у метода без возвращаемого значения первый выходной пин занят выходным аргументом,
        /// который материализован самим вызовом
        if (!HasFluxResultPin(FluxGraphNodeType::Invoke, m_graph.GetNodes()[nodeIndex].GetCallable().FindMethodMeta())) {
            return GetFlowTarget(nodeIndex, 0);
        }

        /// возвращаемое значение среда исполнения кладёт в нулевой регистр, поэтому оно
        /// переносится в собственный регистр только если у него есть потребители
        const uint64_t key = MakeFluxValueKey(nodeIndex, 1);
        if (const uint32_t useCount = GetUseCount(context, key); useCount > 0) {
            FluxValueRef result;
            result.sourceNode = nodeIndex;
            result.sourcePin = 1;
            result.registerIndex = context.AllocateRegister();
            result.operand = context.ToOperand(result.registerIndex);
            result.isRegister = true;
            result.loopDepth = context.loopDepth;

            EmitBinary(context, FluxOpcode::Move, context.ToOperand(0), result.operand, nodeIndex);

            context.materialized.emplace(key, result);
            context.pendingUses.emplace(key, useCount);
        }

        return GetFlowTarget(nodeIndex, 0);
    }

    uint32_t FluxGraphCompiler::CompileWriteVariableNode(FluxGraphCompileContext& context, const uint32_t nodeIndex) const {
        auto&& node = m_graph.GetNodes()[nodeIndex];

        auto&& pStorageIt = context.storageIndices.find(node.GetName());
        if (pStorageIt == context.storageIndices.end()) {
            SR_ERROR("FluxGraphCompiler::CompileWriteVariableNode() : unknown variable \"{}\"!", node.GetName());
            context.hasErrors = true;
            return FluxInvalidNode;
        }

        auto&& pValueLink = m_graph.FindInputLink(nodeIndex, 1);
        if (!pValueLink) {
            SR_ERROR("FluxGraphCompiler::CompileWriteVariableNode() : value pin of node {} is not connected!", nodeIndex);
            context.hasErrors = true;
            return FluxInvalidNode;
        }

        const FluxValueRef value = EvaluateOutput(context, pValueLink->GetSourceNode(), pValueLink->GetSourcePin());
        if (context.hasErrors) {
            return FluxInvalidNode;
        }

        const auto destination = static_cast<FluxRegisterId>(context.program->constants.size() + pStorageIt->second);
        EmitBinary(context, FluxOpcode::Copy, value.operand, destination, nodeIndex);
        ReleaseValue(context, value);

        return GetFlowTarget(nodeIndex, 0);
    }

    uint32_t FluxGraphCompiler::CompileBranchNode(FluxGraphCompileContext& context, const uint32_t nodeIndex) const {
        const FluxValueRef condition = EvaluateCondition(context, nodeIndex, 1);
        if (context.hasErrors) {
            return FluxInvalidNode;
        }

        const uint32_t trueLabel = CreateLabel(context, "branch_true");
        EmitJump(context, FluxOpcode::Branch, trueLabel, nodeIndex);
        ReleaseValue(context, condition);

        const uint32_t terminator = context.terminatorLabel;

        /// ветви являются альтернативными путями исполнения, поэтому вторая ветвь компилируется
        /// с тем же состоянием распределителя и переиспользует те же регистры
        const FluxRegisterSnapshot snapshot = context.SaveState();

        context.EnterFlowSplit();

        CompileFlow(context, GetFlowTarget(nodeIndex, 1), terminator);

        context.RestoreState(snapshot);

        BindLabel(context, trueLabel);
        CompileFlow(context, GetFlowTarget(nodeIndex, 0), terminator);

        context.LeaveFlowSplit();

        context.flowTerminated = true;
        return FluxInvalidNode;
    }

    uint32_t FluxGraphCompiler::CompileWhileNode(FluxGraphCompileContext& context, const uint32_t nodeIndex) const {
        const uint32_t headLabel = CreateLabel(context, "while_head");
        const uint32_t bodyLabel = CreateLabel(context, "while_body");
        const uint32_t endLabel = CreateLabel(context, "while_end");

        /// область цикла открывается до вычисления условия: условие пересчитывается на каждой
        /// итерации, поэтому значения, вычисленные снаружи, не должны освобождаться внутри
        PushLoopScope(context);
        context.EnterFlowSplit();

        BindLabel(context, headLabel);

        const FluxValueRef condition = EvaluateCondition(context, nodeIndex, 1);
        if (context.hasErrors) {
            return FluxInvalidNode;
        }

        EmitJump(context, FluxOpcode::Branch, bodyLabel, nodeIndex);
        ReleaseValue(context, condition);
        EmitJump(context, FluxOpcode::Jump, endLabel, nodeIndex);

        BindLabel(context, bodyLabel);
        CompileFlow(context, GetFlowTarget(nodeIndex, 0), headLabel);

        BindLabel(context, endLabel);
        context.LeaveFlowSplit();
        PopLoopScope(context);

        /// тело цикла завершено собственным переходом, но поток продолжается после цикла
        context.flowTerminated = false;

        return GetFlowTarget(nodeIndex, 1);
    }

    uint32_t FluxGraphCompiler::CompileForNode(FluxGraphCompileContext& context, const uint32_t nodeIndex) const {
        auto&& pFromLink = m_graph.FindInputLink(nodeIndex, 1);
        auto&& pToLink = m_graph.FindInputLink(nodeIndex, 2);

        if (!pFromLink || !pToLink) {
            SR_ERROR("FluxGraphCompiler::CompileForNode() : range pins of node {} are not connected!", nodeIndex);
            context.hasErrors = true;
            return FluxInvalidNode;
        }

        /// счётчик живёт в собственном регистре всё время работы цикла
        const FluxValueRef from = EvaluateOutput(context, pFromLink->GetSourceNode(), pFromLink->GetSourcePin());
        if (context.hasErrors) {
            return FluxInvalidNode;
        }

        FluxValueRef index;
        index.sourceNode = nodeIndex;
        index.sourcePin = 2;
        index.registerIndex = context.AllocateRegister();
        index.operand = context.ToOperand(index.registerIndex);
        index.isRegister = true;
        index.loopDepth = context.loopDepth;

        EmitBinary(context, FluxOpcode::Copy, from.operand, index.operand, nodeIndex);
        ReleaseValue(context, from);

        const FluxValueRef to = EvaluateOutput(context, pToLink->GetSourceNode(), pToLink->GetSourcePin());
        if (context.hasErrors) {
            return FluxInvalidNode;
        }

        FluxValueRef step;
        if (auto&& pStepLink = m_graph.FindInputLink(nodeIndex, 3)) {
            step = EvaluateOutput(context, pStepLink->GetSourceNode(), pStepLink->GetSourcePin());
            if (context.hasErrors) {
                return FluxInvalidNode;
            }
        }
        else {
            auto&& pConstantIt = context.constantIndices.find(MakeFluxValueKey(nodeIndex, 3));
            if (pConstantIt == context.constantIndices.end()) {
                SR_ERROR("FluxGraphCompiler::CompileForNode() : default step of node {} is missing!", nodeIndex);
                context.hasErrors = true;
                return FluxInvalidNode;
            }
            step.sourceNode = nodeIndex;
            step.sourcePin = 3;
            step.operand = static_cast<FluxRegisterId>(pConstantIt->second);
        }

        /// счётчик освобождается вручную после цикла, поэтому к числу его потребителей
        /// добавляется одно "владеющее" использование
        const uint64_t indexKey = MakeFluxValueKey(nodeIndex, 2);
        context.materialized.emplace(indexKey, index);
        context.pendingUses.emplace(indexKey, GetUseCount(context, indexKey) + 1);

        const uint32_t headLabel = CreateLabel(context, "for_head");
        const uint32_t bodyLabel = CreateLabel(context, "for_body");
        const uint32_t stepLabel = CreateLabel(context, "for_step");
        const uint32_t endLabel = CreateLabel(context, "for_end");

        PushLoopScope(context);
        context.EnterFlowSplit();

        BindLabel(context, headLabel);
        {
            auto&& instruction = EmitInstruction(context, FluxOpcode::Call, nodeIndex);
            instruction.callable.object = m_arithmeticObject;
            instruction.callable.function = m_lessFunction;
            instruction.operands.reserve(2);
            instruction.operands.emplace_back(index.operand);
            instruction.operands.emplace_back(to.operand);
        }
        EmitJump(context, FluxOpcode::Branch, bodyLabel, nodeIndex);
        EmitJump(context, FluxOpcode::Jump, endLabel, nodeIndex);

        BindLabel(context, bodyLabel);
        CompileFlow(context, GetFlowTarget(nodeIndex, 0), stepLabel);

        BindLabel(context, stepLabel);
        {
            auto&& instruction = EmitInstruction(context, FluxOpcode::Call, nodeIndex);
            instruction.callable.object = m_arithmeticObject;
            instruction.callable.function = m_addFunction;
            instruction.operands.reserve(2);
            instruction.operands.emplace_back(index.operand);
            instruction.operands.emplace_back(step.operand);
        }
        EmitBinary(context, FluxOpcode::Move, context.ToOperand(0), index.operand, nodeIndex);
        EmitJump(context, FluxOpcode::Jump, headLabel, nodeIndex);

        BindLabel(context, endLabel);
        context.LeaveFlowSplit();
        PopLoopScope(context);

        ReleaseValue(context, to);
        ReleaseValue(context, step);
        ReleaseValue(context, index);

        /// тело цикла завершено собственным переходом, но поток продолжается после цикла
        context.flowTerminated = false;

        return GetFlowTarget(nodeIndex, 1);
    }

    uint32_t FluxGraphCompiler::CompileCastNode(FluxGraphCompileContext& context, const uint32_t nodeIndex) const {
        auto&& node = m_graph.GetNodes()[nodeIndex];

        if (node.GetName().empty()) {
            SR_ERROR("FluxGraphCompiler::CompileCastNode() : target type of node {} is not specified!", nodeIndex);
            context.hasErrors = true;
            return FluxInvalidNode;
        }

        auto&& pObjectLink = m_graph.FindInputLink(nodeIndex, 1);
        if (!pObjectLink) {
            SR_ERROR("FluxGraphCompiler::CompileCastNode() : object pin of node {} is not connected!", nodeIndex);
            context.hasErrors = true;
            return FluxInvalidNode;
        }

        const FluxValueRef object = EvaluateOutput(context, pObjectLink->GetSourceNode(), pObjectLink->GetSourcePin());
        if (context.hasErrors) {
            return FluxInvalidNode;
        }

        /// приведённое значение переносится в собственный регистр только если у него есть
        /// потребители, иначе оно кладётся в нулевой регистр и сразу затирается признаком успеха
        const uint64_t key = MakeFluxValueKey(nodeIndex, 2);
        const uint32_t useCount = GetUseCount(context, key);

        FluxRegisterId destination = context.ToOperand(0);

        if (useCount > 0) {
            FluxValueRef result;
            result.sourceNode = nodeIndex;
            result.sourcePin = 2;
            result.registerIndex = context.AllocateRegister();
            result.operand = context.ToOperand(result.registerIndex);
            result.isRegister = true;
            result.loopDepth = context.loopDepth;

            context.materialized.emplace(key, result);
            context.pendingUses.emplace(key, useCount);

            destination = result.operand;
        }

        {
            auto&& instruction = EmitInstruction(context, FluxOpcode::Cast, nodeIndex);
            /// целевой тип разрешается средой исполнения по имени через FluxUtils::Cast
            instruction.callable.object = node.GetName();
            instruction.operands.reserve(2);
            instruction.operands.emplace_back(object.operand);
            instruction.operands.emplace_back(destination);
        }

        ReleaseValue(context, object);

        const uint32_t successLabel = CreateLabel(context, "cast_success");
        EmitJump(context, FluxOpcode::Branch, successLabel, nodeIndex);

        const uint32_t terminator = context.terminatorLabel;

        /// ветви являются альтернативными путями исполнения, поэтому вторая ветвь компилируется
        /// с тем же состоянием распределителя и переиспользует те же регистры
        const FluxRegisterSnapshot snapshot = context.SaveState();

        context.EnterFlowSplit();

        CompileFlow(context, GetFlowTarget(nodeIndex, 1), terminator);

        context.RestoreState(snapshot);

        BindLabel(context, successLabel);
        CompileFlow(context, GetFlowTarget(nodeIndex, 0), terminator);

        context.LeaveFlowSplit();

        context.flowTerminated = true;
        return FluxInvalidNode;
    }

    Vector<uint32_t> FluxGraphCompiler::CollectSequenceSteps(const uint32_t nodeIndex) const {
        Vector<uint32_t> steps;
        const uint32_t maxPin = m_graph.GetMaxOutputPin(nodeIndex);
        for (uint32_t pin = 0; pin <= maxPin; ++pin) {
            /// неподключенный пин шагом не является - редактор всегда показывает один свободный
            if (m_graph.FindOutputLink(nodeIndex, pin)) {
                steps.emplace_back(pin);
            }
        }
        return steps;
    }

    uint32_t FluxGraphCompiler::CompileSequenceNode(FluxGraphCompileContext& context, const uint32_t nodeIndex) const {
        const Vector<uint32_t> steps = CollectSequenceSteps(nodeIndex);

        /// у узла без подключенных шагов делать нечего - поток завершается снаружи
        if (steps.empty()) {
            return FluxInvalidNode;
        }

        const uint32_t terminator = context.terminatorLabel;

        /// шаг может завершиться несколькими путями, поэтому начало следующего шага является
        /// точкой слияния: значения, вычисленные внутри шага, до неё не доживают
        context.EnterFlowSplit();

        for (uint32_t i = 0; i < steps.size(); ++i) {
            const bool isLast = i + 1 == steps.size();

            /// последний шаг продолжает поток узла, остальные - следующий шаг
            const uint32_t stepTerminator = isLast ? terminator : CreateLabel(context, "sequence_step");

            CompileFlow(context, GetFlowTarget(nodeIndex, steps[i]), stepTerminator);
            if (context.hasErrors) {
                context.LeaveFlowSplit();
                return FluxInvalidNode;
            }

            if (!isLast) {
                context.PruneToFlowSplitScope();
                BindLabelFolded(context, stepTerminator);
            }
        }

        context.LeaveFlowSplit();

        /// последний шаг уже завершил цепочку сам
        context.flowTerminated = true;
        return FluxInvalidNode;
    }

    uint32_t FluxGraphCompiler::CompileParallelSequenceNode(FluxGraphCompileContext& context, const uint32_t nodeIndex) const {
        const Vector<uint32_t> steps = CollectSequenceSteps(nodeIndex);

        if (steps.empty()) {
            return FluxInvalidNode;
        }

        Vector<uint32_t> labels;
        labels.reserve(steps.size());
        for (uint32_t i = 0; i < steps.size(); ++i) {
            labels.emplace_back(CreateLabel(context, "parallel_branch"));
        }

        {
            auto&& instruction = EmitInstruction(context, FluxOpcode::Fork, nodeIndex);
            instruction.operands.reserve(labels.size());
            for (auto&& labelIndex : labels) {
                instruction.operands.emplace_back(static_cast<FluxRegisterId>(labelIndex));
            }
        }

        const uint32_t terminator = context.terminatorLabel;

        /// родитель ветви не исполняет - он продолжает свой путь. Продолжать здесь нечем, у узла
        /// нет пина продолжения, поэтому путь родителя на этом заканчивается
        if (terminator == FluxInvalidLabel) {
            EmitInstruction(context, FluxOpcode::Return, nodeIndex);
        }
        else {
            EmitJump(context, FluxOpcode::Jump, terminator, nodeIndex);
        }

        /// ветви исполняются каждая в своей копии состояния, поэтому они, как и альтернативные
        /// пути ветвления, компилируются с одним и тем же состоянием распределителя
        const FluxRegisterSnapshot snapshot = context.SaveState();

        context.EnterFlowSplit();

        for (uint32_t i = 0; i < steps.size(); ++i) {
            context.RestoreState(snapshot);
            BindLabel(context, labels[i]);
            /// ветвь является самостоятельным исполнением: дойдя до конца своей цепочки, она
            /// обязана завершиться, а не продолжить код родителя
            CompileFlow(context, GetFlowTarget(nodeIndex, steps[i]), FluxInvalidLabel);
            if (context.hasErrors) {
                break;
            }
        }

        context.LeaveFlowSplit();
        context.RestoreState(snapshot);

        context.flowTerminated = true;
        return FluxInvalidNode;
    }

    /// =================================================== Значения ===================================================

    uint32_t FluxGraphCompiler::GetCallArgumentCount(const uint32_t nodeIndex, const uint32_t firstArgumentPin, const Reflection::Method* pMethod) const {
        /// сигнатура метода является источником истины: у выходного аргумента входной пин может
        /// быть не подключен, поэтому по связям его количество не восстановить
        if (pMethod) {
            return pMethod->GetParamsCount();
        }
        const uint32_t maxPin = m_graph.GetMaxInputPin(nodeIndex);
        return maxPin >= firstArgumentPin ? (maxPin - firstArgumentPin) + 1 : 0;
    }

    FluxValueRef FluxGraphCompiler::MaterializeOutArgument(FluxGraphCompileContext& context, const uint32_t nodeIndex, const uint32_t outputPin, const FluxValueRef& source) const {
        /// источник больше не понадобится. Если это был временный регистр, у которого вызов был
        /// последним потребителем, то распределитель вернёт его же и копия не понадобится
        ReleaseValue(context, source);

        FluxValueRef output;
        output.sourceNode = nodeIndex;
        output.sourcePin = outputPin;
        output.registerIndex = context.AllocateRegister();
        output.operand = context.ToOperand(output.registerIndex);
        output.isRegister = true;
        output.loopDepth = context.loopDepth;

        /// метод изменяет аргумент прямо в переданной ячейке, поэтому она обязана принадлежать
        /// узлу: константы неизменяемы, а значение, у которого остались другие потребители,
        /// портить нельзя
        if (output.operand != source.operand) {
            EmitBinary(context, FluxOpcode::Copy, source.operand, output.operand, nodeIndex);
        }

        const uint64_t key = MakeFluxValueKey(nodeIndex, outputPin);
        context.materialized.emplace(key, output);
        /// к потребителям добавляется одно владеющее использование: регистр освобождает сам узел
        /// сразу после вызова, даже если значение никто не читает
        context.pendingUses.emplace(key, GetUseCount(context, key) + 1);

        return output;
    }

    bool FluxGraphCompiler::CompileCall(FluxGraphCompileContext& context, const uint32_t nodeIndex, const uint32_t objectPin) const {
        auto&& node = m_graph.GetNodes()[nodeIndex];

        /// объект синглтона в инструкцию не передаётся - среда исполнения разрешит его по имени,
        /// поэтому пин объекта у такого вызова занят первым аргументом. У остальных вызовов
        /// неподключенный пин объекта тоже не даёт операнда, и вызов разрешается по имени
        const bool isSingleton = IsSingletonCallable(node.GetCallable());
        const uint32_t firstArgumentPin = isSingleton ? objectPin : objectPin + 1;

        auto&& pObjectLink = isSingleton ? nullptr : m_graph.FindInputLink(nodeIndex, objectPin);

        FluxValueRef object;
        if (pObjectLink) {
            object = EvaluateOutput(context, pObjectLink->GetSourceNode(), pObjectLink->GetSourcePin());
            if (context.hasErrors) {
                return false;
            }
        }

        auto&& pMethod = node.GetCallable().FindMethodMeta();
        const uint32_t argumentCount = GetCallArgumentCount(nodeIndex, firstArgumentPin, pMethod);

        Vector<FluxValueRef> arguments;
        Vector<uint32_t> outputPins;
        arguments.reserve(argumentCount);
        outputPins.reserve(argumentCount);

        for (uint32_t param = 0; param < argumentCount; ++param) {
            const uint32_t pin = firstArgumentPin + param;
            const uint32_t outputPin = GetFluxOutArgumentPin(node.GetType(), pMethod, param);

            if (auto&& pArgumentLink = m_graph.FindInputLink(nodeIndex, pin)) {
                arguments.emplace_back(EvaluateOutput(context, pArgumentLink->GetSourceNode(), pArgumentLink->GetSourcePin()));
                if (context.hasErrors) {
                    return false;
                }
            }
            else if (outputPin != FluxInvalidPin) {
                /// значение по умолчанию для неподключенного выходного аргумента подготовлено
                /// проходом CollectConstants
                auto&& pConstantIt = context.constantIndices.find(MakeFluxValueKey(nodeIndex, pin));
                if (pConstantIt == context.constantIndices.end()) {
                    SR_ERROR("FluxGraphCompiler::CompileCall() : default value of argument pin {} of node {} is missing!", pin, nodeIndex);
                    context.hasErrors = true;
                    return false;
                }
                FluxValueRef argument;
                argument.sourceNode = nodeIndex;
                argument.sourcePin = pin;
                argument.operand = static_cast<FluxRegisterId>(pConstantIt->second);
                argument.loopDepth = context.loopDepth;
                arguments.emplace_back(argument);
            }
            else {
                SR_ERROR("FluxGraphCompiler::CompileCall() : argument pin {} of node {} is not connected!", pin, nodeIndex);
                context.hasErrors = true;
                return false;
            }

            outputPins.emplace_back(outputPin);
        }

        /// выходные аргументы переносятся в собственные регистры узла до вызова: инструкция
        /// получит уже те операнды, которые метод имеет право изменить
        for (uint32_t param = 0; param < arguments.size(); ++param) {
            if (outputPins[param] != FluxInvalidPin) {
                arguments[param] = MaterializeOutArgument(context, nodeIndex, outputPins[param], arguments[param]);
            }
        }

        {
            auto&& instruction = EmitInstruction(context, FluxOpcode::Call, nodeIndex);
            instruction.callable = node.GetCallable();
            instruction.operands.reserve(arguments.size() + (pObjectLink ? 1 : 0));
            if (pObjectLink) {
                instruction.operands.emplace_back(object.operand);
            }
            for (auto&& argument : arguments) {
                instruction.operands.emplace_back(argument.operand);
            }
        }

        if (pObjectLink) {
            ReleaseValue(context, object);
        }
        /// у выходного аргумента освобождается его собственное владеющее использование,
        /// у обычного - использование, созданное связью
        for (auto&& argument : arguments) {
            ReleaseValue(context, argument);
        }

        return true;
    }

    FluxValueRef FluxGraphCompiler::EvaluateOutput(FluxGraphCompileContext& context, const uint32_t nodeIndex, const uint32_t pinIndex) const {
        const uint64_t key = MakeFluxValueKey(nodeIndex, pinIndex);

        /// значение, вычисленное ранее, переиспользуется - общие подграфы вычисляются один раз
        if (auto&& pIt = context.materialized.find(key); pIt != context.materialized.end()) {
            return pIt->second;
        }

        auto&& node = m_graph.GetNodes()[nodeIndex];

        FluxValueRef result;
        result.sourceNode = nodeIndex;
        result.sourcePin = pinIndex;
        result.loopDepth = context.loopDepth;

        switch (node.GetType()) {
            case FluxGraphNodeType::Constant: {
                /// константы адресуются напрямую и регистр не занимают
                auto&& pIt = context.constantIndices.find(key);
                if (pIt == context.constantIndices.end()) {
                    SR_ERROR("FluxGraphCompiler::EvaluateOutput() : constant of node {} is missing!", nodeIndex);
                    context.hasErrors = true;
                    return {};
                }
                result.operand = static_cast<FluxRegisterId>(pIt->second);
                return result;
            }
            case FluxGraphNodeType::ReadVariable: {
                /// переменные адресуются напрямую в хранилище и регистр не занимают
                auto&& pIt = context.storageIndices.find(node.GetName());
                if (pIt == context.storageIndices.end()) {
                    SR_ERROR("FluxGraphCompiler::EvaluateOutput() : unknown variable \"{}\"!", node.GetName());
                    context.hasErrors = true;
                    return {};
                }
                result.operand = static_cast<FluxRegisterId>(context.program->constants.size() + pIt->second);
                return result;
            }
            case FluxGraphNodeType::Evaluate: {
                if (context.evaluationStack.find(key) != context.evaluationStack.end()) {
                    SR_ERROR("FluxGraphCompiler::EvaluateOutput() : node {} is a part of a cyclic dependency!", nodeIndex);
                    context.hasErrors = true;
                    return {};
                }
                context.evaluationStack.emplace_back(key);
                const bool compiled = CompileCall(context, nodeIndex, GetCallObjectPin(FluxGraphNodeType::Evaluate));
                context.evaluationStack.pop_back();
                if (!compiled) {
                    return {};
                }

                /// вызов компилируется один раз, поэтому результат забирается из нулевого регистра
                /// сразу - даже если запрошен был выходной аргумент. Иначе чтение результата после
                /// чтения аргумента скомпилировало бы вызов повторно
                const uint64_t resultKey = MakeFluxValueKey(nodeIndex, 0);
                const uint32_t resultUseCount = GetUseCount(context, resultKey);

                if (HasFluxResultPin(FluxGraphNodeType::Evaluate, node.GetCallable().FindMethodMeta()) &&
                    !context.materialized.contains(resultKey) &&
                    (key == resultKey || resultUseCount > 0)
                ) {
                    FluxValueRef resultValue;
                    resultValue.sourceNode = nodeIndex;
                    resultValue.sourcePin = 0;
                    resultValue.registerIndex = context.AllocateRegister();
                    resultValue.operand = context.ToOperand(resultValue.registerIndex);
                    resultValue.isRegister = true;
                    resultValue.loopDepth = context.loopDepth;

                    /// результат вызова лежит в нулевом регистре и будет затёрт следующим вызовом
                    EmitBinary(context, FluxOpcode::Move, context.ToOperand(0), resultValue.operand, nodeIndex);

                    context.materialized.emplace(resultKey, resultValue);
                    context.pendingUses.emplace(resultKey, SR_MAX(resultUseCount, 1u));
                }

                /// выходной аргумент материализован самим вызовом
                if (auto&& pIt = context.materialized.find(key); pIt != context.materialized.end()) {
                    return pIt->second;
                }

                SR_ERROR("FluxGraphCompiler::EvaluateOutput() : output pin {} of node {} is not produced by the call!", pinIndex, nodeIndex);
                context.hasErrors = true;
                return {};
            }
            default:
                SR_ERROR("FluxGraphCompiler::EvaluateOutput() : value of node {} is used before it is produced!", nodeIndex);
                context.hasErrors = true;
                return {};
        }
    }

    FluxValueRef FluxGraphCompiler::EvaluateCondition(FluxGraphCompileContext& context, const uint32_t nodeIndex, const uint32_t pinIndex) const {
        auto&& pLink = m_graph.FindInputLink(nodeIndex, pinIndex);
        if (!pLink) {
            SR_ERROR("FluxGraphCompiler::EvaluateCondition() : condition pin of node {} is not connected!", nodeIndex);
            context.hasErrors = true;
            return {};
        }

        const uint32_t sourceNode = pLink->GetSourceNode();
        const uint32_t sourcePin = pLink->GetSourcePin();
        const uint64_t key = MakeFluxValueKey(sourceNode, sourcePin);

        /// условие читается средой исполнения из нулевого регистра. Если оно вычисляется чистым
        /// вызовом с единственным потребителем, то результат уже находится там - регистр не нужен.
        /// Выходной аргумент такого вызова лежит в собственном регистре, поэтому для него путь
        /// не годится
        if (m_graph.GetNodes()[sourceNode].GetType() == FluxGraphNodeType::Evaluate &&
            sourcePin == 0 &&
            HasFluxResultPin(FluxGraphNodeType::Evaluate, m_graph.GetNodes()[sourceNode].GetCallable().FindMethodMeta()) &&
            !context.materialized.contains(key) &&
            GetUseCount(context, key) <= 1
        ) {
            context.evaluationStack.emplace_back(key);
            const bool compiled = CompileCall(context, sourceNode, GetCallObjectPin(FluxGraphNodeType::Evaluate));
            context.evaluationStack.pop_back();
            if (!compiled) {
                return {};
            }
            FluxValueRef result;
            result.sourceNode = sourceNode;
            result.sourcePin = sourcePin;
            result.operand = context.ToOperand(0);
            result.loopDepth = context.loopDepth;
            return result;
        }

        const FluxValueRef condition = EvaluateOutput(context, sourceNode, sourcePin);
        if (context.hasErrors) {
            return {};
        }

        if (condition.operand != context.ToOperand(0)) {
            EmitBinary(context, FluxOpcode::Copy, condition.operand, context.ToOperand(0), nodeIndex);
        }

        return condition;
    }

    void FluxGraphCompiler::ReleaseValue(FluxGraphCompileContext& context, const FluxValueRef& value) const {
        if (!value.IsValid() || !value.isRegister) {
            return;
        }

        /// значение, вычисленное снаружи цикла, не может быть освобождено внутри тела: тело
        /// исполняется многократно, а значение больше не пересчитывается
        if (value.loopDepth < context.loopDepth) {
            context.deferredReleases.emplace_back(value);
            return;
        }

        const uint64_t key = MakeFluxValueKey(value.sourceNode, value.sourcePin);

        auto&& pIt = context.pendingUses.find(key);
        if (pIt == context.pendingUses.end()) {
            return;
        }

        if (pIt->second > 0) {
            --pIt->second;
        }

        if (pIt->second == 0) {
            context.FreeRegister(value.registerIndex);
            context.pendingUses.erase(key);
            context.materialized.erase(key);
        }
    }

    void FluxGraphCompiler::PushLoopScope(FluxGraphCompileContext& context) const {
        context.loopScopeStarts.emplace_back(static_cast<uint32_t>(context.deferredReleases.size()));
        ++context.loopDepth;
    }

    void FluxGraphCompiler::PopLoopScope(FluxGraphCompileContext& context) const {
        if (context.loopScopeStarts.empty()) {
            SRHalt("FluxGraphCompiler::PopLoopScope() : no loop scope to pop!");
            return;
        }

        const uint32_t start = context.loopScopeStarts.back();
        context.loopScopeStarts.pop_back();
        --context.loopDepth;

        Vector<FluxValueRef> pending;
        pending.reserve(context.deferredReleases.size() - start);
        for (uint32_t i = start; i < context.deferredReleases.size(); ++i) {
            pending.emplace_back(context.deferredReleases[i]);
        }
        context.deferredReleases.resize(start);

        /// накопленные освобождения применяются на уровне, где значения были материализованы.
        /// Если это всё ещё внутренний цикл, то они будут отложены снова
        for (auto&& value : pending) {
            ReleaseValue(context, value);
        }
    }

    /// ================================================== Генерация ===================================================

    FluxInstruction& FluxGraphCompiler::EmitInstruction(FluxGraphCompileContext& context, const FluxOpcode opcode, const uint32_t debugId) const {
        auto&& instruction = context.program->instructions.emplace_back();
        instruction.opcode = opcode;
        instruction.debugId = debugId;
        instruction.operands = Vector<FluxRegisterId>(context.program->allocator.Get());
        return instruction;
    }

    void FluxGraphCompiler::EmitBinary(FluxGraphCompileContext& context, const FluxOpcode opcode, const FluxRegisterId source, const FluxRegisterId destination, const uint32_t debugId) const {
        auto&& instruction = EmitInstruction(context, opcode, debugId);
        instruction.operands.reserve(2);
        instruction.operands.emplace_back(source);
        instruction.operands.emplace_back(destination);
    }

    void FluxGraphCompiler::EmitJump(FluxGraphCompileContext& context, const FluxOpcode opcode, const uint32_t labelIndex, const uint32_t debugId) const {
        auto&& instruction = EmitInstruction(context, opcode, debugId);
        instruction.operands.reserve(1);
        instruction.operands.emplace_back(static_cast<FluxRegisterId>(labelIndex));
    }

    uint32_t FluxGraphCompiler::CreateLabel(FluxGraphCompileContext& context, const StringView prefix) const {
        auto&& program = *context.program;

        FluxLabel& label = program.labels.emplace_back();
        label.name = String(program.allocator.Get());
        FormatTo(label.name, "{}_{}", prefix, context.labelCounter++);
        /// точка перехода станет известна позже - метка привязывается через BindLabel
        label.instructionPointer = 0;

        return static_cast<uint32_t>(program.labels.size() - 1);
    }

    void FluxGraphCompiler::BindLabel(FluxGraphCompileContext& context, const uint32_t labelIndex) const {
        auto&& program = *context.program;
        if (labelIndex >= program.labels.size()) {
            SRHalt("FluxGraphCompiler::BindLabel() : invalid label index!");
            return;
        }
        program.labels[labelIndex].instructionPointer = static_cast<uint32_t>(program.instructions.size());
    }

    void FluxGraphCompiler::BindLabelFolded(FluxGraphCompileContext& context, const uint32_t labelIndex) const {
        auto&& instructions = context.program->instructions;

        /// переход на метку, которая связывается прямо здесь, ничего не делает: код шага просто
        /// продолжается кодом следующего шага. Остальные переходы на эту метку не страдают -
        /// она встанет ровно туда, куда вёл убранный переход
        if (!instructions.empty()) {
            auto&& last = instructions.back();
            if (last.opcode == FluxOpcode::Jump && !last.operands.empty() && last.operands[0] == labelIndex) {
                instructions.pop_back();
            }
        }

        BindLabel(context, labelIndex);
    }

    /// ==================================================== Связи =====================================================

    uint32_t FluxGraphCompiler::GetFlowTarget(const uint32_t nodeIndex, const uint32_t pinIndex) const {
        auto&& pLink = m_graph.FindOutputLink(nodeIndex, pinIndex);
        return pLink ? pLink->GetTargetNode() : FluxInvalidNode;
    }

    uint32_t FluxGraphCompiler::GetUseCount(const FluxGraphCompileContext& context, const uint64_t key) {
        auto&& pIt = context.dataUseCount.find(key);
        return pIt == context.dataUseCount.end() ? 0 : pIt->second;
    }

    uint32_t FluxGraphCompiler::GetFlowInputCount(const FluxGraphCompileContext& context, const uint32_t nodeIndex) {
        auto&& pIt = context.flowInputCount.find(nodeIndex);
        return pIt == context.flowInputCount.end() ? 0 : pIt->second;
    }

    Optional<FluxProgram> FluxGraphCompiler::Compile() const {
        SR_TRACY_ZONE;

        FluxProgram program;
        program.allocator = (IAllocator*)(new MonotonicAllocator(64_KB));

        program.constants = Vector<FluxVariable>(program.allocator.Get());
        program.storage   = Vector<FluxVariable>(program.allocator.Get());
        program.instructions = Vector<FluxInstruction>(program.allocator.Get());
        program.labels = Vector<FluxLabel>(program.allocator.Get());

        program.constants.reserve(16);
        program.storage.reserve(16);
        program.instructions.reserve(m_graph.GetNodes().size() * 4);
        program.labels.reserve(16);

        FluxGraphCompileContext context;
        context.program = &program;

        /// константы и хранилище должны быть собраны до генерации инструкций: идентификатор операнда
        /// является смещением в общем адресном пространстве [constants][storage][registers]
        m_graph.CollectStorage(context);
        m_graph.CollectConstants(context);
        m_graph.CollectUsages(context);

        context.registerBase = static_cast<uint32_t>(program.constants.size() + program.storage.size());

        for (uint32_t i = 0; i < m_graph.GetNodes().size() && !context.hasErrors; ++i) {
            if (m_graph.GetNodes()[i].GetType() != FluxGraphNodeType::Event) {
                continue;
            }
            context.ResetExecutionState();
            context.nodeIndex = i;
            CompileEventNode(context);
        }

        if (context.hasErrors) {
            SR_ERROR("FluxGraph::Compile() : failed to compile graph!");
            return Optional<FluxProgram>();
        }

        program.requiredRegisters = context.requiredRegisters;

        OptimizeProgram(program);

        return program;
    }
}