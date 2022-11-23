#pragma once

template<class T>
class JsonValueHolder
{
public:

    JsonValueHolder(std::function<void(void)> func)
        : m_hotReloadFunc(func)
    {}

    const T& Get() const {
        return holder;
    }

    void HotReload() {
        m_hotReloadFunc();
    }

private:

    T m_holder;
    std::function<void(void)> m_hotReloadFunc;

};
