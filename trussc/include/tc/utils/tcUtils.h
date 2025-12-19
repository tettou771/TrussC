#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <bitset>
#include <cstdint>

// tcPlatform.h の前方宣言（循環インクルード回避）
namespace trussc { namespace platform {
    std::string getExecutableDir();
}}

namespace trussc {

// ---------------------------------------------------------------------------
// データパス（oF の ofToDataPath と同様）
// ---------------------------------------------------------------------------

namespace internal {
    // デフォルトは "data/"（実行ファイルからの相対パス）
    inline std::string dataPathRoot = "data/";
    inline bool dataPathRootIsAbsolute = false;
}

// データパスのルートを設定
// 相対パスの場合、実行ファイルのディレクトリを基準に解決される
// 絶対パス（/ で始まる）の場合はそのまま使用
inline void setDataPathRoot(const std::string& path) {
    internal::dataPathRoot = path;
    // 末尾にスラッシュがなければ追加
    if (!internal::dataPathRoot.empty() && internal::dataPathRoot.back() != '/') {
        internal::dataPathRoot += '/';
    }
    // 絶対パスかどうかを記録
    internal::dataPathRootIsAbsolute = (!path.empty() && path[0] == '/');
}

// データパスのルートを取得
inline std::string getDataPathRoot() {
    return internal::dataPathRoot;
}

// ファイル名からデータパスを取得
// 実行ファイルのディレクトリを基準に解決される
inline std::string getDataPath(const std::string& filename) {
    if (internal::dataPathRootIsAbsolute) {
        // 絶対パスの場合はそのまま
        return internal::dataPathRoot + filename;
    } else {
        // 相対パスの場合は実行ファイルのディレクトリを基準に
        return platform::getExecutableDir() + internal::dataPathRoot + filename;
    }
}

// macOS バンドル配布用: Resources フォルダを data パスに設定
// xxx.app/Contents/Resources/data/ を参照するようになる
// macOS 以外では何もしない
inline void setDataPathToResources() {
    #ifdef __APPLE__
    setDataPathRoot("../Resources/data/");
    #endif
}

// ---------------------------------------------------------------------------
// toString - 値を文字列に変換
// ---------------------------------------------------------------------------

// 基本的な文字列変換
template <class T>
std::string toString(const T& value) {
    std::ostringstream out;
    out << value;
    return out.str();
}

// 小数点精度を指定して文字列に変換
// 例: toString(3.14159, 2) → "3.14"
template <class T>
std::string toString(const T& value, int precision) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(precision) << value;
    return out.str();
}

// 幅とフィル文字を指定して文字列に変換
// 例: toString(42, 5, '0') → "00042"
template <class T>
std::string toString(const T& value, int width, char fill) {
    std::ostringstream out;
    out << std::setfill(fill) << std::setw(width) << value;
    return out.str();
}

// 精度、幅、フィル文字を指定して文字列に変換
// 例: toString(3.14, 2, 6, '0') → "003.14"
template <class T>
std::string toString(const T& value, int precision, int width, char fill) {
    std::ostringstream out;
    out << std::fixed << std::setfill(fill) << std::setw(width) << std::setprecision(precision) << value;
    return out.str();
}

// vectorを文字列に変換
// 例: toString(vector{1,2,3}) → "{1, 2, 3}"
template <class T>
std::string toString(const std::vector<T>& values) {
    std::ostringstream out;
    out << "{";
    for (size_t i = 0; i < values.size(); i++) {
        if (i > 0) out << ", ";
        out << values[i];
    }
    out << "}";
    return out.str();
}

// ---------------------------------------------------------------------------
// 文字列から数値への変換
// ---------------------------------------------------------------------------

// 文字列を整数に変換
inline int toInt(const std::string& str) {
    try {
        return std::stoi(str);
    } catch (...) {
        return 0;
    }
}

// 文字列を64bit整数に変換
inline int64_t toInt64(const std::string& str) {
    try {
        return std::stoll(str);
    } catch (...) {
        return 0;
    }
}

// 文字列をfloatに変換
inline float toFloat(const std::string& str) {
    try {
        return std::stof(str);
    } catch (...) {
        return 0.0f;
    }
}

// 文字列をdoubleに変換
inline double toDouble(const std::string& str) {
    try {
        return std::stod(str);
    } catch (...) {
        return 0.0;
    }
}

// 文字列をboolに変換（"true", "TRUE", "1" → true）
inline bool toBool(const std::string& str) {
    if (str.empty()) return false;
    // 小文字に変換して比較
    std::string lower = str;
    for (auto& c : lower) c = std::tolower(c);
    return (lower == "true" || lower == "1" || lower == "yes");
}

// ---------------------------------------------------------------------------
// toHex - 16進数文字列に変換
// ---------------------------------------------------------------------------

// 数値を16進数文字列に変換
template <class T>
std::string toHex(const T& value) {
    std::ostringstream out;
    out << std::hex << std::uppercase << value;
    return out.str();
}

// 整数を16進数文字列に変換（ゼロパディング）
inline std::string toHex(int value, int width = 0) {
    std::ostringstream out;
    if (width > 0) {
        out << std::setfill('0') << std::setw(width);
    }
    out << std::hex << std::uppercase << value;
    return out.str();
}

// 文字列を16進数文字列に変換
inline std::string toHex(const std::string& value) {
    std::ostringstream out;
    for (unsigned char c : value) {
        out << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << (int)c;
    }
    return out.str();
}

// ---------------------------------------------------------------------------
// toBinary - 2進数文字列に変換
// ---------------------------------------------------------------------------

// 整数を2進数文字列に変換
inline std::string toBinary(int value) {
    return std::bitset<32>(value).to_string();
}

inline std::string toBinary(unsigned int value) {
    return std::bitset<32>(value).to_string();
}

inline std::string toBinary(char value) {
    return std::bitset<8>(value).to_string();
}

inline std::string toBinary(unsigned char value) {
    return std::bitset<8>(value).to_string();
}

// 文字列を2進数文字列に変換（各文字を8bitで表現）
inline std::string toBinary(const std::string& value) {
    std::string result;
    for (unsigned char c : value) {
        if (!result.empty()) result += " ";
        result += std::bitset<8>(c).to_string();
    }
    return result;
}

// ---------------------------------------------------------------------------
// fromHex - 16進数文字列から数値に変換
// ---------------------------------------------------------------------------

inline int hexToInt(const std::string& hexStr) {
    try {
        return std::stoi(hexStr, nullptr, 16);
    } catch (...) {
        return 0;
    }
}

inline unsigned int hexToUInt(const std::string& hexStr) {
    try {
        return static_cast<unsigned int>(std::stoul(hexStr, nullptr, 16));
    } catch (...) {
        return 0;
    }
}

// ---------------------------------------------------------------------------
// 文字列操作
// ---------------------------------------------------------------------------

/// 文字列内に別の文字列が含まれているか検索
/// oFの ofIsStringInString と同じ
inline bool isStringInString(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

/// 文字列内に別の文字列が何回出現するかカウント
/// oFの ofStringTimesInString と同じ
inline std::size_t stringTimesInString(const std::string& haystack, const std::string& needle) {
    const size_t step = needle.size();
    size_t count = 0;
    size_t pos = 0;
    while ((pos = haystack.find(needle, pos)) != std::string::npos) {
        pos += step;
        ++count;
    }
    return count;
}

/// 文字列を区切り文字で分割
/// oFの ofSplitString と同じ引数順
/// @param source 元の文字列
/// @param delimiter 区切り文字列
/// @param ignoreEmpty 空の要素を無視するか
/// @param trim 各要素の前後の空白を削除するか
inline std::vector<std::string> splitString(const std::string& source, const std::string& delimiter, bool ignoreEmpty = false, bool trim = false) {
    std::vector<std::string> result;
    if (delimiter.empty()) {
        result.push_back(source);
        return result;
    }
    std::string::const_iterator substart = source.begin(), subend;
    while (true) {
        subend = std::search(substart, source.end(), delimiter.begin(), delimiter.end());
        std::string sub(substart, subend);
        if (trim) {
            // 前後の空白を削除
            size_t start = sub.find_first_not_of(" \t\r\n");
            size_t end = sub.find_last_not_of(" \t\r\n");
            if (start != std::string::npos && end != std::string::npos) {
                sub = sub.substr(start, end - start + 1);
            } else if (start == std::string::npos) {
                sub.clear();
            }
        }
        if (!ignoreEmpty || !sub.empty()) {
            result.push_back(sub);
        }
        if (subend == source.end()) {
            break;
        }
        substart = subend + delimiter.size();
    }
    return result;
}

/// 文字列配列を区切り文字で結合
/// oFの ofJoinString と同じ
inline std::string joinString(const std::vector<std::string>& stringElements, const std::string& delimiter) {
    std::string str;
    if (stringElements.empty()) {
        return str;
    }
    auto numStrings = stringElements.size();
    std::string::size_type strSize = delimiter.size() * (numStrings - 1);
    for (const std::string& s : stringElements) {
        strSize += s.size();
    }
    str.reserve(strSize);
    str += stringElements[0];
    for (decltype(numStrings) i = 1; i < numStrings; ++i) {
        str += delimiter;
        str += stringElements[i];
    }
    return str;
}

/// 文字列内の検索文字列を置換（破壊的）
/// oFの ofStringReplace と同じ
inline void stringReplace(std::string& input, const std::string& searchStr, const std::string& replaceStr) {
    auto pos = input.find(searchStr);
    while (pos != std::string::npos) {
        input.replace(pos, searchStr.size(), replaceStr);
        pos += replaceStr.size();
        std::string nextfind(input.begin() + pos, input.end());
        auto nextpos = nextfind.find(searchStr);
        if (nextpos == std::string::npos) {
            break;
        }
        pos += nextpos;
    }
}

/// 文字列の前後の空白を削除
/// oFの ofTrim と同じ
inline std::string trim(const std::string& src) {
    size_t start = src.find_first_not_of(" \t\r\n");
    size_t end = src.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    return src.substr(start, end - start + 1);
}

/// 文字列の先頭の空白を削除
/// oFの ofTrimFront と同じ
inline std::string trimFront(const std::string& src) {
    size_t start = src.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    return src.substr(start);
}

/// 文字列の末尾の空白を削除
/// oFの ofTrimBack と同じ
inline std::string trimBack(const std::string& src) {
    size_t end = src.find_last_not_of(" \t\r\n");
    if (end == std::string::npos) {
        return "";
    }
    return src.substr(0, end + 1);
}

/// 文字列を小文字に変換
/// oFの ofToLower と同じ
inline std::string toLower(const std::string& src) {
    std::string dst = src;
    for (auto& c : dst) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return dst;
}

/// 文字列を大文字に変換
/// oFの ofToUpper と同じ
inline std::string toUpper(const std::string& src) {
    std::string dst = src;
    for (auto& c : dst) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return dst;
}

} // namespace trussc
