#pragma once

#include <unordered_map>
#include <memory>
#include <mutex>

namespace MicroServiceDddLib
{

    enum class Error_t : uint8_t
    {
        NotEnoughResources
    };

    /** Represents a message passed from chain to chain.
     */
    class IMsg
    {
    public:
        virtual ~IMsg() = 0;

        virtual void Rollback() = 0;
        virtual std::unique_ptr<IMsg> Clone() = 0;
    };

    /** Represents a cell in a chain.
     */
    class ICell : std::enable_shared_from_this<ICell>
    {
    public:
        virtual ~ICell() = 0;

        /** Reads next message from this cell.
         * @throws std::logic_error.
         */
        virtual std::unique_ptr<IMsg> Read() = 0;
        
        /** Reads next message from this cell.
         */
        virtual Error_t TryRead(_Out_ std::unique_ptr<IMsg>& pMsg) = 0;

        /** Writes a message into this cell.
         * @throws std::logic_error.
         */
        virtual void Write(IMsg&& pMsg) = 0;

        /** Writes a message into this cell.
         */
        virtual Error_t TryWrite(_Inout_ std::unique_ptr<IMsg>&& pMsg) = 0;

        /** Connects this chain to @p pSrc (to read data from it).
         */
        virtual void ConnectTo(ICell& pSrc) = 0;


        /** Reports whether this cell is a passive generator.
         */
        virtual bool IsPassive() = 0;

    protected:
        /** Connects @p pDst as the destination of messages.
         */
        virtual void Connect(std::shared_ptr<ICell> pDst) = 0;
    };

    /** Injector as in the Dependency Injection pattern.
     */
    class Injector
    {
    protected:
        Injector() {}

        class Storage_t
        {
        public:
            virtual uint8_t GetId() const noexcept = 0;
            static uint8_t lastIdUsed_;
        };

        template<typename _T> class Storage : public Storage_t
        {
        public:
            Storage(std::shared_ptr<_T> pService) : val_{ pService }
            {
                if (!myId_)
                {
                    myId_ = ++lastIdUsed_;
                }
            }
            Storage() {}

            uint8_t GetId() const noexcept override { return myId_; }

            std::shared_ptr<_T> val_;
            static uint8_t myId_;
        };

        std::unordered_map<uint8_t, std::unique_ptr<Storage_t>> serviceMap_;
        std::mutex lock_;

        /** Internal function, allows us to avoid dragging to many system headers into here.
         */
        // TODO: eliminate need for <mutex>?
        bool Find(uint8_t const id, Storage_t*& pDst) const;

    public:
        static Injector& Instance() noexcept;

        template<typename _T> void Set(std::shared_ptr<_T>&& pService)
        {
            std::lock_guard<decltype(lock_)> const lock{ lock_ };

            auto ptr{ std::unique_ptr<Storage_t>{
                new Storage<_T>{ std::forward<std::shared_ptr<_T>>(pService) } } };
            auto const id{ ptr->GetId() };
            if (serviceMap_.find(id) != std::end(serviceMap_))
            {
                serviceMap_.erase(id);
            }

            serviceMap_.emplace(id, std::move(ptr));
        }

        template<typename _T> void Set(std::shared_ptr<_T> const& pService)
        {
            Set(std::shared_ptr<_T>{ pService });
        }

        template<typename _T> bool TryGet(std::shared_ptr<_T>& dst)
        {
            std::lock_guard<decltype(lock_)> const lock{ lock_ };

            dst.reset();

            Storage<_T> service;    // Dummy object to obtain myId_ of it.

            Storage_t* pStorage;
            if (!Find(service.GetId(), pStorage))
            {
                return false;
            }

            dst = static_cast<Storage<_T>*>(pStorage)->val_;
            return true;
        }

        template<typename _T> std::shared_ptr<_T> Get()
        {
            std::shared_ptr<_T> result;
            if (!TryGet(result))
            {
                throw std::logic_error{ "Service wasn't found" };
            }

            return result;
        }
    };

    template<typename _T> uint8_t Injector::Storage<_T>::myId_{ 0 };

} // namespace MicroServiceDddLib
