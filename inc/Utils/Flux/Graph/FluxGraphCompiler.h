//
// Created by Monika on 03.09.2026.
//

#ifndef SR_ENGINE_FLUX_GRAPH_COMPILER_H
#define SR_ENGINE_FLUX_GRAPH_COMPILER_H

#include <Utils/Flux/IR/FluxInstruction.h>

namespace SR_FLUX_NS {
    struct FluxGraphCompileContext;
    struct FluxValueRef;
    class FluxGraph;
    struct FluxProgram;

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
    class FluxGraphCompiler {
    public:
        FluxGraphCompiler(const FluxGraph& graph);

    public:
        Optional<FluxProgram> Compile() const;

    private:
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

    private:
        const FluxGraph& m_graph;

        /// объект, предоставляющий арифметику для узла For. Разрешается средой исполнения по имени,
        /// поэтому может быть переопределён без перекомпиляции
        StringAtom m_arithmeticObject;
        StringAtom m_addFunction;
        StringAtom m_lessFunction;

    };
}

#endif //SR_ENGINE_FLUX_GRAPH_COMPILER_H
