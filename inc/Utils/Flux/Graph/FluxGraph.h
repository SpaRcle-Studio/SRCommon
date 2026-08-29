//
// Created by Monika on 18.08.2026.
//

#ifndef SR_ENGINE_COMMON_FLUX_GRAPH_H
#define SR_ENGINE_COMMON_FLUX_GRAPH_H

#include <Utils/Reflection/Value.h>
#include <Utils/Reflection/Method.h>
#include <Utils/Flux/IR/FluxInstruction.h>
#include <Utils/Types/Optional.h>

namespace SR_FLUX_NS {
    struct FluxGraphCompileContext;
    struct FluxValueRef;
    struct FluxProgram;
    class FluxGraphNode;

    SR_MAYBE_UNUSED static constexpr uint32_t FluxInvalidPin = SR_UINT32_MAX;

    /// Ключ значения, которое производит выходной пин узла графа
    SR_MAYBE_UNUSED static constexpr uint64_t MakeFluxValueKey(uint32_t nodeIndex, uint32_t pinIndex) noexcept {
        return (static_cast<uint64_t>(nodeIndex) << 32u) | static_cast<uint64_t>(pinIndex);
    }

    class FluxGraphLink : public Serializable {
        SR_CLASS()
    public:
        SR_NODISCARD uint32_t GetSourceNode() const { return m_sourceNode; }
        SR_NODISCARD uint32_t GetTargetNode() const { return m_targetNode; }
        SR_NODISCARD uint32_t GetSourcePin() const { return m_sourcePin; }
        SR_NODISCARD uint32_t GetTargetPin() const { return m_targetPin; }
        SR_NODISCARD void* GetUserData() const { return m_userData; }

        void SetSourceNode(uint32_t sourceNode) { m_sourceNode = sourceNode; }
        void SetTargetNode(uint32_t targetNode) { m_targetNode = targetNode; }
        void SetSourcePin(uint32_t sourcePin) { m_sourcePin = sourcePin; }
        void SetTargetPin(uint32_t targetPin) { m_targetPin = targetPin; }
        void SetUserData(void* userData) { m_userData = userData; }

        SR_NODISCARD bool operator==(const FluxGraphLink& other) const;
        SR_NODISCARD auto operator!=(const FluxGraphLink& other) const { return !(*this == other); }

    private:
        /// @property
        uint32_t m_sourceNode = 0;
        /// @property
        uint32_t m_targetNode = 0;
        /// @property
        uint32_t m_sourcePin = 0;
        /// @property
        uint32_t m_targetPin = 0;

        void* m_userData = nullptr;
    };

    SR_ENUM_NS_CLASS_T(FluxGraphNodeType, uint8_t,
        Unknown,

        Event,

        Evaluate,
        Invoke,

        Constant,
        ReadVariable,
        WriteVariable,

        Branch,
        For,
        While,

        Cast,

        Sequence,
        ParallelSequence

        /// TODO:
        /// Synchronize,
    )

    /// Узел, у которого все выходные пины являются шагами потока исполнения
    SR_NODISCARD inline bool IsFluxSequenceNode(const FluxGraphNodeType type) noexcept {
        return type == FluxGraphNodeType::Sequence || type == FluxGraphNodeType::ParallelSequence;
    }

    /// Индекс первого выходного пина, отведённого под выходные аргументы вызова.
    /// FluxInvalidPin, если узел не является вызовом
    SR_NODISCARD inline uint32_t GetFluxOutArgumentBasePin(const FluxGraphNodeType type, const Reflection::Method* pMethod) noexcept {
        /// нулевой выходной пин у Invoke занят потоком исполнения, а у Evaluate - результатом.
        /// Пин результата существует только у методов, которые что-то возвращают. Сигнатура
        /// неизвестного метода считается возвращающей значение - так раскладка совпадает с той,
        /// что строил редактор до появления метаданных
        const bool hasResult = !pMethod || pMethod->HasReturn();
        switch (type) {
            case FluxGraphNodeType::Invoke:
                return hasResult ? 2 : 1;
            case FluxGraphNodeType::Evaluate:
                return hasResult ? 1 : 0;
            default:
                return FluxInvalidPin;
        }
    }

    /// Производит ли нулевой выходной пин узла Evaluate (или первый - узла Invoke) результат вызова
    SR_NODISCARD inline bool HasFluxResultPin(const FluxGraphNodeType type, const Reflection::Method* pMethod) noexcept {
        return (type == FluxGraphNodeType::Invoke || type == FluxGraphNodeType::Evaluate) && (!pMethod || pMethod->HasReturn());
    }

    /// Выходной пин, на котором узел публикует изменённое значение аргумента paramIndex.
    /// FluxInvalidPin, если аргумент не является выходным
    SR_NODISCARD inline uint32_t GetFluxOutArgumentPin(const FluxGraphNodeType type, const Reflection::Method* pMethod, const uint32_t paramIndex) noexcept {
        const uint32_t basePin = GetFluxOutArgumentBasePin(type, pMethod);
        if (!pMethod || basePin == FluxInvalidPin || !pMethod->IsOutputParam(paramIndex)) {
            return FluxInvalidPin;
        }
        uint32_t offset = 0;
        for (uint32_t i = 0; i < paramIndex; ++i) {
            offset += pMethod->IsOutputParam(i) ? 1 : 0;
        }
        return basePin + offset;
    }

    /// Слияние ветвей. Выходной flow-пин ведёт ровно в один узел, а входной flow-пин принимает
    /// сколько угодно связей. Узел с несколькими входящими потоками компилируется один раз и
    /// получает метку, остальные пути приходят в неё переходом - код после точки слияния не
    /// дублируется. Значение, вычисленное внутри одной ветви, до точки слияния не доживает:
    /// остальные пути его регистр не заполняли, поэтому после метки оно вычисляется заново
    /// (для чистых узлов) либо приводит к ошибке компиляции.
    ///
    /// Аргументы, принимаемые методом по неконстантной ссылке, считаются выходными: помимо
    /// входного пина узел получает для такого аргумента отдельный выходной пин, на котором
    /// публикуется значение, изменённое вызовом. Выходные пины аргументов идут подряд, в порядке
    /// следования параметров, сразу за пином результата (см. GetFluxOutArgumentBasePin).
    /// Значение аргумента всегда попадает в собственный регистр узла, поэтому источник вызовом
    /// не портится: константа копируется в регистр (константы неизменяемы), значение переменной
    /// тоже копируется - чтобы записать результат обратно, нужен узел WriteVariable. Копирование
    /// не выполняется лишь тогда, когда источником был временный регистр, у которого этот вызов
    /// был последним потребителем - такой регистр переиспользуется напрямую.
    /// Входной пин выходного аргумента разрешается не подключать: тогда метод получит значение
    /// по умолчанию для типа параметра.
    ///
    /// Sequence инструкцией не является - это удобство редактора. Шаги компилируются подряд,
    /// в порядке следования выходных пинов: код следующего шага просто идёт за кодом предыдущего,
    /// как если бы узлы были соединены в одну цепочку. Значение, вычисленное внутри шага, до
    /// следующего шага не доживает - шаг может завершиться несколькими путями, и не каждый из них
    /// заполнял бы регистр.
    ///
    /// ParallelSequence, в отличие от него, компилируется в инструкцию fork: каждый её операнд
    /// является меткой шага, с которой среда исполнения запускает новое исполнение с копией
    /// состояния (регистры, стек значений, стек вызовов) на момент выполнения инструкции - как
    /// fork(2) для процессов. Родитель продолжает свой путь дальше по потоку, а каждая ветвь
    /// живёт самостоятельно и завершается по достижении конца своей цепочки.
    ///
    /// Раскладка пинов узлов. Flow-пин всегда имеет индекс 0 (и на входе, и на выходе).
    /// Узлы Evaluate / Constant / ReadVariable являются чистыми - они не участвуют в потоке
    /// исполнения и вычисляются в точке использования.
    ///
    ///   Event         | out: 0 - flow, 1..N - аргументы события
    ///   Invoke        | in:  0 - flow, 1 - объект (у синглтона пин занят первым аргументом),
    ///                 |      далее - аргументы
    ///                 | out: 0 - flow, 1 - возвращаемое значение (если метод его имеет),
    ///                 |      далее - выходные аргументы
    ///   Evaluate      | in:  0 - объект (у синглтона пин занят первым аргументом),
    ///                 |      далее - аргументы
    ///                 | out: 0 - результат вызова (если метод его имеет), далее - выходные аргументы
    ///   Constant      | out: 0 - значение
    ///   ReadVariable  | out: 0 - значение
    ///   WriteVariable | in:  0 - flow, 1 - значение          | out: 0 - flow
    ///   Branch        | in:  0 - flow, 1 - условие           | out: 0 - flow (true), 1 - flow (false)
    ///   While         | in:  0 - flow, 1 - условие           | out: 0 - flow (тело), 1 - flow (после цикла)
    ///   For           | in:  0 - flow, 1 - начало, 2 - конец, 3 - шаг (опционально)
    ///                 | out: 0 - flow (тело), 1 - flow (после цикла), 2 - текущий индекс
    ///   Cast          | in:  0 - flow, 1 - объект
    ///                 | out: 0 - flow (успех), 1 - flow (cast failed), 2 - приведённый объект
    ///   Sequence      | in:  0 - flow                        | out: 0..N - flow (шаги)
    ///   ParallelSequence | in: 0 - flow                      | out: 0..N - flow (ветви)
    class FluxGraphNode : public Serializable {
        SR_CLASS()
    public:
        SR_NODISCARD FluxGraphNodeType GetType() const { return m_type; }
        SR_NODISCARD FluxCallable GetCallable() const;
        SR_NODISCARD StringAtom GetName() const { return m_name; }
        SR_NODISCARD const Reflection::Value& GetConstant() const { return m_constant; }
        SR_NODISCARD Reflection::Value& GetConstantMutable() { return m_constant; }
        SR_NODISCARD SR_MATH_NS::FVector2 GetPosition() const { return m_position; }
        SR_NODISCARD void* GetUserData() const { return m_userData; }

        void SetType(FluxGraphNodeType type) { m_type = type; }
        void SetCallable(const FluxCallable& callable);
        void SetName(const StringAtom& name) { m_name = name; }
        void SetConstant(const Reflection::Value& constant) { m_constant = constant; }
        void SetPosition(const SR_MATH_NS::FVector2& position) { m_position = position; }
        void SetUserData(void* userData) { m_userData = userData; }

        SR_NODISCARD bool operator==(const FluxGraphNode& other) const;

    private:
        /// @property
        FluxGraphNodeType m_type = FluxGraphNodeType::Unknown;
        /// @property @tooltip(for ReadVariable / WriteVariable / Event, target type for Cast)
        StringAtom m_name;
        /// @property
        SR_MATH_NS::FVector2 m_position;
        /// @property
        Reflection::Value m_constant;
        /// @property
        StringAtom m_callableObject;
        /// @property
        StringAtom m_callableFunction;

        void* m_userData = nullptr;

    };

    class FluxGraph : public Serializable {
        SR_CLASS()
    public:
        FluxGraph();
        ~FluxGraph() override;

        FluxGraph(const FluxGraph& other);
        FluxGraph& operator=(const FluxGraph& other);

    public:
        /// @return индекс добавленного узла
        uint32_t AddNode(const FluxGraphNode& node);
        void AddLink(const FluxGraphLink& link);

        /// удаляет узел вместе со связями, индексы оставшихся узлов сдвигаются
        void RemoveNode(uint32_t nodeIndex);
        void RemoveLink(uint32_t sourceNode, uint32_t sourcePin, uint32_t targetNode, uint32_t targetPin);
        void RemoveInputLink(uint32_t nodeIndex, uint32_t pinIndex);
        void RemoveOutputLink(uint32_t nodeIndex, uint32_t pinIndex);

        SR_NODISCARD Optional<FluxProgram> Compile() const;

        SR_NODISCARD Reflection::Value* FindVariable(StringAtom name);

        SR_NODISCARD uint32_t GetNodeCount() const { return static_cast<uint32_t>(m_nodes.size()); }
        SR_NODISCARD uint32_t GetNodeIndex(const FluxGraphNode* pNode) const;
        SR_NODISCARD FluxGraphNode* GetNode(uint32_t nodeIndex);
        SR_NODISCARD const FluxGraphNode* GetNode(uint32_t nodeIndex) const;

        SR_NODISCARD const Vector<FluxGraphNode>& GetNodes() const { return m_nodes; }
        SR_NODISCARD Vector<FluxGraphNode>& GetNodes() { return m_nodes; }
        SR_NODISCARD const Vector<FluxGraphLink>& GetLinks() const { return m_links; }
        SR_NODISCARD Vector<FluxGraphLink>& GetLinks() { return m_links; }
        SR_NODISCARD const Map<StringAtom, Reflection::Value>& GetVariables() const { return m_variables; }
        SR_NODISCARD Map<StringAtom, Reflection::Value>& GetVariables() { return m_variables; }

        SR_NODISCARD const FluxGraphLink* FindInputLink(uint32_t nodeIndex, uint32_t pinIndex) const;
        SR_NODISCARD const FluxGraphLink* FindOutputLink(uint32_t nodeIndex, uint32_t pinIndex) const;
        SR_NODISCARD uint32_t GetMaxInputPin(uint32_t nodeIndex) const;
        SR_NODISCARD uint32_t GetMaxOutputPin(uint32_t nodeIndex) const;

    private:
        /// подготовительные проходы, выполняются до генерации инструкций, так как адресное
        /// пространство операндов зависит от количества констант и ячеек хранилища
        void CollectStorage(FluxGraphCompileContext& context) const;
        void CollectConstants(FluxGraphCompileContext& context) const;
        void CollectUsages(FluxGraphCompileContext& context) const;

        void CompileEventNode(FluxGraphCompileContext& context) const;
        void CompileFlow(FluxGraphCompileContext& context, uint32_t nodeIndex, uint32_t terminatorLabel) const;
        SR_NODISCARD uint32_t CompileNode(FluxGraphCompileContext& context) const;

        SR_NODISCARD uint32_t CompileInvokeNode(FluxGraphCompileContext& context, uint32_t nodeIndex) const;
        SR_NODISCARD uint32_t CompileWriteVariableNode(FluxGraphCompileContext& context, uint32_t nodeIndex) const;
        SR_NODISCARD uint32_t CompileBranchNode(FluxGraphCompileContext& context, uint32_t nodeIndex) const;
        SR_NODISCARD uint32_t CompileWhileNode(FluxGraphCompileContext& context, uint32_t nodeIndex) const;
        SR_NODISCARD uint32_t CompileForNode(FluxGraphCompileContext& context, uint32_t nodeIndex) const;
        SR_NODISCARD uint32_t CompileCastNode(FluxGraphCompileContext& context, uint32_t nodeIndex) const;
        SR_NODISCARD uint32_t CompileSequenceNode(FluxGraphCompileContext& context, uint32_t nodeIndex) const;
        SR_NODISCARD uint32_t CompileParallelSequenceNode(FluxGraphCompileContext& context, uint32_t nodeIndex) const;
        /// выходные flow-пины узла, к которым подключены шаги, в порядке следования
        SR_NODISCARD Vector<uint32_t> CollectSequenceSteps(uint32_t nodeIndex) const;

        SR_NODISCARD bool CompileCall(FluxGraphCompileContext& context, uint32_t nodeIndex, uint32_t objectPin) const;
        /// материализует выходной аргумент в собственном регистре узла и публикует его на выходном пине
        SR_NODISCARD FluxValueRef MaterializeOutArgument(FluxGraphCompileContext& context, uint32_t nodeIndex, uint32_t outputPin, const FluxValueRef& source) const;
        SR_NODISCARD uint32_t GetCallArgumentCount(uint32_t nodeIndex, uint32_t firstArgumentPin, const Reflection::Method* pMethod) const;

        SR_NODISCARD FluxValueRef EvaluateOutput(FluxGraphCompileContext& context, uint32_t nodeIndex, uint32_t pinIndex) const;
        SR_NODISCARD FluxValueRef EvaluateCondition(FluxGraphCompileContext& context, uint32_t nodeIndex, uint32_t pinIndex) const;
        void ReleaseValue(FluxGraphCompileContext& context, const FluxValueRef& value) const;

        void PushLoopScope(FluxGraphCompileContext& context) const;
        void PopLoopScope(FluxGraphCompileContext& context) const;

        SR_NODISCARD uint32_t GetFlowTarget(uint32_t nodeIndex, uint32_t pinIndex) const;
        SR_NODISCARD static uint32_t GetUseCount(const FluxGraphCompileContext& context, uint64_t key);
        SR_NODISCARD static uint32_t GetFlowInputCount(const FluxGraphCompileContext& context, uint32_t nodeIndex);

        FluxInstruction& EmitInstruction(FluxGraphCompileContext& context, FluxOpcode opcode, uint32_t debugId) const;
        void EmitBinary(FluxGraphCompileContext& context, FluxOpcode opcode, FluxRegisterId source, FluxRegisterId destination, uint32_t debugId) const;
        void EmitJump(FluxGraphCompileContext& context, FluxOpcode opcode, uint32_t labelIndex, uint32_t debugId) const;
        SR_NODISCARD uint32_t CreateLabel(FluxGraphCompileContext& context, StringView prefix) const;
        void BindLabel(FluxGraphCompileContext& context, uint32_t labelIndex) const;
        /// связывает метку, убирая хвостовой переход на неё же - код просто продолжается дальше
        void BindLabelFolded(FluxGraphCompileContext& context, uint32_t labelIndex) const;

        SR_NODISCARD uint32_t AddVariable(FluxGraphCompileContext& context, const Reflection::Value& value, bool isStorage) const;

    private:
        /// @property
        Vector<FluxGraphNode> m_nodes;
        /// @property
        Vector<FluxGraphLink> m_links;
        /// @property
        Map<StringAtom, Reflection::Value> m_variables;

        /// объект, предоставляющий арифметику для узла For. Разрешается средой исполнения по имени,
        /// поэтому может быть переопределён без перекомпиляции
        StringAtom m_arithmeticObject;
        StringAtom m_addFunction;
        StringAtom m_lessFunction;

        RawPointerHolder<IAllocator> m_allocator;

    };
}

#endif //SR_ENGINE_COMMON_FLUX_GRAPH_H
