#pragma once
#include <memory>
#include <string_view>


namespace ZQF
{
    class ZxCvt
    {
    private:
        enum LastError
        {
            NOT_ERROR,
            ERROR_NOT_ALL_CVT,
            ERROR_OUT_OF_MEMORY,
            ERROR_INVALID_ENCODING,
            ERROR_CVT_FAILED,
        };

    private:
        std::size_t m_nMemABytes{};
        std::unique_ptr<std::uint8_t[]> m_upMemA;
        std::size_t m_nMemBBytes{};
        std::unique_ptr<std::uint8_t[]> m_upMemB;
        ZxCvt::LastError m_eError{};
        bool m_isPrintError{ true };

    public:
        ZxCvt() {}

    private:
        template <class T> auto ReSize(const std::size_t nBytes, const bool isSlotB = false) -> T;
        template <class T> auto Ptr(const bool isSlotB = false) const noexcept -> T;

    public:
        auto UTF16LEToMBCS(const std::u16string_view u16Str, const std::size_t nCodePage, const bool isSlotB = false) -> std::string_view;
        auto MBCSToUTF16LE(const std::string_view msStr, const std::size_t nCodePage, const bool isSlotB = false) -> std::u16string_view;
        auto MBCSToMBCS(const std::string_view msStr, const std::size_t nCodePageA, const std::size_t nCodePageB) -> std::string_view;

        auto UTF16LEToUTF8(const std::u16string_view u16Str) -> std::string_view;
        auto UTF8ToUTF16LE(const std::string_view u8Str) -> std::u16string_view;
        auto UTF8ToUTF16LE(const std::u8string_view u8Str) -> std::u16string_view;
        auto UTF8ToMBCS(const std::string_view u8Str, const std::size_t nCodePage) -> std::string_view;
        auto UTF8ToMBCS(const std::u8string_view u8Str, const std::size_t nCodePage) -> std::string_view;
        auto MBCSToUTF8(const std::string_view msStr, const std::size_t nCodePage) -> std::string_view;

    public:
        auto GetErrorStatus() const -> bool;
        auto GetLastErrorAsStr() const -> std::string_view;
        auto GetPrintErrorStatus() const -> bool;
        auto SetPrintErrorStatus(const bool isStatus) -> void;

    private:
        auto ClsError() -> void;
        auto SetError(const ZxCvt::LastError eError) -> void;

#ifdef __linux__
    private:
        auto IConvConv(const void* pSrc, const std::size_t nSrcBytes, const std::size_t nSrcCodePage, const std::size_t nDestCodePage, const std::size_t nDestEleSize, const bool isSlotB = false) -> size_t;
#endif
    };

    template <class T>
    auto ZxCvt::Ptr(const bool isSlotB) const noexcept -> T
    {
        if constexpr (std::is_pointer_v<T>)
        {
            return isSlotB ? reinterpret_cast<T>(m_upMemB.get()) : reinterpret_cast<T>(m_upMemA.get());
        }
        else
        {
            static_assert(false, "ZxCvt::Ptr<T>(): not pointer type!");
        }
    }

    template<class T>
    auto ZxCvt::ReSize(const std::size_t nBytes, const bool isSlotB) -> T
    {
        if (isSlotB)
        {
            if (m_nMemBBytes < nBytes)
            {
                m_upMemB = std::make_unique_for_overwrite<std::uint8_t[]>(nBytes);
                m_nMemBBytes = nBytes;
            }
        }
        else
        {
            if (m_nMemABytes < nBytes)
            {
                m_upMemA = std::make_unique_for_overwrite<std::uint8_t[]>(nBytes);
                m_nMemABytes = nBytes;
            }
        }

        return this->Ptr<T>(isSlotB);
    }

}
