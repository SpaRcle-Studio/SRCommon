//
// Created by Monika on 17.02.2022.
//

#ifndef SR_ENGINE_EVENT_H
#define SR_ENGINE_EVENT_H

#include <Utils/Debug.h>

namespace SR_UTILS_NS {
    class SR_COMMON_DLL_API IEvent {
    protected:
        IEvent();
        virtual ~IEvent();

    public:
        SR_NODISCARD virtual const std::string& GetEventName() const = 0;

    };

    template <typename ..._args> class SR_COMMON_DLL_API Event : public IEvent {
        using CallBack = std::function<void(_args...)>;
    public:
        explicit Event(std::string name)
            : m_name(std::move(name))
        { }

        ~Event() override = default;

    public:
        virtual void Trigger(_args... a) { }

    private:
        SR_NODISCARD const std::string& GetEventName() const override;

    private:
        std::string m_name;

    };

#ifdef SR_COMMON_DLL_EXPORTS
    template<typename... _args> const std::string &Event<_args...>::GetEventName() const {
        return m_name;
    }
#endif
}

#endif //SR_ENGINE_EVENT_H
