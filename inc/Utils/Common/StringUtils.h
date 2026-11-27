//
// Created by Nikita on 17.11.2020.
//

#ifndef SR_ENGINE_COMMON_STRING_UTILS_H
#define SR_ENGINE_COMMON_STRING_UTILS_H

#include <Utils/Math/Vector3.h>
#include <Utils/Profile/TracyContext.h>
#include <Utils/Types/String.h>

namespace SR_UTILS_NS {
    class StringAtom;

    SR_COMMON_DLL_API String GetErrorString(int err);
    SR_COMMON_DLL_API std::wstring s2ws(StringView str);
    SR_COMMON_DLL_API String ws2s(const std::wstring& wstr);

    #define SR_S2WS(str) s2ws(str)
    #define SR_WS2S(wstr) ws2s(wstr)

    /// @noCopyable @noMovable
    class SR_COMMON_DLL_API StringUtils : public SR_UTILS_NS::Singleton<StringUtils>, public SRClass {
        SR_REGISTER_SINGLETON(StringUtils);
        SR_CLASS()
    public:
        /// @method @evaluate
        void Reverse(String& str);
        /// @method @evaluate
        void ToLower(String& str);
        /// @method @evaluate
        void ToUpper(String& str);
        /// @method @evaluate
        String Concat(const String& left, const String& right);
        /// @method @evaluate
        Path ConcatPath(const Path& left, const Path& right);
        /// @method @evaluate
        Path RemoveSubPath(const Path& path, const Path& subPath);
        /// @method @evaluate
        bool CompareAnyCase(StringView left, StringView right);

        /// @method @evaluate
        StringAtom MakeAtom(StringView str);

        /// @method @evaluate
        bool Compare(StringView left, StringView right);
        /// @method @evaluate
        StringView GetViewFromAtom(StringAtom atom);
        /// @method @evaluate
        StringView GetViewFromPath(const Path& path);
        /// @method @evaluate
        String PathToString(const Path& path);
        /// @method @evaluate
        String FloatToString(float_t value, uint32_t precision);
        /// @method @evaluate
        String IntToString(int64_t value);

        void Base64Decode(StringView source, String& base64);
        void Base64Encode(StringView base64, String& result);

        void ConvertToUTF8(const wchar_t* pSource, uint32_t size, String& result);
        void ConvertToWideString(const char* pSource, uint32_t size, std::wstring& result);

        void SplitViewByLines(StringView buffer, Vector<StringView>& outLines);
        void ReplaceChars(String& buffer, char from, char to);

        static bool CheckSearchMatch(StringView searchBuffer, StringView text);

        static String MakeDisplayName(StringView name);

        static SR_FORCE_INLINE StringView GetDirToFileFromFullPath(StringView path) {
            size_t size = path.size();

            while (size > 0){
                size--;
                if (path[size] == '\\' || path[size] == '/')
                    break;
            }

            path.remove_suffix(path.size() - size);

            return path;
        }

        static String RemoveCharsFromString(String source, StringView charsToRemove);

        static char* Strncat(char* dest, const char* src, size_t count) {
            char* d = dest;
            while (*d) {
                ++d;
            }
            while (count-- && *src) {
                *d++ = *src++;
            }
            *d = '\0';
            return dest;
        }

        static char* Strcpy(char* dest, const char* src) {
            char* d = dest;
            while (*src) {
                *d++ = *src++;
            }
            *d = '\0';
            return dest;
        }

        static String Remove(const String& source, uint64_t count);
        static String Remove(const String& source, uint64_t start, uint64_t count);

        static std::string_view GetBetween(std::string_view source, char begin, char end, int8_t* pResult = nullptr);

        static std::string_view SubstringView(const std::string_view& source, char symbol, uint32_t offset = 0);
        static std::string_view SubstringView(const std::string_view& source, StringAtom substr, uint32_t offset = 0);

        static String ToKebabCase(std::string_view str);

        inline static unsigned long FastStrLen(const char* str) noexcept {
            unsigned long len = 0;
            while (*str != '\0') {
                len++;
                str++;
            }
            return len;
        }

        SR_NODISCARD static Vector<std::string_view> SplitView(std::string_view source, std::string_view delimiter);
        SR_NODISCARD static Vector<String> Split(std::string_view source, std::string_view delimiter);

        void SplitView(StringView source, StringView delimiter, Vector<StringView>& out) const;

        SR_NODISCARD inline static char** Split(const char* source, char chr, unsigned short start, unsigned short count_strs) {
            char** strs = new char*[count_strs];
            unsigned char   found_floats = 0;

            unsigned short	count = 0;
            unsigned short	found = start;

            static short t = 0;
            static short len = 0;

            len = static_cast<short>(FastStrLen(source));

            for (t = static_cast<short>(start); t < len; t++) {
                count++;

                if (t + 1 == len) {
                    if (found_floats == count_strs)
                        return strs;

                    char* temp = new char[count + 1]; Strcpy(temp, "");

                    Strncat(temp, source + found, count);

                    strs[found_floats] = temp;

                    return strs;
                }
                else if (source[t] == chr) {
                    if (found_floats + 1 == count_strs)
                        return strs;

                    char* temp = new char[count + 1]; Strcpy(temp, "");

                    Strncat(temp, source + found, count - 1);

                    strs[found_floats] = temp;
                    found_floats++;

                    found = t + 1;
                    count = 0;
                }
            }
            return nullptr;
        }

        SR_NODISCARD inline static float* SplitFloats(const char* source, char chr, unsigned short start, unsigned short count_floats) {
            auto*			floats			= new float[count_floats];
            unsigned char   found_floats	= 0;

            unsigned short	count			= 0;
            unsigned short	found			= start;

            static short t = 0;
            static short len = 0;

            len = (short)FastStrLen(source);

            for (t = static_cast<short>(start); t < len; t++) {
                count++;

                if (t + 1 == len) {

                    if (found_floats == count_floats)
                        return floats;

                    char* temp = new char[count + 1]; Strcpy(temp, "");

                    Strncat(temp, source + found, count);
                    floats[found_floats] = std::strtof(temp, nullptr);
                    delete[] temp;

                    return floats;
                } else if (source[t] == chr) {
                    if (found_floats + 1 == count_floats)
                        return floats;

                    char* temp = new char[count + 1]; Strcpy(temp, "");

                    Strncat(temp, source + found, count - 1);

                    floats[found_floats] = std::strtof(temp, nullptr);
                    found_floats++;

                    delete[] temp;

                    found = t + 1;
                    count = 0;
                }
            }
            return nullptr;
        }

        SR_NODISCARD inline static unsigned char MathCount(const char* str, char symb) noexcept {
            unsigned char count = 0;
            while (*str != '\0') {
                if (*str == symb)
                    count++;
                str++;
            }
            return count;
        }

        template<typename ST1, typename ST2, typename ST3> SR_NODISCARD static ST1 ReplaceAll(ST1 const& original, ST2 const& from, ST3 const& to) noexcept {
            ST1 results;
            results.reserve(original.size());
            typename ST1::const_iterator end = original.end();
            typename ST1::const_iterator current = original.begin();
            typename ST1::const_iterator next = std::search(current, end, from.begin(), from.end());
            while (next != end) {
                results.append(current, next);
                results.append(to);
                current = next + from.size();
                next = std::search(current, end, from.begin(), from.end());
            }
            results.append(current, next);
            return results;
        }

        SR_NODISCARD String MakePath(StringView str, bool toLower = false) noexcept;

        // Cut string (file name) to output it
        SR_NODISCARD static String CutName(String str, uint32_t frompos){
            SR_TRACY_ZONE;
            if (str.size() > frompos){
                str = str.substr(0,frompos);
                str.append("..."); //должно быть "…", но utf-16 символы не поддерживаются в ImGui, вероятно можно выбрать набор с нужными глифами
            }
            return str;
        }
    };
}

#endif //SR_ENGINE_COMMON_STRING_UTILS_H
