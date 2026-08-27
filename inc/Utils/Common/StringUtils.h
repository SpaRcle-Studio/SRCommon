//
// Created by Nikita on 17.11.2020.
//

#ifndef SR_ENGINE_COMMON_STRING_UTILS_H
#define SR_ENGINE_COMMON_STRING_UTILS_H

#include <Utils/Math/Mathematics.h>
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
        StringAtom MakeAtom(StringView str);

        void Base64Decode(StringView source, String& base64);
        void Base64Encode(StringView base64, String& result);

        static bool CheckSearchMatch(StringView searchBuffer, StringView text);

        static String MakeDisplayName(StringView name);

        inline static glm::vec3 IntToColor(size_t index) noexcept {
            unsigned char r = ((index >> 16) & 0xFF);  // Extract the RR byte
            unsigned char g = ((index >> 8) & 0xFF);   // Extract the GG byte
            unsigned char b = ((index) & 0xFF);        // Extract the BB byte

            return { r, g, b };
        }

        static SR_FORCE_INLINE std::string GetDirToFileFromFullPath(std::string full_path) {
            size_t size = full_path.size();

            while (size > 0){
                size--;
                if (full_path[size] == '\\' || full_path[size] == '/')
                    break;
            }

            full_path.resize(size);

            return full_path;
        }

        static SR_FORCE_INLINE std::string MultiConcat(const std::string& target, int32_t n) {
            std::string out;
            for (auto i = 0; i < n; i++) {
                out += target;
            }

            return out;
        }

        static SR_FORCE_INLINE std::string GetDirFromPath(const std::string &path) {
            return path.substr(0, path.find_last_of("/\\"));
        }

        static SR_FORCE_INLINE int32_t IndexOf(const std::string& str, const char& symbol, uint32_t offset = 0) {
            for (uint32_t i = offset; i < str.size(); i++)
                if (str[i] == symbol)
                    return (int32_t)i;
            return SR_INVALID_STR_POS;
        }

        static SR_FORCE_INLINE int32_t IndexOf(const std::string& str, const std::string& subStr, uint32_t offset = 0) {
            auto&& index = str.find(subStr, offset);
            if (index == std::string::npos) {
                return SR_INVALID_STR_POS;
            }
            return static_cast<int32_t>(index);
        }

        static SR_FORCE_INLINE std::string GetFileNameFromFullPath(std::string full_path) {
            size_t size = full_path.size();

            while (size > 0){
                size--;
                if (full_path[size] == '\\' || full_path[size] == '/'){
                    full_path = full_path.erase(0, size + 1);
                    break;
                }
            }

            return full_path;
        }

        static std::string RemoveCharsFromString(std::string source, const std::string& charsToRemove) {
            for (char i : charsToRemove) {
                source.erase(remove(source.begin(), source.end(), i), source.end());
            }
            return source;
        }

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

        static std::string Remove(const String& source, uint64_t count);
        static std::string Remove(const String& source, uint64_t start, uint64_t count);

        static std::string GetBetween(const std::string& source, int64_t begin, uint64_t end);
        static std::string_view GetBetween(std::string_view source, char begin, char end, int8_t* pResult = nullptr);
        static std::string GetBetween(const std::string& source, const std::string& begin, const std::string& end);

        static std::string Tab(std::string code, uint32_t count = 1);

        static std::string Substring(const std::string& source, char symbol, uint32_t offset = 0) {
            if (auto&& pos = source.find(symbol); pos == std::string::npos)
                return source;
            else
                return source.substr(pos + offset, source.size() - 1);
        }

        static std::string Substring(const std::string& source, const char* subStr) {
            if (auto&& pos = source.find(subStr); pos == std::string::npos)
                return source;
            else {
                return source.substr(pos, source.size() - 1);
            }
        }

        static std::string Substring(const std::string_view& source, char symbol, uint32_t offset = 0) {
            if (auto&& pos = source.find(symbol); pos == std::string::npos)
                return std::string(source);
            else {
                return std::string(source.substr(pos + offset, source.size() - 1));
            }
        }

        static std::string_view SubstringView(const std::string_view& source, char symbol, uint32_t offset = 0);
        static std::string_view SubstringView(const std::string_view& source, StringAtom substr, uint32_t offset = 0);

        static SR_FORCE_INLINE std::string BackRead(const std::string& str, const char c, const int offset = 0) {
            std::string result = std::string();

            size_t size = str.size();

            for (int64_t i = (static_cast<int64_t>(size) - 1) + offset; i >= 0; --i) {
                if (str[i] != c)
                    result += str[i];
                else
                    break;
            }

            std::reverse(result.begin(), result.end());

            return result;
        }

        static std::string ReadFrom(const std::string& str, const char& c, uint32_t start);

        static std::string ReadNumber(const std::string& str, uint32_t start);

        static int32_t FindClosest(const std::string& str, const std::string& characters) {
            int32_t pos = SR_INVALID_STR_POS;
            for (char c : characters) {
                auto find = str.find(c);
                if (find != std::string::npos && (pos == -1 || find < pos)) {
                    pos = static_cast<int32_t>(find);
                }
            }
            return pos;
        }

        static inline std::string ReadTo(std::string str, const char c, int offset = 0) {
            size_t size = str.size();
            for (size_t t = 0; t < size; t++){
                if (str[t] == c){
                    str.resize(t + offset + 1);
                    break;
                }
            }
            return str;
        }

        static inline std::string Resize(std::string str, size_t newSize) noexcept {
            str.resize(newSize);
            return str;
        }

        static inline std::string BackSubstring(const std::string& str, char c){
            std::string result;
            for (size_t t = str.size() - 1; t > 0; t--){
                if (str[t] == c)
                    return result;
                else
                    result += str[t];
            }
            return std::string();
        }

        static std::string ToKebabCase(std::string_view str);

        inline static unsigned long FastStrLen(const char* str) noexcept {
            unsigned long len = 0;
            while (*str != '\0') {
                len++;
                str++;
            }
            return len;
        }

        SR_NODISCARD static std::pair<std::string, std::string> SplitTwo(std::string source, const std::string& delimiter);

        SR_NODISCARD static std::vector<std::string_view> SplitView(std::string_view source, std::string_view delimiter);
        SR_NODISCARD static std::vector<std::string_view> SplitViewWithEmpty(std::string_view source, std::string_view delimiter);
        SR_NODISCARD static std::vector<std::string> Split(std::string source, const std::string& delimiter);

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

        SR_NODISCARD inline static bool Contains(const std::string& str, const std::string& word) noexcept {
            return str.find(word) != std::string::npos;
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

        SR_NODISCARD static std::string ReplaceAllRecursive(const std::string& original, const std::vector<std::string>& fromList, const std::string& to) noexcept;

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

        SR_NODISCARD inline static std::string FromCharVector(const std::vector<char>& vs) noexcept {
            std::string result(begin(vs), end(vs));
            return result;
        }

        // Cut string (file name) to output it
        SR_NODISCARD static std::string CutName(std::string str, unsigned int frompos){
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
