//
// Created by Monika on 11.09.2026.
//

#ifndef SR_ENGINE_BIND_H
#define SR_ENGINE_BIND_H

#include <Utils/stdInclude.h>

/// Аналог std::bind без включения <functional>.
/// Не выполняет ни одной аллокации в куче: связанные аргументы лежат прямо в объекте выражения,
/// а пустые типы (лямбды без захвата, заполнители) схлопываются в ноль байт через пустые базы.
/// Bind лямбды без захвата с одними заполнителями занимает 1 байт, как и сама лямбда.
///
/// Поддерживается:
///     - обычные функции, указатели на функции, лямбды и функторы;
///     - указатели на методы и на поля класса (объект передаётся значением, ссылкой,
///       сырым указателем, умным указателем или через SR_UTILS_NS::MakeRef);
///     - заполнители SR_UTILS_NS::Placeholders::_1 ... _10;
///     - SR_UTILS_NS::MakeRef / SR_UTILS_NS::CRef для передачи по ссылке;
///     - вложенные Bind-выражения (как у std::bind);
///     - SR_UTILS_NS::BindFront - более дешёвый вариант без заполнителей.
///
/// Пример:
///     using namespace SR_UTILS_NS::Placeholders;
///     auto callback = SR_UTILS_NS::Bind(&Foo::Update, this, _1, 0.5f);
///     callback(deltaTime);

/// MSVC не применяет оптимизацию пустых баз к множественному наследованию без явного указания
#ifndef SR_EMPTY_BASES
    #if defined(SR_MSVC)
        #define SR_EMPTY_BASES __declspec(empty_bases)
    #else
        #define SR_EMPTY_BASES
    #endif
#endif

namespace SR_UTILS_NS {
    /// ----------------------------------------------------------------------------------------------------------- ///
    /// Ref - замена std::reference_wrapper
    /// ----------------------------------------------------------------------------------------------------------- ///

    template<typename T> class Ref {
    public:
        using Type = T;

        SR_FAST_CONSTRUCTOR Ref(T& value) SR_NOEXCEPT /// NOLINT(google-explicit-constructor)
            : m_pValue(__builtin_addressof(value))
        { }

        Ref(T&&) = delete;

        SR_NODISCARD SR_FORCE_INLINE SR_CONSTEXPR T& Get() const SR_NOEXCEPT { return *m_pValue; }
        SR_FORCE_INLINE SR_CONSTEXPR operator T&() const SR_NOEXCEPT { return *m_pValue; } /// NOLINT(google-explicit-constructor)

    private:
        T* m_pValue = nullptr;

    };

    template<typename T> SR_NODISCARD SR_FORCE_INLINE SR_CONSTEXPR Ref<T> MakeRef(T& value) SR_NOEXCEPT { return Ref<T>(value); }
    template<typename T> SR_NODISCARD SR_FORCE_INLINE SR_CONSTEXPR Ref<const T> CRef(const T& value) SR_NOEXCEPT { return Ref<const T>(value); }

    template<typename T> void MakeRef(const T&&) = delete;
    template<typename T> void CRef(const T&&) = delete;

    /// ----------------------------------------------------------------------------------------------------------- ///
    /// Invoke - замена std::invoke
    /// ----------------------------------------------------------------------------------------------------------- ///

    namespace BindDetail {
        template<typename T> struct IsRefWrapper : std::false_type { };
        template<typename T> struct IsRefWrapper<Ref<T>> : std::true_type { };
        template<typename T> SR_CONSTEXPR bool IsRefWrapperV = IsRefWrapper<std::remove_cv_t<T>>::value;

        template<typename T> struct MemberPointerTraits { };
        template<typename TPointed, typename TClass> struct MemberPointerTraits<TPointed TClass::*> {
            using Class = TClass;
            using Pointed = TPointed;
        };

        template<typename TMemberPtr, typename TObject, typename... TArgs>
        SR_FORCE_INLINE SR_CONSTEXPR decltype(auto) InvokeMemberPtr(TMemberPtr memberPtr, TObject&& object, TArgs&&... args) {
            using Traits = MemberPointerTraits<std::remove_cv_t<TMemberPtr>>;
            using ObjectType = std::remove_cvref_t<TObject>;

            SR_CONSTEXPR bool isFunction = std::is_function_v<typename Traits::Pointed>;
            SR_CONSTEXPR bool isDirect = std::is_base_of_v<typename Traits::Class, ObjectType>;

            if SR_CONSTEXPR (isFunction) {
                if SR_CONSTEXPR (isDirect) {
                    return (std::forward<TObject>(object).*memberPtr)(std::forward<TArgs>(args)...);
                }
                else if SR_CONSTEXPR (IsRefWrapperV<ObjectType>) {
                    return (object.Get().*memberPtr)(std::forward<TArgs>(args)...);
                }
                else {
                    /// сырой указатель, итератор или любой умный указатель
                    return ((*std::forward<TObject>(object)).*memberPtr)(std::forward<TArgs>(args)...);
                }
            }
            else {
                static_assert(sizeof...(TArgs) == 0, "Pointer to data member cannot be invoked with arguments!");

                if SR_CONSTEXPR (isDirect) {
                    return std::forward<TObject>(object).*memberPtr;
                }
                else if SR_CONSTEXPR (IsRefWrapperV<ObjectType>) {
                    return object.Get().*memberPtr;
                }
                else {
                    return (*std::forward<TObject>(object)).*memberPtr;
                }
            }
        }
    }

    template<typename TCallable, typename... TArgs>
    SR_FORCE_INLINE SR_CONSTEXPR decltype(auto) Invoke(TCallable&& callable, TArgs&&... args) {
        if SR_CONSTEXPR (std::is_member_pointer_v<std::remove_cvref_t<TCallable>>) {
            static_assert(sizeof...(TArgs) > 0, "Member pointer requires an object to be invoked on!");
            return BindDetail::InvokeMemberPtr(callable, std::forward<TArgs>(args)...);
        }
        else {
            return std::forward<TCallable>(callable)(std::forward<TArgs>(args)...);
        }
    }

    template<typename TCallable, typename... TArgs>
    using InvokeResultT = decltype(SR_UTILS_NS::Invoke(std::declval<TCallable>(), std::declval<TArgs>()...));

    /// ----------------------------------------------------------------------------------------------------------- ///
    /// Заполнители
    /// ----------------------------------------------------------------------------------------------------------- ///

    template<uint32_t INDEX> struct PlaceholderT {
        static_assert(INDEX > 0, "Placeholder index starts from 1!");
        SR_INLINE_STATIC SR_CONSTEXPR uint32_t index = INDEX;
    };

    namespace Placeholders {
        SR_INLINE SR_CONSTEXPR PlaceholderT<1> _1 = { };
        SR_INLINE SR_CONSTEXPR PlaceholderT<2> _2 = { };
        SR_INLINE SR_CONSTEXPR PlaceholderT<3> _3 = { };
        SR_INLINE SR_CONSTEXPR PlaceholderT<4> _4 = { };
        SR_INLINE SR_CONSTEXPR PlaceholderT<5> _5 = { };
        SR_INLINE SR_CONSTEXPR PlaceholderT<6> _6 = { };
        SR_INLINE SR_CONSTEXPR PlaceholderT<7> _7 = { };
        SR_INLINE SR_CONSTEXPR PlaceholderT<8> _8 = { };
        SR_INLINE SR_CONSTEXPR PlaceholderT<9> _9 = { };
        SR_INLINE SR_CONSTEXPR PlaceholderT<10> _10 = { };
    }

    template<typename T> struct IsPlaceholder : std::false_type { };
    template<uint32_t INDEX> struct IsPlaceholder<PlaceholderT<INDEX>> : std::true_type { };
    template<typename T> SR_CONSTEXPR bool IsPlaceholderV = IsPlaceholder<std::remove_cv_t<T>>::value;

    template<typename T> struct IsBindExpression : std::false_type { };
    template<typename T> SR_CONSTEXPR bool IsBindExpressionV = IsBindExpression<std::remove_cv_t<T>>::value;

    /// ----------------------------------------------------------------------------------------------------------- ///
    /// Хранилище связанных аргументов
    /// ----------------------------------------------------------------------------------------------------------- ///

    namespace BindDetail {
        template<typename T> SR_CONSTEXPR bool IsInheritableV = std::is_empty_v<T> && !std::is_final_v<T> && !std::is_union_v<T>;

        /// Каждый аргумент лежит в отдельной базе, поэтому доступ к нему разрешается перегрузкой,
        /// без рекурсивного разворачивания списка типов.
        template<uint32_t INDEX, typename T, bool INHERIT = IsInheritableV<T>> struct BindElement {
            T value;

            SR_NODISCARD SR_FORCE_INLINE SR_CONSTEXPR T& Get() SR_NOEXCEPT { return value; }
            SR_NODISCARD SR_FORCE_INLINE SR_CONSTEXPR const T& Get() const SR_NOEXCEPT { return value; }
        };

        /// Пустой аргумент (заполнитель, лямбда без захвата, stateless-функтор) наследуется,
        /// а не хранится полем - только так он гарантированно занимает ноль байт.
        template<uint32_t INDEX, typename T> struct BindElement<INDEX, T, true> : T {
            BindElement() = default;

            template<typename U> SR_FAST_CONSTRUCTOR explicit BindElement(U&& value) /// NOLINT(bugprone-forwarding-reference-overload)
                : T(std::forward<U>(value))
            { }

            SR_NODISCARD SR_FORCE_INLINE SR_CONSTEXPR T& Get() SR_NOEXCEPT { return static_cast<T&>(*this); }
            SR_NODISCARD SR_FORCE_INLINE SR_CONSTEXPR const T& Get() const SR_NOEXCEPT { return static_cast<const T&>(*this); }
        };

        /// Вызываемый объект лежит в том же хранилище под отдельным индексом - так пустая лямбда
        /// схлопывается ровно так же, как и пустой связанный аргумент.
        SR_CONSTEXPR uint32_t BIND_CALLABLE_INDEX = ~static_cast<uint32_t>(0);

        /// SR_EMPTY_BASES обязателен и на основном шаблоне - иначе MSVC не схлопывает базы в специализации.
        template<typename TSequence, typename TCallable, typename... TArgs> struct SR_EMPTY_BASES BindStorage;

        /// Агрегат - инициализируется прямо списком баз, без единого лишнего копирования.
        template<uint32_t... INDICES, typename TCallable, typename... TArgs>
        struct SR_EMPTY_BASES BindStorage<std::integer_sequence<uint32_t, INDICES...>, TCallable, TArgs...>
            : BindElement<BIND_CALLABLE_INDEX, TCallable>
            , BindElement<INDICES, TArgs>...
        { };

        template<uint32_t INDEX, typename T, bool INHERIT>
        SR_NODISCARD SR_FORCE_INLINE SR_CONSTEXPR T& GetBindElement(BindElement<INDEX, T, INHERIT>& element) SR_NOEXCEPT { return element.Get(); }

        template<uint32_t INDEX, typename T, bool INHERIT>
        SR_NODISCARD SR_FORCE_INLINE SR_CONSTEXPR const T& GetBindElement(const BindElement<INDEX, T, INHERIT>& element) SR_NOEXCEPT { return element.Get(); }

        /// Выбор N-го аргумента вызова без создания кортежа.
        template<uint32_t INDEX, typename TFirst, typename... TOther>
        SR_NODISCARD SR_FORCE_INLINE SR_CONSTEXPR decltype(auto) GetNthArg(SR_MAYBE_UNUSED TFirst&& first, SR_MAYBE_UNUSED TOther&&... other) SR_NOEXCEPT {
            if SR_CONSTEXPR (INDEX == 0) {
                return static_cast<TFirst&&>(first);
            }
            else {
                static_assert(sizeof...(TOther) > 0, "Placeholder index is out of range of the call arguments!");
                return GetNthArg<INDEX - 1>(static_cast<TOther&&>(other)...);
            }
        }

        /// Превращает хранимый аргумент в то, что реально уйдёт в вызов.
        template<typename TStored, typename... TCallArgs>
        SR_NODISCARD SR_FORCE_INLINE SR_CONSTEXPR decltype(auto) SelectBindArg(TStored& stored, SR_MAYBE_UNUSED TCallArgs&&... callArgs) {
            using StoredType = std::remove_cv_t<TStored>;

            if SR_CONSTEXPR (IsPlaceholderV<StoredType>) {
                static_assert(StoredType::index <= sizeof...(TCallArgs), "Placeholder index is out of range of the call arguments!");
                return GetNthArg<StoredType::index - 1>(std::forward<TCallArgs>(callArgs)...);
            }
            else if SR_CONSTEXPR (IsRefWrapperV<StoredType>) {
                return stored.Get();
            }
            else if SR_CONSTEXPR (IsBindExpressionV<StoredType>) {
                return stored(std::forward<TCallArgs>(callArgs)...);
            }
            else {
                return static_cast<TStored&>(stored);
            }
        }
    }

    /// ----------------------------------------------------------------------------------------------------------- ///
    /// Bind-выражение
    /// ----------------------------------------------------------------------------------------------------------- ///

    /// Хранилище наследуется, а не хранится полем - так пустое хранилище гарантированно занимает ноль байт.
    template<typename TCallable, typename... TBoundArgs> class SR_EMPTY_BASES BindExpression
        : private BindDetail::BindStorage<std::make_integer_sequence<uint32_t, static_cast<uint32_t>(sizeof...(TBoundArgs))>, TCallable, TBoundArgs...>
    {
        using Indices = std::make_integer_sequence<uint32_t, static_cast<uint32_t>(sizeof...(TBoundArgs))>;
        using Storage = BindDetail::BindStorage<Indices, TCallable, TBoundArgs...>;

    public:
        template<typename TCallableArg, typename... TArgs>
            requires (!std::is_same_v<std::remove_cvref_t<TCallableArg>, BindExpression>)
        SR_FAST_CONSTRUCTOR explicit BindExpression(TCallableArg&& callable, TArgs&&... args)
            : Storage(MakeStorage(Indices{ }, std::forward<TCallableArg>(callable), std::forward<TArgs>(args)...))
        {
            static_assert(sizeof...(TArgs) == sizeof...(TBoundArgs), "Bound arguments count mismatch!");
        }

        template<typename... TCallArgs>
        SR_FORCE_INLINE SR_CONSTEXPR decltype(auto) operator()(TCallArgs&&... callArgs) {
            return Call(Indices{ }, static_cast<Storage&>(*this), std::forward<TCallArgs>(callArgs)...);
        }

        template<typename... TCallArgs>
        SR_FORCE_INLINE SR_CONSTEXPR decltype(auto) operator()(TCallArgs&&... callArgs) const {
            return Call(Indices{ }, static_cast<const Storage&>(*this), std::forward<TCallArgs>(callArgs)...);
        }

    private:
        /// Возвращается prvalue, поэтому хранилище строится сразу по месту - без копирования и перемещения.
        template<uint32_t... INDICES, typename TCallableArg, typename... TArgs>
        SR_NODISCARD SR_FORCE_INLINE SR_CONSTEXPR static Storage MakeStorage(std::integer_sequence<uint32_t, INDICES...>, TCallableArg&& callable, TArgs&&... args) {
            return Storage{
                BindDetail::BindElement<BindDetail::BIND_CALLABLE_INDEX, TCallable>{ std::forward<TCallableArg>(callable) },
                BindDetail::BindElement<INDICES, TBoundArgs>{ std::forward<TArgs>(args) }...
            };
        }

        template<uint32_t... INDICES, typename TStorage, typename... TCallArgs>
        SR_FORCE_INLINE SR_CONSTEXPR static decltype(auto) Call(
            std::integer_sequence<uint32_t, INDICES...>,
            TStorage& storage,
            TCallArgs&&... callArgs
        ) {
            return SR_UTILS_NS::Invoke(
                BindDetail::GetBindElement<BindDetail::BIND_CALLABLE_INDEX>(storage),
                BindDetail::SelectBindArg(BindDetail::GetBindElement<INDICES>(storage), std::forward<TCallArgs>(callArgs)...)...
            );
        }

    };

    template<typename TCallable, typename... TBoundArgs>
    struct IsBindExpression<BindExpression<TCallable, TBoundArgs...>> : std::true_type { };

    /// Связывает вызываемый объект с аргументами. Все аргументы копируются (decay) в возвращаемый объект,
    /// чтобы передать что-то по ссылке - используйте SR_UTILS_NS::MakeRef / SR_UTILS_NS::CRef.
    template<typename TCallable, typename... TArgs>
    SR_NODISCARD SR_FORCE_INLINE SR_CONSTEXPR auto Bind(TCallable&& callable, TArgs&&... args) {
        using ResultType = BindExpression<std::decay_t<TCallable>, std::decay_t<TArgs>...>;
        return ResultType(std::forward<TCallable>(callable), std::forward<TArgs>(args)...);
    }

    /// ----------------------------------------------------------------------------------------------------------- ///
    /// BindFront - аналог std::bind_front. Без заполнителей и потому дешевле по компиляции
    /// ----------------------------------------------------------------------------------------------------------- ///

    /// Хранилище наследуется, а не хранится полем - так пустое хранилище гарантированно занимает ноль байт.
    template<typename TCallable, typename... TBoundArgs> class SR_EMPTY_BASES BindFrontExpression
        : private BindDetail::BindStorage<std::make_integer_sequence<uint32_t, static_cast<uint32_t>(sizeof...(TBoundArgs))>, TCallable, TBoundArgs...>
    {
        using Indices = std::make_integer_sequence<uint32_t, static_cast<uint32_t>(sizeof...(TBoundArgs))>;
        using Storage = BindDetail::BindStorage<Indices, TCallable, TBoundArgs...>;

    public:
        template<typename TCallableArg, typename... TArgs>
            requires (!std::is_same_v<std::remove_cvref_t<TCallableArg>, BindFrontExpression>)
        SR_FAST_CONSTRUCTOR explicit BindFrontExpression(TCallableArg&& callable, TArgs&&... args)
            : Storage(MakeStorage(Indices{ }, std::forward<TCallableArg>(callable), std::forward<TArgs>(args)...))
        {
            static_assert(sizeof...(TArgs) == sizeof...(TBoundArgs), "Bound arguments count mismatch!");
        }

        template<typename... TCallArgs>
        SR_FORCE_INLINE SR_CONSTEXPR decltype(auto) operator()(TCallArgs&&... callArgs) {
            return Call(Indices{ }, static_cast<Storage&>(*this), std::forward<TCallArgs>(callArgs)...);
        }

        template<typename... TCallArgs>
        SR_FORCE_INLINE SR_CONSTEXPR decltype(auto) operator()(TCallArgs&&... callArgs) const {
            return Call(Indices{ }, static_cast<const Storage&>(*this), std::forward<TCallArgs>(callArgs)...);
        }

    private:
        template<uint32_t... INDICES, typename TCallableArg, typename... TArgs>
        SR_NODISCARD SR_FORCE_INLINE SR_CONSTEXPR static Storage MakeStorage(std::integer_sequence<uint32_t, INDICES...>, TCallableArg&& callable, TArgs&&... args) {
            return Storage{
                BindDetail::BindElement<BindDetail::BIND_CALLABLE_INDEX, TCallable>{ std::forward<TCallableArg>(callable) },
                BindDetail::BindElement<INDICES, TBoundArgs>{ std::forward<TArgs>(args) }...
            };
        }

        template<uint32_t... INDICES, typename TStorage, typename... TCallArgs>
        SR_FORCE_INLINE SR_CONSTEXPR static decltype(auto) Call(
            std::integer_sequence<uint32_t, INDICES...>,
            TStorage& storage,
            TCallArgs&&... callArgs
        ) {
            return SR_UTILS_NS::Invoke(
                BindDetail::GetBindElement<BindDetail::BIND_CALLABLE_INDEX>(storage),
                BindDetail::GetBindElement<INDICES>(storage)...,
                std::forward<TCallArgs>(callArgs)...
            );
        }

    };

    template<typename TCallable, typename... TBoundArgs>
    struct IsBindExpression<BindFrontExpression<TCallable, TBoundArgs...>> : std::true_type { };

    template<typename TCallable, typename... TArgs>
    SR_NODISCARD SR_FORCE_INLINE SR_CONSTEXPR auto BindFront(TCallable&& callable, TArgs&&... args) {
        using ResultType = BindFrontExpression<std::decay_t<TCallable>, std::decay_t<TArgs>...>;
        return ResultType(std::forward<TCallable>(callable), std::forward<TArgs>(args)...);
    }
}

#endif //SR_ENGINE_BIND_H
