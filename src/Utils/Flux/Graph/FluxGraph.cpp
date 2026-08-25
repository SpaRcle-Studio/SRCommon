//
// Created by Monika on 18.08.2026.
//

#include <Utils/Flux/Graph/FluxGraph.h>
#include <Utils/Flux/Graph/FluxGraphCompileContext.h>
#include <Utils/Flux/IR/FluxProgram.h>
#include <Utils/Memory/Allocator.h>
#include <Utils/Memory/MemoryLiterals.h>
#include <Utils/Reflection/TypeInfoSerialization.h>
#include <Utils/Serialization/JsonSerialization.h>

#include <Codegen/FluxGraph.generated.hpp>

namespace SR_FLUX_NS {
    namespace {
        /// Первый пин, с которого начинаются аргументы вызова.
        /// У Invoke нулевой пин занят потоком исполнения, поэтому объект сдвинут на единицу
        SR_NODISCARD uint32_t GetCallObjectPin(const FluxGraphNodeType type) {
            return type == FluxGraphNodeType::Invoke ? 1 : 0;
        }

        /// Является ли выходной пин узла пином данных, а не потока исполнения
        SR_NODISCARD bool IsDataOutputPin(const FluxGraphNodeType type, const uint32_t pinIndex) {
            switch (type) {
                case FluxGraphNodeType::Event:
                    return pinIndex >= 1;
                case FluxGraphNodeType::Invoke:
                    return pinIndex == 1;
                case FluxGraphNodeType::For:
                case FluxGraphNodeType::Cast:
                    return pinIndex == 2;
                case FluxGraphNodeType::Evaluate:
                case FluxGraphNodeType::Constant:
                case FluxGraphNodeType::ReadVariable:
                    return pinIndex == 0;
                default:
                    return false;
            }
        }

    }

    /// ================================================== FluxGraph ===================================================

    FluxGraph::FluxGraph()
        : Serializable()
    {
        m_allocator = (IAllocator*)(new UnSynchronizedPoolAllocator());
        m_nodes = Vector<FluxGraphNode>(m_allocator.Get());
        m_links = Vector<FluxGraphLink>(m_allocator.Get());
        m_variables = Map<StringAtom, Reflection::Value>(m_allocator.Get());

        m_arithmeticObject = StringAtom("Math");
        m_addFunction = StringAtom("Add");
        m_lessFunction = StringAtom("Less");
    }

    FluxGraph::~FluxGraph() {
        std::exchange(m_nodes, {});
        std::exchange(m_links, {});
        std::exchange(m_variables, {});
    }

    FluxGraph::FluxGraph(const FluxGraph& other)
        : Serializable(other)
        , m_allocator(new UnSynchronizedPoolAllocator())
        , m_arithmeticObject(other.m_arithmeticObject)
        , m_addFunction(other.m_addFunction)
        , m_lessFunction(other.m_lessFunction)
    {
        m_nodes = Vector<FluxGraphNode>(m_allocator.Get(), other.m_nodes.begin(), other.m_nodes.end());
        m_links = Vector<FluxGraphLink>(m_allocator.Get(), other.m_links.begin(), other.m_links.end());
        m_variables = Map<StringAtom, Reflection::Value>(m_allocator.Get(), other.m_variables);
    }

    FluxGraph& FluxGraph::operator=(const FluxGraph& other) {
        if (this != &other) {
            auto&& pNewAllocator = (IAllocator*)(new UnSynchronizedPoolAllocator());
            m_nodes = Vector<FluxGraphNode>(pNewAllocator, other.m_nodes.begin(), other.m_nodes.end());
            m_links = Vector<FluxGraphLink>(pNewAllocator, other.m_links.begin(), other.m_links.end());
            m_variables = Map<StringAtom, Reflection::Value>(pNewAllocator, other.m_variables);
            m_arithmeticObject = other.m_arithmeticObject;
            m_addFunction = other.m_addFunction;
            m_lessFunction = other.m_lessFunction;
            m_allocator = pNewAllocator;
        }
        return *this;
    }

    FluxProgram FluxGraph::Compile() const {
        SR_TRACY_ZONE;

        FluxProgram program;
        program.allocator = (IAllocator*)(new MonotonicAllocator(64_KB));

        program.constants = Vector<FluxVariable>(program.allocator.Get());
        program.storage   = Vector<FluxVariable>(program.allocator.Get());
        program.instructions = Vector<FluxInstruction>(program.allocator.Get());
        program.labels = Vector<FluxLabel>(program.allocator.Get());

        program.constants.reserve(16);
        program.storage.reserve(16);
        program.instructions.reserve(m_nodes.size() * 4);
        program.labels.reserve(16);

        FluxGraphCompileContext context;
        context.program = &program;

        /// константы и хранилище должны быть собраны до генерации инструкций: идентификатор операнда
        /// является смещением в общем адресном пространстве [constants][storage][registers]
        CollectStorage(context);
        CollectConstants(context);
        CollectUsages(context);

        context.registerBase = static_cast<uint32_t>(program.constants.size() + program.storage.size());

        for (uint32_t i = 0; i < m_nodes.size() && !context.hasErrors; ++i) {
            if (m_nodes[i].GetType() != FluxGraphNodeType::Event) {
                continue;
            }
            context.ResetExecutionState();
            context.nodeIndex = i;
            CompileEventNode(context);
        }

        if (context.hasErrors) {
            SR_ERROR("FluxGraph::Compile() : failed to compile graph!");
            program.instructions.clear();
            program.labels.clear();
            program.requiredRegisters = 0;
            return program;
        }

        program.requiredRegisters = context.requiredRegisters;

        return program;
    }

    /// ============================================== Подготовительные проходы =========================================

    void FluxGraph::CollectStorage(FluxGraphCompileContext& context) const {
        for (auto&& [name, value] : m_variables) {
            const uint32_t index = AddVariable(context, value, true);
            if (index == SR_UINT32_MAX) {
                SR_ERROR("FluxGraph::CollectStorage() : failed to serialize variable \"{}\"!", name);
                context.hasErrors = true;
                return;
            }
            context.storageIndices.emplace(name, index);
        }
    }

    void FluxGraph::CollectConstants(FluxGraphCompileContext& context) const {
        for (uint32_t i = 0; i < m_nodes.size(); ++i) {
            auto&& node = m_nodes[i];

            if (node.GetType() == FluxGraphNodeType::Constant) {
                const uint32_t index = AddVariable(context, node.GetConstant(), false);
                if (index == SR_UINT32_MAX) {
                    SR_ERROR("FluxGraph::CollectConstants() : failed to serialize constant of node {}!", i);
                    context.hasErrors = true;
                    return;
                }
                context.constantIndices.emplace(MakeFluxValueKey(i, 0), index);
                continue;
            }

            /// шаг цикла по умолчанию, если пин шага не подключен
            if (node.GetType() == FluxGraphNodeType::For && !FindInputLink(i, 3)) {
                const uint32_t index = AddVariable(context, Reflection::Value::Create<int32_t>(1), false);
                if (index == SR_UINT32_MAX) {
                    SR_ERROR("FluxGraph::CollectConstants() : failed to create default step of node {}!", i);
                    context.hasErrors = true;
                    return;
                }
                context.constantIndices.emplace(MakeFluxValueKey(i, 3), index);
            }
        }
    }

    void FluxGraph::CollectUsages(FluxGraphCompileContext& context) const {
        for (auto&& link : m_links) {
            const uint32_t sourceNode = link.GetSourceNode();
            const uint32_t targetNode = link.GetTargetNode();

            if (sourceNode >= m_nodes.size() || targetNode >= m_nodes.size()) {
                SR_ERROR("FluxGraph::CollectUsages() : link references an unknown node!");
                context.hasErrors = true;
                return;
            }

            if (IsDataOutputPin(m_nodes[sourceNode].GetType(), link.GetSourcePin())) {
                ++context.dataUseCount[MakeFluxValueKey(sourceNode, link.GetSourcePin())];
            }
            else {
                ++context.flowInputCount[targetNode];
            }
        }
    }

    /// =================================================== Поток ======================================================

    void FluxGraph::CompileEventNode(FluxGraphCompileContext& context) const {
        auto&& program = *context.program;
        const uint32_t nodeIndex = context.nodeIndex;
        auto&& node = m_nodes[nodeIndex];

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
        const uint32_t argumentCount = GetMaxOutputPin(nodeIndex);

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

    void FluxGraph::CompileFlow(FluxGraphCompileContext& context, uint32_t nodeIndex, const uint32_t terminatorLabel) const {
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

    uint32_t FluxGraph::CompileNode(FluxGraphCompileContext& context) const {
        const uint32_t nodeIndex = context.nodeIndex;
        auto&& node = m_nodes[nodeIndex];

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
            case FluxGraphNodeType::Evaluate:
            case FluxGraphNodeType::Constant:
            case FluxGraphNodeType::ReadVariable:
                SR_ERROR("FluxGraph::CompileNode() : pure node {} cannot be a part of the execution flow!", nodeIndex);
                context.hasErrors = true;
                return FluxInvalidNode;
            default:
                SRHalt("FluxGraph::CompileNode() : unhandled node type!");
                context.hasErrors = true;
                return FluxInvalidNode;
        }
    }

    uint32_t FluxGraph::CompileInvokeNode(FluxGraphCompileContext& context, const uint32_t nodeIndex) const {
        if (!CompileCall(context, nodeIndex, GetCallObjectPin(FluxGraphNodeType::Invoke))) {
            return FluxInvalidNode;
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

    uint32_t FluxGraph::CompileWriteVariableNode(FluxGraphCompileContext& context, const uint32_t nodeIndex) const {
        auto&& node = m_nodes[nodeIndex];

        auto&& pStorageIt = context.storageIndices.find(node.GetName());
        if (pStorageIt == context.storageIndices.end()) {
            SR_ERROR("FluxGraph::CompileWriteVariableNode() : unknown variable \"{}\"!", node.GetName());
            context.hasErrors = true;
            return FluxInvalidNode;
        }

        auto&& pValueLink = FindInputLink(nodeIndex, 1);
        if (!pValueLink) {
            SR_ERROR("FluxGraph::CompileWriteVariableNode() : value pin of node {} is not connected!", nodeIndex);
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

    uint32_t FluxGraph::CompileBranchNode(FluxGraphCompileContext& context, const uint32_t nodeIndex) const {
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

    uint32_t FluxGraph::CompileWhileNode(FluxGraphCompileContext& context, const uint32_t nodeIndex) const {
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

    uint32_t FluxGraph::CompileForNode(FluxGraphCompileContext& context, const uint32_t nodeIndex) const {
        auto&& pFromLink = FindInputLink(nodeIndex, 1);
        auto&& pToLink = FindInputLink(nodeIndex, 2);

        if (!pFromLink || !pToLink) {
            SR_ERROR("FluxGraph::CompileForNode() : range pins of node {} are not connected!", nodeIndex);
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
        if (auto&& pStepLink = FindInputLink(nodeIndex, 3)) {
            step = EvaluateOutput(context, pStepLink->GetSourceNode(), pStepLink->GetSourcePin());
            if (context.hasErrors) {
                return FluxInvalidNode;
            }
        }
        else {
            auto&& pConstantIt = context.constantIndices.find(MakeFluxValueKey(nodeIndex, 3));
            if (pConstantIt == context.constantIndices.end()) {
                SR_ERROR("FluxGraph::CompileForNode() : default step of node {} is missing!", nodeIndex);
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

    uint32_t FluxGraph::CompileCastNode(FluxGraphCompileContext& context, const uint32_t nodeIndex) const {
        auto&& node = m_nodes[nodeIndex];

        if (node.GetName().empty()) {
            SR_ERROR("FluxGraph::CompileCastNode() : target type of node {} is not specified!", nodeIndex);
            context.hasErrors = true;
            return FluxInvalidNode;
        }

        auto&& pObjectLink = FindInputLink(nodeIndex, 1);
        if (!pObjectLink) {
            SR_ERROR("FluxGraph::CompileCastNode() : object pin of node {} is not connected!", nodeIndex);
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

    /// =================================================== Значения ===================================================

    bool FluxGraph::CompileCall(FluxGraphCompileContext& context, const uint32_t nodeIndex, const uint32_t objectPin) const {
        auto&& node = m_nodes[nodeIndex];

        /// если пин объекта не подключен, то вызов считается обращением к синглтону и объект
        /// не передаётся в инструкцию - среда исполнения разрешит его по имени
        auto&& pObjectLink = FindInputLink(nodeIndex, objectPin);

        FluxValueRef object;
        if (pObjectLink) {
            object = EvaluateOutput(context, pObjectLink->GetSourceNode(), pObjectLink->GetSourcePin());
            if (context.hasErrors) {
                return false;
            }
        }

        Vector<FluxValueRef> arguments;
        const uint32_t maxPin = GetMaxInputPin(nodeIndex);
        arguments.reserve(maxPin);

        for (uint32_t pin = objectPin + 1; pin <= maxPin; ++pin) {
            auto&& pArgumentLink = FindInputLink(nodeIndex, pin);
            if (!pArgumentLink) {
                SR_ERROR("FluxGraph::CompileCall() : argument pin {} of node {} is not connected!", pin, nodeIndex);
                context.hasErrors = true;
                return false;
            }
            arguments.emplace_back(EvaluateOutput(context, pArgumentLink->GetSourceNode(), pArgumentLink->GetSourcePin()));
            if (context.hasErrors) {
                return false;
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
        for (auto&& argument : arguments) {
            ReleaseValue(context, argument);
        }

        return true;
    }

    FluxValueRef FluxGraph::EvaluateOutput(FluxGraphCompileContext& context, const uint32_t nodeIndex, const uint32_t pinIndex) const {
        const uint64_t key = MakeFluxValueKey(nodeIndex, pinIndex);

        /// значение, вычисленное ранее, переиспользуется - общие подграфы вычисляются один раз
        if (auto&& pIt = context.materialized.find(key); pIt != context.materialized.end()) {
            return pIt->second;
        }

        auto&& node = m_nodes[nodeIndex];

        FluxValueRef result;
        result.sourceNode = nodeIndex;
        result.sourcePin = pinIndex;
        result.loopDepth = context.loopDepth;

        switch (node.GetType()) {
            case FluxGraphNodeType::Constant: {
                /// константы адресуются напрямую и регистр не занимают
                auto&& pIt = context.constantIndices.find(key);
                if (pIt == context.constantIndices.end()) {
                    SR_ERROR("FluxGraph::EvaluateOutput() : constant of node {} is missing!", nodeIndex);
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
                    SR_ERROR("FluxGraph::EvaluateOutput() : unknown variable \"{}\"!", node.GetName());
                    context.hasErrors = true;
                    return {};
                }
                result.operand = static_cast<FluxRegisterId>(context.program->constants.size() + pIt->second);
                return result;
            }
            case FluxGraphNodeType::Evaluate: {
                if (context.evaluationStack.find(key) != context.evaluationStack.end()) {
                    SR_ERROR("FluxGraph::EvaluateOutput() : node {} is a part of a cyclic dependency!", nodeIndex);
                    context.hasErrors = true;
                    return {};
                }
                context.evaluationStack.emplace_back(key);
                const bool compiled = CompileCall(context, nodeIndex, GetCallObjectPin(FluxGraphNodeType::Evaluate));
                context.evaluationStack.pop_back();
                if (!compiled) {
                    return {};
                }
                result.registerIndex = context.AllocateRegister();
                result.operand = context.ToOperand(result.registerIndex);
                result.isRegister = true;
                /// результат вызова лежит в нулевом регистре и будет затёрт следующим вызовом
                EmitBinary(context, FluxOpcode::Move, context.ToOperand(0), result.operand, nodeIndex);
                break;
            }
            default:
                SR_ERROR("FluxGraph::EvaluateOutput() : value of node {} is used before it is produced!", nodeIndex);
                context.hasErrors = true;
                return {};
        }

        context.materialized.emplace(key, result);
        context.pendingUses.emplace(key, SR_MAX(GetUseCount(context, key), 1u));

        return result;
    }

    FluxValueRef FluxGraph::EvaluateCondition(FluxGraphCompileContext& context, const uint32_t nodeIndex, const uint32_t pinIndex) const {
        auto&& pLink = FindInputLink(nodeIndex, pinIndex);
        if (!pLink) {
            SR_ERROR("FluxGraph::EvaluateCondition() : condition pin of node {} is not connected!", nodeIndex);
            context.hasErrors = true;
            return {};
        }

        const uint32_t sourceNode = pLink->GetSourceNode();
        const uint32_t sourcePin = pLink->GetSourcePin();
        const uint64_t key = MakeFluxValueKey(sourceNode, sourcePin);

        /// условие читается средой исполнения из нулевого регистра. Если оно вычисляется чистым
        /// вызовом с единственным потребителем, то результат уже находится там - регистр не нужен
        if (m_nodes[sourceNode].GetType() == FluxGraphNodeType::Evaluate &&
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

    void FluxGraph::ReleaseValue(FluxGraphCompileContext& context, const FluxValueRef& value) const {
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

    void FluxGraph::PushLoopScope(FluxGraphCompileContext& context) const {
        context.loopScopeStarts.emplace_back(static_cast<uint32_t>(context.deferredReleases.size()));
        ++context.loopDepth;
    }

    void FluxGraph::PopLoopScope(FluxGraphCompileContext& context) const {
        if (context.loopScopeStarts.empty()) {
            SRHalt("FluxGraph::PopLoopScope() : no loop scope to pop!");
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

    /// ==================================================== Связи =====================================================

    const FluxGraphLink* FluxGraph::FindInputLink(const uint32_t nodeIndex, const uint32_t pinIndex) const {
        for (auto&& link : m_links) {
            if (link.GetTargetNode() == nodeIndex && link.GetTargetPin() == pinIndex) {
                return &link;
            }
        }
        return nullptr;
    }

    const FluxGraphLink* FluxGraph::FindOutputLink(const uint32_t nodeIndex, const uint32_t pinIndex) const {
        for (auto&& link : m_links) {
            if (link.GetSourceNode() == nodeIndex && link.GetSourcePin() == pinIndex) {
                return &link;
            }
        }
        return nullptr;
    }

    uint32_t FluxGraph::GetFlowTarget(const uint32_t nodeIndex, const uint32_t pinIndex) const {
        auto&& pLink = FindOutputLink(nodeIndex, pinIndex);
        return pLink ? pLink->GetTargetNode() : FluxInvalidNode;
    }

    uint32_t FluxGraph::GetMaxInputPin(const uint32_t nodeIndex) const {
        uint32_t maxPin = 0;
        for (auto&& link : m_links) {
            if (link.GetTargetNode() == nodeIndex) {
                maxPin = SR_MAX(maxPin, link.GetTargetPin());
            }
        }
        return maxPin;
    }

    uint32_t FluxGraph::GetMaxOutputPin(const uint32_t nodeIndex) const {
        uint32_t maxPin = 0;
        for (auto&& link : m_links) {
            if (link.GetSourceNode() == nodeIndex) {
                maxPin = SR_MAX(maxPin, link.GetSourcePin());
            }
        }
        return maxPin;
    }

    uint32_t FluxGraph::GetUseCount(const FluxGraphCompileContext& context, const uint64_t key) {
        auto&& pIt = context.dataUseCount.find(key);
        return pIt == context.dataUseCount.end() ? 0 : pIt->second;
    }

    uint32_t FluxGraph::GetFlowInputCount(const FluxGraphCompileContext& context, const uint32_t nodeIndex) {
        auto&& pIt = context.flowInputCount.find(nodeIndex);
        return pIt == context.flowInputCount.end() ? 0 : pIt->second;
    }

    /// ================================================== Генерация ===================================================

    FluxInstruction& FluxGraph::EmitInstruction(FluxGraphCompileContext& context, const FluxOpcode opcode, const uint32_t debugId) const {
        auto&& instruction = context.program->instructions.emplace_back();
        instruction.opcode = opcode;
        instruction.debugId = debugId;
        instruction.operands = Vector<FluxRegisterId>(context.program->allocator.Get());
        return instruction;
    }

    void FluxGraph::EmitBinary(FluxGraphCompileContext& context, const FluxOpcode opcode, const FluxRegisterId source, const FluxRegisterId destination, const uint32_t debugId) const {
        auto&& instruction = EmitInstruction(context, opcode, debugId);
        instruction.operands.reserve(2);
        instruction.operands.emplace_back(source);
        instruction.operands.emplace_back(destination);
    }

    void FluxGraph::EmitJump(FluxGraphCompileContext& context, const FluxOpcode opcode, const uint32_t labelIndex, const uint32_t debugId) const {
        auto&& instruction = EmitInstruction(context, opcode, debugId);
        instruction.operands.reserve(1);
        instruction.operands.emplace_back(static_cast<FluxRegisterId>(labelIndex));
    }

    uint32_t FluxGraph::CreateLabel(FluxGraphCompileContext& context, const StringView prefix) const {
        auto&& program = *context.program;

        FluxLabel& label = program.labels.emplace_back();
        label.name = String(program.allocator.Get());
        FormatTo(label.name, "{}_{}", prefix, context.labelCounter++);
        /// точка перехода станет известна позже - метка привязывается через BindLabel
        label.instructionPointer = 0;

        return static_cast<uint32_t>(program.labels.size() - 1);
    }

    void FluxGraph::BindLabel(FluxGraphCompileContext& context, const uint32_t labelIndex) const {
        auto&& program = *context.program;
        if (labelIndex >= program.labels.size()) {
            SRHalt("FluxGraph::BindLabel() : invalid label index!");
            return;
        }
        program.labels[labelIndex].instructionPointer = static_cast<uint32_t>(program.instructions.size());
    }

    uint32_t FluxGraph::AddVariable(FluxGraphCompileContext& context, const Reflection::Value& value, const bool isStorage) const {
        if (!value.IsValid()) {
            return SR_UINT32_MAX;
        }

        String type;
        Reflection::SaveTypeInfo(type, &value.GetTypeInfo());
        if (type.empty()) {
            return SR_UINT32_MAX;
        }

        JsonSerializer serializer;
        serializer.SetPrettyPrint(false);
        Reflection::SerializeValue(value, serializer);
        const String serialized = serializer.ToStringBase();

        auto&& program = *context.program;
        auto&& variables = isStorage ? program.storage : program.constants;

        /// одинаковые константы схлопываются - это уменьшает адресное пространство программы.
        /// Ячейки хранилища схлопывать нельзя, они адресуются по имени переменной
        if (!isStorage) {
            for (uint32_t i = 0; i < variables.size(); ++i) {
                if (variables[i].type == type && variables[i].value == serialized) {
                    return i;
                }
            }
        }

        FluxVariable& variable = variables.emplace_back();
        variable.type = String(program.allocator.Get());
        variable.value = String(program.allocator.Get());
        variable.type += type;
        variable.value += serialized;

        return static_cast<uint32_t>(variables.size() - 1);
    }

    uint32_t FluxGraph::AddNode(const FluxGraphNode& node) {
        m_nodes.emplace_back(node);
        return static_cast<uint32_t>(m_nodes.size() - 1);
    }

    void FluxGraph::AddLink(const FluxGraphLink& link) {
        /// дубликат связи удвоил бы число входящих потоков узла и создал бы метку слияния на ровном месте
        if (m_links.find_if([&link](const FluxGraphLink& other) { return other == link; }) != m_links.end()) {
            return;
        }
        m_links.emplace_back(link);
    }

    void FluxGraph::RemoveNode(const uint32_t nodeIndex) {
        if (nodeIndex >= m_nodes.size()) {
            SRHalt("FluxGraph::RemoveNode() : node index {} is out of range!", nodeIndex);
            return;
        }

        /// связи хранят индексы узлов, поэтому после удаления узла их нужно пересчитать
        for (auto pIt = m_links.begin(); pIt != m_links.end();) {
            if (pIt->GetSourceNode() == nodeIndex || pIt->GetTargetNode() == nodeIndex) {
                pIt = m_links.erase(pIt);
                continue;
            }

            if (pIt->GetSourceNode() > nodeIndex) {
                pIt->SetSourceNode(pIt->GetSourceNode() - 1);
            }
            if (pIt->GetTargetNode() > nodeIndex) {
                pIt->SetTargetNode(pIt->GetTargetNode() - 1);
            }

            ++pIt;
        }

        m_nodes.erase(m_nodes.begin() + nodeIndex);
    }

    void FluxGraph::RemoveLink(const uint32_t sourceNode, const uint32_t sourcePin, const uint32_t targetNode, const uint32_t targetPin) {
        std::erase_if(m_links, [&](const FluxGraphLink& link) {
            return link.GetSourceNode() == sourceNode && link.GetSourcePin() == sourcePin &&
                link.GetTargetNode() == targetNode && link.GetTargetPin() == targetPin;
        });
    }

    void FluxGraph::RemoveInputLink(const uint32_t nodeIndex, const uint32_t pinIndex) {
        std::erase_if(m_links, [&](const FluxGraphLink& link) {
            return link.GetTargetNode() == nodeIndex && link.GetTargetPin() == pinIndex;
        });
    }

    void FluxGraph::RemoveOutputLink(const uint32_t nodeIndex, const uint32_t pinIndex) {
        std::erase_if(m_links, [&](const FluxGraphLink& link) {
            return link.GetSourceNode() == nodeIndex && link.GetSourcePin() == pinIndex;
        });
    }

    FluxGraphNode* FluxGraph::GetNode(const uint32_t nodeIndex) {
        return nodeIndex < m_nodes.size() ? &m_nodes[nodeIndex] : nullptr;
    }

    const FluxGraphNode* FluxGraph::GetNode(const uint32_t nodeIndex) const {
        return nodeIndex < m_nodes.size() ? &m_nodes[nodeIndex] : nullptr;
    }

    void FluxGraphNode::SetCallable(const FluxCallable& callable) {
        m_callableObject = callable.object;
        m_callableFunction = callable.function;
    }

    FluxCallable FluxGraphNode::GetCallable() const {
        return { m_callableObject, m_callableFunction };
    }

    bool FluxGraphNode::operator==(const FluxGraphNode &other) const {
        return std::tie(m_type, m_name, m_position, m_constant, m_callableObject, m_callableFunction) ==
               std::tie(other.m_type, other.m_name, other.m_position, other.m_constant, other.m_callableObject, other.m_callableFunction);
    }

    bool FluxGraphLink::operator==(const FluxGraphLink &other) const {
        return std::tie(m_sourceNode, m_targetNode, m_sourcePin, m_targetPin) ==
               std::tie(other.m_sourceNode, other.m_targetNode, other.m_sourcePin, other.m_targetPin);
    }
}
