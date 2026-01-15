//
// Created by Monika on 21.07.2022.
//

#ifndef SR_ENGINE_FUNCTION_H
#define SR_ENGINE_FUNCTION_H

#include <Utils/Common/AssertFwd.h>

#define SR_EMPTY_FUNCTION_QUALIFIER

#if defined(SR_COMPILER_CPP17_ENABLED)
    #define SR_GENERATE_MEMBER_FUNCTION_NOEXCEPT_VARIANTS(PATTERN, OTHER_QUALIFIER) \
        PATTERN(OTHER_QUALIFIER SR_EMPTY_FUNCTION_QUALIFIER) \
        PATTERN(OTHER_QUALIFIER noexcept)
#else
    #define SR_GENERATE_MEMBER_FUNCTION_NOEXCEPT_VARIANTS(PATTERN, OTHER_QUALIFIER) \
	    PATTERN(OTHER_QUALIFIER SR_EMPTY_FUNCTION_QUALIFIER)
#endif

#define SR_GENERATE_MEMBER_FUNCTION_REF_VARIANTS(PATTERN, OTHER_QUALIFIER) \
	SR_GENERATE_MEMBER_FUNCTION_NOEXCEPT_VARIANTS(PATTERN, OTHER_QUALIFIER SR_EMPTY_FUNCTION_QUALIFIER) \
	SR_GENERATE_MEMBER_FUNCTION_NOEXCEPT_VARIANTS(PATTERN, OTHER_QUALIFIER &) \
	SR_GENERATE_MEMBER_FUNCTION_NOEXCEPT_VARIANTS(PATTERN, OTHER_QUALIFIER &&)

#define SR_GENERATE_MEMBER_FUNCTION_VOLATILE_VARIANTS(PATTERN, OTHER_QUALIFIER) \
	SR_GENERATE_MEMBER_FUNCTION_REF_VARIANTS(PATTERN, OTHER_QUALIFIER SR_EMPTY_FUNCTION_QUALIFIER) \
	SR_GENERATE_MEMBER_FUNCTION_REF_VARIANTS(PATTERN, OTHER_QUALIFIER volatile)

#define SR_GENERATE_MEMBER_FUNCTION_CONST_VARIANTS(PATTERN) \
	SR_GENERATE_MEMBER_FUNCTION_VOLATILE_VARIANTS(PATTERN, SR_EMPTY_FUNCTION_QUALIFIER) \
	SR_GENERATE_MEMBER_FUNCTION_VOLATILE_VARIANTS(PATTERN, const)

// Helper to generate all combination of qualifiers you can apply to member functions.
// PATTERN must be a macro that will receive as an argument a possible combination of qualifiers and generate a pattern from it.
#define SR_GENERATE_MEMBER_FUNCTION_VARIANTS(PATTERN) SR_GENERATE_MEMBER_FUNCTION_CONST_VARIANTS(PATTERN)

namespace SR_UTILS_NS::Details {
    class UnusedClass {};

    union FunctorStorageAligment {
        void (*unused_func_ptr)(void);
        void (UnusedClass::*unused_func_mem_ptr)(void);
        void* unused_ptr;
        int64_t unused_var;
    };

    template <int SIZE_IN_BYTES>
    struct FunctorStorage {
        static_assert(SIZE_IN_BYTES >= 0, "local buffer storage cannot have a negative size!");
        union {
            FunctorStorageAligment align;
            char storage[SIZE_IN_BYTES];
        };
    };

    template <> struct FunctorStorage<0> {
        union {
            FunctorStorageAligment align;
            char storage[sizeof(FunctorStorageAligment)];
        };
    };

    template <typename Functor, int SIZE_IN_BYTES> struct IsFunctorInPlaceAllocatable {
        static SR_CONSTEXPR bool value = sizeof(Functor) <= sizeof(FunctorStorage<SIZE_IN_BYTES>) && (alignof(FunctorStorage<SIZE_IN_BYTES>) % alignof(Functor)) == 0;
    };

    enum ManagerOperations : int
    {
        MGROPS_DESTRUCT_FUNCTOR = 0,
        MGROPS_COPY_FUNCTOR = 1,
        MGROPS_MOVE_FUNCTOR = 2,
    #if SR_RTTI_ENABLED
        MGROPS_GET_TYPE_INFO = 3,
        MGROPS_GET_FUNC_PTR = 4,
    #endif
    };

    template <int SIZE_IN_BYTES> class FunctionBaseDetail {
    public:
        using FunctorStorageType = FunctorStorage<SIZE_IN_BYTES>;
        FunctorStorageType mStorage;

        template <typename Functor, typename = void>
        class FunctionManagerBase
        {
        public:
            static Functor* GetFunctorPtr(void* storage) SR_NOEXCEPT {
                return static_cast<Functor*>(storage);
            }

            template <typename T> static void CreateFunctor(void* storage, T&& functor) {
                ::new (GetFunctorPtr(storage)) Functor(std::forward<T>(functor));
            }

            static void DestructFunctor(void* storage) {
                GetFunctorPtr(storage)->~Functor();
            }

            static void CopyFunctor(void* to, void* from) {
                ::new (GetFunctorPtr(to)) Functor(*GetFunctorPtr(from));
            }

            static void MoveFunctor(void* to, void* from) SR_NOEXCEPT {
                ::new (GetFunctorPtr(to)) Functor(std::move(*GetFunctorPtr(from)));
            }

            static void* Manager(void* to, void* from, typename Details::ManagerOperations ops) SR_NOEXCEPT {
                switch (ops) {
                    case MGROPS_DESTRUCT_FUNCTOR: {
                        DestructFunctor(to);
                        break;
                    }
                    case MGROPS_COPY_FUNCTOR: {
                        CopyFunctor(to, from);
                        break;
                    }
                    case MGROPS_MOVE_FUNCTOR: {
                        MoveFunctor(to, from);
                        DestructFunctor(from);
                        break;
                    }
                    default:
                        break;
                }
                return nullptr;
            }
        };

        // Functor is allocated on the heap
        template <typename Functor>
        class FunctionManagerBase<Functor, typename std::enable_if<!IsFunctorInPlaceAllocatable<Functor, SIZE_IN_BYTES>::value>::type> {
        public:
            static Functor* GetFunctorPtr(void* storage) SR_NOEXCEPT {
                return *static_cast<Functor**>(storage);
            }

            static Functor*& GetFunctorPtrRef(void* storage) SR_NOEXCEPT {
                return *static_cast<Functor**>(storage);
            }

            template <typename T> static void CreateFunctor(void* storage, T&& functor) {
                auto* func = static_cast<Functor*>(
                    ::operator new(sizeof(Functor), std::align_val_t(alignof(Functor)))
                );
                ::new (static_cast<void*>(func)) Functor(std::forward<T>(functor));
                GetFunctorPtrRef(storage) = func;
            }

            static void DestructFunctor(void* storage) {
                Functor* func = GetFunctorPtr(storage);
                if (func) {
                    func->~Functor();
                    ::operator delete(func, std::align_val_t(alignof(Functor)));
                }
            }

            static void CopyFunctor(void* to, void* from) {
                Functor* func = static_cast<Functor*>(
                    ::operator new(sizeof(Functor), std::align_val_t(alignof(Functor)))
                );

                ::new (static_cast<void*>(func)) Functor(*GetFunctorPtr(from));
                GetFunctorPtrRef(to) = func;
            }

            static void MoveFunctor(void* to, void* from) SR_NOEXCEPT {
                Functor* func = GetFunctorPtr(from);
                GetFunctorPtrRef(to) = func;
                GetFunctorPtrRef(from) = nullptr;
            }

            static void* Manager(void* to, void* from, typename Details::ManagerOperations ops) SR_NOEXCEPT {
                switch (ops) {
                    case MGROPS_DESTRUCT_FUNCTOR: {
                        DestructFunctor(to);
                        break;
                    }
                    case MGROPS_COPY_FUNCTOR: {
                        CopyFunctor(to, from);
                        break;
                    }
                    case MGROPS_MOVE_FUNCTOR: {
                        MoveFunctor(to, from);
                        break;
                    }
                    default:
                        break;
                }
                return nullptr;
            }
        };

        template <typename Functor, typename R, typename... Args> class FunctionManager final : public FunctionManagerBase<Functor> {
        public:
            using Base = FunctionManagerBase<Functor>;

        #if SR_RTTI_ENABLED
            static void* GetTypeInfo() SR_NOEXCEPT {
                return reinterpret_cast<void*>(const_cast<std::type_info*>(&typeid(Functor)));
            }

            static void* Manager(void* to, void* from, ManagerOperations ops) SR_NOEXCEPT {
                switch (ops) {
                    case MGROPS_GET_TYPE_INFO: {
                        return GetTypeInfo();
                    }
                    case MGROPS_GET_FUNC_PTR: {
                        return static_cast<void*>(Base::GetFunctorPtr(to));
                    }
                    default: {
                        return Base::Manager(to, from, ops);
                    }
                }
            }
        #endif
            static R Invoker(Args... args, void* storage) {
                return std::invoke(*Base::GetFunctorPtr(storage), std::forward<Args>(args)...);
            }
        };

        FunctionBaseDetail() SR_NOEXCEPT = default;
        ~FunctionBaseDetail() SR_NOEXCEPT = default;
    };

    #define SR_INTERNAL_FUNCTION_VALID_FUNCTION_ARGS(FUNCTOR, RET, ARGS, BASE, MYSELF)         \
                typename std::enable_if_t<std::is_invocable_r_v<RET, FUNCTOR, ARGS> &&         \
                                            !std::is_base_of_v<BASE, std::decay_t<FUNCTOR>> && \
                                            !std::is_same_v<std::decay_t<FUNCTOR>, MYSELF>>

    #define SR_INTERNAL_FUNCTION_DETAIL_VALID_FUNCTION_ARGS(FUNCTOR, RET, ARGS, MYSELF) \
                SR_INTERNAL_FUNCTION_VALID_FUNCTION_ARGS(FUNCTOR, RET, ARGS, MYSELF, MYSELF)

    template <int, typename>
    class FunctionDetail;

    template <int SIZE_IN_BYTES, typename R, typename... Args>
    class FunctionDetail<SIZE_IN_BYTES, R(Args...)> : public FunctionBaseDetail<SIZE_IN_BYTES> {
    template <int OTHER_SIZE_IN_BYTES, typename T>
    friend class FunctionDetail;
    public:
        using ResultType = R;

    protected:
        using Base = FunctionBaseDetail<SIZE_IN_BYTES>;
        using FunctorStorageType = typename FunctionBaseDetail<SIZE_IN_BYTES>::FunctorStorageType;
        using Base::mStorage;

    public:
        FunctionDetail() SR_NOEXCEPT = default;
        FunctionDetail(std::nullptr_t) SR_NOEXCEPT {}

        FunctionDetail(const FunctionDetail& other) {
            if (this != &other) {
                Copy(other);
            }
        }

        FunctionDetail(FunctionDetail&& other) {
            if (this != &other) {
                Move(std::move(other));
            }
        }

        template<int OTHER_SIZE_IN_BYTES>
        FunctionDetail(FunctionDetail <OTHER_SIZE_IN_BYTES, R(Args...)> && other) {
            static_assert(OTHER_SIZE_IN_BYTES < SIZE_IN_BYTES, "Other sized FunctionDetail must be smaller");
            Move(std::move(other));
        }

        template <int OTHER_SIZE_IN_BYTES>
        FunctionDetail(const FunctionDetail<OTHER_SIZE_IN_BYTES, R(Args...)>& other) {
            static_assert(OTHER_SIZE_IN_BYTES < SIZE_IN_BYTES, "Other sized FunctionDetail must be smaller");
            Copy(other);
        }

        template <typename Functor, typename = SR_INTERNAL_FUNCTION_DETAIL_VALID_FUNCTION_ARGS(Functor, R, Args..., FunctionDetail)>
        FunctionDetail(Functor&& functor) {
            CreateForwardFunctor(std::forward<Functor>(functor));
        }

        ~FunctionDetail() SR_NOEXCEPT {
            Destroy();
        }

        FunctionDetail& operator=(const FunctionDetail& other) {
            if (this != &other) {
                Destroy();
                Copy(other);
            }

            return *this;
        }

        FunctionDetail& operator=(FunctionDetail&& other) {
            if(this != &other) {
                Destroy();
                Move(std::move(other));
            }
            return *this;
        }

        template <int OTHER_SIZE_IN_BYTES>
        FunctionDetail& operator=(const FunctionDetail<OTHER_SIZE_IN_BYTES, R(Args...)>& other) {
            static_assert(OTHER_SIZE_IN_BYTES < SIZE_IN_BYTES, "Other sized FunctionDetail must be smaller");
            Destroy();
            Copy(other);
            return *this;
        }

        template <int OTHER_SIZE_IN_BYTES>
        FunctionDetail& operator=(FunctionDetail<OTHER_SIZE_IN_BYTES, R(Args...)>&& other) {
            static_assert(OTHER_SIZE_IN_BYTES < SIZE_IN_BYTES, "Other sized FunctionDetail must be smaller");
            Destroy();
            Move(std::move(other));
            return *this;
        }

        FunctionDetail& operator=(std::nullptr_t) SR_NOEXCEPT {
            Destroy();
            mMgrFuncPtr = nullptr;
            mInvokeFuncPtr = &DefaultInvoker;

            return *this;
        }

        template <typename Functor, typename = SR_INTERNAL_FUNCTION_DETAIL_VALID_FUNCTION_ARGS(Functor, R, Args..., FunctionDetail)>
        FunctionDetail& operator=(Functor&& functor) {
            Destroy();
            CreateForwardFunctor(std::forward<Functor>(functor));
            return *this;
        }

        template <typename Functor>
        FunctionDetail& operator=(std::reference_wrapper<Functor> f) SR_NOEXCEPT {
            Destroy();
            CreateForwardFunctor(f);
            return *this;
        }

        void swap(FunctionDetail& other) SR_NOEXCEPT {
            if(this == &other)
                return;

            FunctorStorageType tempStorage;
            if (other.HaveManager()) {
                (void)(*other.mMgrFuncPtr)(tempStorage.storage, other.mStorage.storage, ManagerOperations::MGROPS_MOVE_FUNCTOR);
            }

            if (HaveManager()) {
                (void)(*mMgrFuncPtr)(other.mStorage.storage, mStorage.storage, ManagerOperations::MGROPS_MOVE_FUNCTOR);
            }

            if (other.HaveManager()) {
                (void)(*other.mMgrFuncPtr)(mStorage.storage, tempStorage.storage, ManagerOperations::MGROPS_MOVE_FUNCTOR);
            }

            std::swap(mMgrFuncPtr, other.mMgrFuncPtr);
            std::swap(mInvokeFuncPtr, other.mInvokeFuncPtr);
        }

        explicit operator bool() const SR_NOEXCEPT
        {
            return HaveManager();
        }

        SR_FORCE_INLINE R operator ()(Args... args) const {
            return (*mInvokeFuncPtr)(std::forward<Args>(args)..., const_cast<char*>(this->mStorage.storage));
        }

        #if SR_RTTI_ENABLED
            const std::type_info& target_type() const SR_NOEXCEPT {
                if (HaveManager()) {
                    void* ret = (*mMgrFuncPtr)(nullptr, nullptr, ManagerOperations::MGROPS_GET_TYPE_INFO);
                    return *(static_cast<const std::type_info*>(ret));
                }
                return typeid(void);
            }

            template <typename Functor>
            Functor* target() SR_NOEXCEPT {
                if (HaveManager() && target_type() == typeid(Functor)) {
                    void* ret = (*mMgrFuncPtr)(mStorage.storage, nullptr, ManagerOperations::MGROPS_GET_FUNC_PTR);
                    return ret ? static_cast<Functor*>(ret) : nullptr;
                }
                return nullptr;
            }

            template <typename Functor>
            const Functor* target() const SR_NOEXCEPT {
                if (HaveManager() && target_type() == typeid(Functor)) {
                    const void* ret = (*mMgrFuncPtr)(const_cast<char*>(mStorage.storage), nullptr, ManagerOperations::MGROPS_GET_FUNC_PTR);
                    return ret ? static_cast<const Functor*>(ret) : nullptr;
                }
                return nullptr;
            }
        #endif

    private:
        bool HaveManager() const SR_NOEXCEPT {
            return (mMgrFuncPtr != nullptr);
        }

        void Destroy() SR_NOEXCEPT {
            if (HaveManager()) {
                (void)(*mMgrFuncPtr)(mStorage.storage, nullptr, ManagerOperations::MGROPS_DESTRUCT_FUNCTOR);
            }
        }

        void Copy(const FunctionDetail& other) {
            if (other.HaveManager()) {
                (void)(*other.mMgrFuncPtr)(mStorage.storage, const_cast<char*>(other.mStorage.storage), ManagerOperations::MGROPS_COPY_FUNCTOR);
            }

            mMgrFuncPtr = other.mMgrFuncPtr;
            mInvokeFuncPtr = other.mInvokeFuncPtr;
        }

        template <int OTHER_SIZE_IN_BYTES>
        void Copy(const FunctionDetail<OTHER_SIZE_IN_BYTES, R(Args...)>& other) {
            static_assert(OTHER_SIZE_IN_BYTES < SIZE_IN_BYTES, "Other sized FunctionDetail must be smaller");

            if (other.HaveManager()) {
                (void)(*other.mMgrFuncPtr)(mStorage.storage, const_cast<char*>(other.mStorage.storage), ManagerOperations::MGROPS_COPY_FUNCTOR);
            }

            mMgrFuncPtr = other.mMgrFuncPtr;
            mInvokeFuncPtr = other.mInvokeFuncPtr;
        }

        void Move(FunctionDetail&& other) {
            if (other.HaveManager()) {
                (void)(*other.mMgrFuncPtr)(mStorage.storage, other.mStorage.storage, ManagerOperations::MGROPS_MOVE_FUNCTOR);
            }

            mMgrFuncPtr = other.mMgrFuncPtr;
            mInvokeFuncPtr = other.mInvokeFuncPtr;
            other.mMgrFuncPtr = nullptr;
            other.mInvokeFuncPtr = &DefaultInvoker;
        }

        template <int OTHER_SIZE_IN_BYTES>
        void Move(FunctionDetail<OTHER_SIZE_IN_BYTES, R(Args...)>&& other) {
            static_assert(OTHER_SIZE_IN_BYTES < SIZE_IN_BYTES, "Other sized FunctionDetail must be smaller");

            if (other.HaveManager()) {
                (void)(*other.mMgrFuncPtr)(mStorage.storage, other.mStorage.storage, ManagerOperations::MGROPS_MOVE_FUNCTOR);
            }

            mMgrFuncPtr = other.mMgrFuncPtr;
            mInvokeFuncPtr = other.mInvokeFuncPtr;

            other.mMgrFuncPtr = nullptr;
            other.mInvokeFuncPtr = &FunctionDetail<OTHER_SIZE_IN_BYTES, R(Args...)>::DefaultInvoker;
        }

        template <typename Functor>
        void CreateForwardFunctor(Functor&& functor) {
            using DecayedFunctorType = typename std::decay<Functor>::type;
            using FunctionManagerType = typename Base::template FunctionManager<DecayedFunctorType, R, Args...>;

            if (IsNullCallable(functor)) {
                mMgrFuncPtr = nullptr;
                mInvokeFuncPtr = &DefaultInvoker;
            }
            else {
                mMgrFuncPtr = &FunctionManagerType::Manager;
                mInvokeFuncPtr = &FunctionManagerType::Invoker;
                FunctionManagerType::CreateFunctor(mStorage.storage, std::forward<Functor>(functor));
            }
        }

    private:
        typedef void* (*ManagerFuncPtr)(void*, void*, typename Details::ManagerOperations);
        typedef R (*InvokeFuncPtr)(Args..., void*);

        static R DefaultInvoker(Args... /*args*/, void* /*storage*/)
        {
            SRHalt("FunctionDetail call on an empty function_detail<R(Args..)>");
            // We want to explicitly crash here, since calling this function is equivalent
            // to dereferencing a nullptr, we don't want to silently continue execution and
            // have this function (which has no return value) potentially return arbitrary
            // garbage to the caller.
            *((volatile int*)0) = 0xDEADC0DE;
            std::terminate();
        };

        ManagerFuncPtr mMgrFuncPtr = nullptr;
        InvokeFuncPtr mInvokeFuncPtr = &DefaultInvoker;
    };

    template <class Callable>
    struct extract_signature_from_callable;

    #define SR_EXTRACT_SIGNATURE_PATTERN(QUALIFIERS) \
        template <typename ReturnType, typename MemberOfType, typename... Args> \
        struct extract_signature_from_callable<ReturnType (MemberOfType::*)(Args...) QUALIFIERS> \
        { \
            using type = ReturnType(Args...); \
        };

    SR_GENERATE_MEMBER_FUNCTION_VARIANTS(SR_EXTRACT_SIGNATURE_PATTERN)

    // Helper
    template <typename Callable>
    using extract_signature_from_callable_t = typename extract_signature_from_callable<Callable>::type;
}

namespace SR_UTILS_NS {
    /*template <typename UnusedType> class Function;

    template <typename ReturnType, typename... ArgumentTypes>
    class Function <ReturnType (ArgumentTypes...)>
    {
        class function_holder_base;
        using invoker_t = std::unique_ptr<function_holder_base>;
    public:
        typedef ReturnType signature_type(ArgumentTypes...);

        Function()
            : mInvoker()
        { }

        template <typename FunctionT> Function(FunctionT f) /// NOLINT
            : mInvoker(new free_function_holder<FunctionT>(f))
        { }

        Function(Function&& function) noexcept
            : mInvoker(std::move(function.mInvoker))
        { }

        Function& operator=(Function&& function) noexcept {
            mInvoker = std::move(function.mInvoker);
            return *this;
        }

        template <typename FunctionType, typename ClassType> Function(FunctionType ClassType::* f) /// NOLINT
            : mInvoker(new member_function_holder<FunctionType, ArgumentTypes ...>(f))
        { }

        Function(const Function& other)
            : mInvoker((bool)other.mInvoker.get() ? other.mInvoker->clone() : invoker_t())
        { }

        Function& operator=(const Function& other) {
            mInvoker = (bool)other.mInvoker.get() ? other.mInvoker->clone() : invoker_t();
            return *this;
        }

        ReturnType operator()(ArgumentTypes... args) const noexcept {
            return mInvoker->invoke(args...);
        }

        operator bool() const { /// NOLINT
            return mInvoker.get();
        }

    private:
        class function_holder_base
        {
        public:
            function_holder_base() = default;
            virtual ~function_holder_base() = default;

            virtual ReturnType invoke(ArgumentTypes... args) = 0;
            virtual invoker_t clone() = 0;

        private:
            function_holder_base(const function_holder_base&); /// NOLINT
            void operator=(const function_holder_base&); /// NOLINT

        };

        template <typename FunctionT>
        class free_function_holder : public function_holder_base
        {
        public:
            free_function_holder(FunctionT func) /// NOLINT
                : function_holder_base()
                , mFunction(func)
            { }

            virtual ReturnType invoke(ArgumentTypes... args) {
                return mFunction(args...);
            }

            virtual invoker_t clone() {
                return invoker_t(new free_function_holder(mFunction));
            }

        private:
            FunctionT mFunction;

        };

        template <typename FunctionType, typename ClassType, typename ... RestArgumentTypes>
        class member_function_holder : public function_holder_base
        {
        public:
            typedef FunctionType ClassType::* member_function_signature_t;

            member_function_holder(member_function_signature_t f) /// NOLINT
                : mFunction(f)
            { }

            virtual ReturnType invoke(ClassType obj, RestArgumentTypes... restArgs) {
                return (obj.*mFunction)(restArgs...);
            }

            virtual invoker_t clone() {
                return invoker_t(new member_function_holder(mFunction));
            }

        private:
            member_function_signature_t mFunction;

        };

        invoker_t mInvoker;
    };*/

    #ifndef SR_FUNCTION_DEFAULT_CAPTURE_SSO_SIZE
		#define SR_FUNCTION_DEFAULT_CAPTURE_SSO_SIZE (2 * sizeof(void*))
	#endif

	static_assert(SR_FUNCTION_DEFAULT_CAPTURE_SSO_SIZE >= sizeof(void*), "Functor storage must be able to hold at least a pointer!");

	template <typename>
	class Function;

	template <typename R, typename... Args>
	class Function<R(Args...)> : public Details::FunctionDetail<SR_FUNCTION_DEFAULT_CAPTURE_SSO_SIZE, R(Args...)>
	{
	private:
		using Super = Details::FunctionDetail<SR_FUNCTION_DEFAULT_CAPTURE_SSO_SIZE, R(Args...)>;
	public:
		using typename Super::ResultType;

        Function() SR_NOEXCEPT = default;
        Function(std::nullptr_t p) SR_NOEXCEPT
			: Super(p)
		{ }

        Function(const Function& other)
			: Super(other)
		{ }

        Function(Function&& other)
			: Super(std::move(other))
		{ }

		template <typename Functor, typename = SR_INTERNAL_FUNCTION_VALID_FUNCTION_ARGS(Functor, R, Args..., Super, Function)>
        Function(Functor&& functor)
			: Super(std::forward<Functor>(functor))
		{ }

		~Function() SR_NOEXCEPT = default;

        Function& operator=(const Function& other) {
            Super::operator=(other);
			return *this;
		}

        Function& operator=(Function&& other) {
            Super::operator=(std::move(other));
			return *this;
		}

        Function& operator=(std::nullptr_t p) SR_NOEXCEPT {
            Super::operator=(p);
			return *this;
		}

		template <typename Functor, typename = EASTL_INTERNAL_FUNCTION_VALID_FUNCTION_ARGS(Functor, R, Args..., Super, Function)>
        Function& operator=(Functor&& functor) {
            Super::operator=(std::forward<Functor>(functor));
			return *this;
		}

		template <typename Functor> Function& operator=(std::reference_wrapper<Functor> f) SR_NOEXCEPT {
            Super::operator=(f);
			return *this;
		}

		void swap(Function& other) SR_NOEXCEPT {
            Super::swap(other);
		}

		explicit operator bool() const SR_NOEXCEPT {
			return Super::operator bool();
		}

		R operator ()(Args... args) const {
			return Super::operator ()(std::forward<Args>(args)...);
		}

	#if SR_RTTI_ENABLED
		const std::type_info& target_type() const SR_NOEXCEPT {
			return Super::target_type();
		}

		template <typename Functor>
		Functor* target() SR_NOEXCEPT {
			return Super::template target<Functor>();
		}

		template <typename Functor>
		const Functor* target() const SR_NOEXCEPT {
			return Super::template target<Functor>();
		}
	#endif
	};

	template <typename R, typename... Args>
	bool operator==(const Function<R(Args...)>& f, std::nullptr_t) SR_NOEXCEPT
	{
		return !f;
	}
#if !defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	template <typename R, typename... Args>
	bool operator==(std::nullptr_t, const Function<R(Args...)>& f) SR_NOEXCEPT
	{
		return !f;
	}

	template <typename R, typename... Args>
	bool operator!=(const Function<R(Args...)>& f, std::nullptr_t) SR_NOEXCEPT
	{
		return !!f;
	}

	template <typename R, typename... Args>
	bool operator!=(std::nullptr_t, const Function<R(Args...)>& f) SR_NOEXCEPT
	{
		return !!f;
	}
#endif
	template <typename R, typename... Args>
	void swap(Function<R(Args...)>& lhs, Function<R(Args...)>& rhs)
	{
		lhs.swap(rhs);
	}

#ifdef __cpp_deduction_guides
	template<typename ReturnType, typename... Args>
    Function(ReturnType(*)(Args...)) -> Function<ReturnType(Args...)>;

	template<typename Callable>
    Function(Callable) -> Function<Details::extract_signature_from_callable_t<decltype(&Callable::operator())>>;
#endif
}

namespace SR_HTYPES_NS {
    using SR_UTILS_NS::Function;
}

#endif //SR_ENGINE_FUNCTION_H
