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
        size_t m_nMemABytes{};
        size_t m_nMemBBytes{};
        std::unique_ptr<uint8_t[]> m_upMemA;
        std::unique_ptr<uint8_t[]> m_upMemB;
        ZxCvt::LastError m_eError{};

    public:
        ZxCvt() {}

    private:
        template<class T>
        auto ReSize(size_t nBytes, bool isSlotB = false) -> T;
        template <class T>
        auto Ptr(bool isSlotB = false) const noexcept -> T;

    public:
        auto UTF16LEToMBCS(const std::u16string_view u16Str, const size_t nCodePage, bool isSlotB = false) -> std::string_view;
        auto MBCSToUTF16LE(const std::string_view msStr, const size_t nCodePage, bool isSlotB = false) -> std::u16string_view;
        auto MBCSToMBCS(const std::string_view msStr, const size_t nCodePageA, const size_t nCodePageB) -> std::string_view;

        auto UTF16LEToUTF8(const std::u16string_view u16Str) -> std::string_view;
        auto UTF8ToUTF16LE(const std::string_view u8Str) -> std::u16string_view;
        auto UTF8ToUTF16LE(const std::u8string_view u8Str) -> std::u16string_view;
        auto UTF8ToMBCS(const std::string_view u8Str, const size_t nCodePage) -> std::string_view;
        auto UTF8ToMBCS(const std::u8string_view u8Str, const size_t nCodePage) -> std::string_view;
        auto MBCSToUTF8(const std::string_view msStr, const size_t nCodePage) -> std::string_view;

    public:
        auto NotError() const -> bool;
        auto GetError() const->std::string_view;

    private:
        auto ErrorClear() -> void;
        auto ErrorSet(ZxCvt::LastError eError) -> void;

#ifdef __linux__
    private:
        auto IConvConv(const void* cpSrc, size_t nSrcBytes, size_t nSrcCodePage, size_t nDestCodePage, const size_t nDestEleSize, bool isSlotB = false) -> size_t;
#endif


    };

    template <class T>
    auto ZxCvt::Ptr(bool isSlotB) const noexcept -> T
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
    auto ZxCvt::ReSize(size_t nBytes, bool isSlotB) -> T
    {
        if (isSlotB)
        {
            if (m_nMemBBytes < nBytes)
            {
                m_upMemB = std::make_unique_for_overwrite<uint8_t[]>(nBytes);
                m_nMemBBytes = nBytes;
            }
        }
        else
        {
            if (m_nMemABytes < nBytes)
            {
                m_upMemA = std::make_unique_for_overwrite<uint8_t[]>(nBytes);
                m_nMemABytes = nBytes;
            }
        }

        return this->Ptr<T>(isSlotB);
    }

}
