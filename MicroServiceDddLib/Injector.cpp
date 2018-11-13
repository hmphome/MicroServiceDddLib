#include "stdafx.h"

#include "Types.h"

#include <algorithm>

namespace MicroServiceDddLib
{
    Injector & MicroServiceDddLib::Injector::Instance() noexcept
    {
        static Injector instance;
        return instance;
    }

    uint8_t Injector::Storage_t::lastIdUsed_{ 0 };

    bool Injector::Find(uint8_t const id, Injector::Storage_t*& dst) const
    {
        auto const predicate{
            [id](std::pair<const uint8_t, std::unique_ptr<Storage_t>> const& item){
            return item.second->GetId() == id;
        }};
        auto const it{ std::find_if(std::begin(serviceMap_), std::end(serviceMap_), predicate) };
        if (it == std::end(serviceMap_))
        {
            return false;
        }

        dst = it->second.get();
        return true;
    }
} // namespace MicroServiceDddLib
