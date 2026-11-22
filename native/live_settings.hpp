#pragma once
#include <windows.h>
#include <filesystem>
#include <string>
#include <cmath>
#include "vendor/swofty-json.hpp"
#include "smoothing.hpp"

namespace wing {
// Read only the profile named by this session's startup settings. Never follow the
// launcher's subsequently selected profile, and never write launcher/game files.
class LiveSettings {
    std::filesystem::path path;
    std::string profile;
    double observed = default_ms;
    bool seeded = false;
    FILETIME modified{};
    DWORD size = 0;

    static std::filesystem::path utf8_path(const std::string& s) {
        return std::filesystem::path(std::u8string(s.begin(),s.end()));
    }

    static bool text(const std::filesystem::path& file, std::string& out) {
        HANDLE h=CreateFileW(file.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                             nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
        if (h==INVALID_HANDLE_VALUE) return false;
        LARGE_INTEGER n{};bool ok=GetFileSizeEx(h,&n) && n.QuadPart>0 && n.QuadPart<=2*1024*1024;
        if (ok) {
            out.resize(static_cast<size_t>(n.QuadPart));DWORD got{};
            ok=ReadFile(h,out.data(),static_cast<DWORD>(out.size()),&got,nullptr) && got==out.size();
        }
        CloseHandle(h);return ok;
    }
    bool read(double& ms) {
        std::string data;json::Value doc;
        if (!text(path,data) || !json::parse(data,doc) || doc["schema_version"].num()!=2 ||
            doc["name"].str()!=profile) return false;
        const auto& v=doc["options"]["unorthodoxstonendmodding.myteam-macarena-wing"]["travel_ms"];
        if (v.type==json::Value::Null) {ms=default_ms;return true;} // Reset to defaults.
        if (v.type!=json::Value::Num || !std::isfinite(v.n) || v.n<min_ms || v.n>max_ms ||
            std::fmod(v.n-min_ms,step_ms)!=0) return false;
        ms=v.n;return true;
    }
public:
    bool start(const std::filesystem::path& startup) {
        path.clear();seeded=false;modified={};size=0;
        std::string data;json::Value doc;
        if (!text(startup,data) || !json::parse(data,doc) || doc["schema"].num()!=1) return false;
        profile=doc["profile"].str();
        if (profile.empty() || profile=="." || profile==".." ||
            profile.find_first_of("/\\:")!=std::string::npos) return false;
        const auto request=utf8_path(doc["requests"].str());
        if (!request.is_absolute() || request.filename()!=utf8_path(profile+".json") ||
            request.parent_path().filename()!=L"requests" || request.parent_path().parent_path().filename()!=L"state") return false;
        path=request.parent_path().parent_path().parent_path()/L"profiles"/utf8_path(profile+".json");
        seeded=read(observed);
        return true;
    }
    bool poll(double& ms) {
        if (path.empty()) return false;
        WIN32_FILE_ATTRIBUTE_DATA attr{};
        if (!GetFileAttributesExW(path.c_str(),GetFileExInfoStandard,&attr) || attr.nFileSizeHigh) return false;
        if (seeded && CompareFileTime(&modified,&attr.ftLastWriteTime)==0 && size==attr.nFileSizeLow) return false;
        double current{};
        if (!read(current)) return false; // A partial/invalid write keeps the last good value and is retried.
        modified=attr.ftLastWriteTime;size=attr.nFileSizeLow;
        bool changed=!seeded || current!=observed;
        observed=current;seeded=true;ms=current;return changed;
    }
};
}
