#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#include <swofty_api.h>
#include "smoothing.hpp"
#include "live_settings.hpp"
#include <array>
#include <atomic>
#include <cstring>
#include <mutex>

namespace {
const SwoftyApi* api{};
std::mutex state_mutex;
std::array<wing::State, wing::slots> states{};
std::atomic<uint64_t> epoch{1};
std::atomic<double> duration{wing::default_ms / 1000};
std::atomic<bool> reported{false};
uint32_t event_tokens[2]{};
uint32_t live_token{};
wing::LiveSettings live_settings;
uint64_t last_poll{};
constexpr char binding_name[] = "site_Car_RearWingRenderFraction";
constexpr unsigned char site_bytes[] = {0x41,0x8b,0x9e,0x48,0x18,0,0,0x41,0x33,0x9e,0x50,0x18,0,0};

template<class T> bool read(uintptr_t p, T& out) {
    if (p < 0x10000 || p > UINTPTR_MAX - sizeof(T)) return false;
    __try { std::memcpy(&out, reinterpret_cast<const void*>(p), sizeof(T)); return true; }
    __except(EXCEPTION_EXECUTE_HANDLER) { return false; }
}

double seconds() {
    static const double frequency = [] { LARGE_INTEGER x{}; QueryPerformanceFrequency(&x); return double(x.QuadPart); }();
    LARGE_INTEGER x{}; QueryPerformanceCounter(&x);
    return double(x.QuadPart) / frequency;
}

void apply(SwoftyContext& ctx, double now) {
    // This site is AFTER all front-wing matrix updates and BEFORE both rear-wing updates.
    // r14 = F1Aero; xmm13.low = rear render fraction; rdi = model owner.
    // Scope to the marker angle supplied ONLY by this add-on's five My Team records.
    uint32_t encoded{}, key{};
    uintptr_t car{};
    uint8_t vehicle{};
    int32_t node{};
    if (!read(ctx.r14 + 0x1848, encoded) || !read(ctx.r14 + 0x1850, key) ||
        !read(ctx.r14 + 8, car) || !read(car + 0xe8, vehicle) || vehicle >= wing::slots ||
        !read(ctx.rdi + 0x1300, node) || node < 0) return;
    const uint32_t bits = encoded ^ key;
    float angle{}; std::memcpy(&angle, &bits, sizeof(angle));
    if (angle != wing::marker_angle) return;
    const float input = ctx.xmm13.f32[0];
    if (!std::isfinite(input) || input < 0 || input > 1) return;
    {
        std::lock_guard lock(state_mutex);
        ctx.xmm13.f32[0] = wing::advance(states[vehicle], ctx.rdi, ctx.r14, epoch.load(), now,
                                         input, duration.load());
    }
    if (!reported.exchange(true) && api && api->log)
        api->log("[myteam-wing] rear render smoothing active; full travel %.0f ms; physics memory untouched", duration.load()*1000);
}

void render(SwoftyContext* ctx) { if (ctx) apply(*ctx, seconds()); }
void loaded() {
    api->log("[myteam-wing] Swofty rear-wing render hook installed; %.0f ms full travel", duration.load()*1000);
}
void session_event(const SwoftyEvent*, void*) { epoch.fetch_add(1); }
void setting(const char*, const SwoftySettingValue* value, void*) {
    if (value && value->type == SWOFTY_SETTING_SLIDER && std::isfinite(value->slider)) {
        duration.store(std::clamp(value->slider, wing::min_ms, wing::max_ms) / 1000.0);
        if (api && api->log) api->log("[myteam-wing] visual wing travel set to %.0f ms", duration.load()*1000);
    }
}

void configure_settings() {
    bool supplied = false;
    if (api->setting) {
        SwoftySettingValue value{};
        if (api->setting("travel_ms", SWOFTY_SETTING_SLIDER, &value)) {
            setting(nullptr, &value, nullptr);
            supplied = value.type == SWOFTY_SETTING_SLIDER && std::isfinite(value.slider);
        }
    }
    if (!supplied) api->log("[myteam-wing] travel_ms unavailable; using %.0f ms", duration.load()*1000);
    const bool watching = api->watch_setting && api->watch_setting("travel_ms", setting, nullptr);
    api->log("[myteam-wing] travel_ms setting watcher %s", watching ? "registered" : "unavailable");
}

void live_frame(double,void*) {
    const uint64_t now=GetTickCount64();
    if (now-last_poll<250) return;
    last_poll=now;
    double ms{};
    if (live_settings.poll(ms)) {
        SwoftySettingValue value{};value.type=SWOFTY_SETTING_SLIDER;value.slider=ms;
        setting(nullptr,&value,nullptr);
        api->log("[myteam-wing] live launcher slider applied: %.0f ms",ms);
    }
}
void start_live(const std::filesystem::path& startup) {
    try {
        if (live_settings.start(startup) && api->events && api->events->on_frame &&
            api->events->on_frame(live_frame,nullptr,&live_token) == SWOFTY_ASSET_OK) {
            api->log("[myteam-wing] live launcher slider enabled; session profile checked every 250 ms");
            return;
        }
    } catch (const std::filesystem::filesystem_error&) {
        // Invalid startup path encoding must not prevent the original animation from loading.
    }
    api->log("[myteam-wing] live launcher slider unavailable; startup value and in-game watcher retained");
}

bool supported_site(uintptr_t site) {
    std::array<unsigned char,sizeof(site_bytes)> bytes{};
    return read(site,bytes) && !std::memcmp(bytes.data(),site_bytes,sizeof(site_bytes));
}

bool verified_image(uintptr_t base, uintptr_t site) {
    // Exact Steam build 26566308. Do not try this instruction site on another executable.
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return false;
    const auto* pe = reinterpret_cast<const IMAGE_NT_HEADERS64*>(base + dos->e_lfanew);
    if (pe->Signature != IMAGE_NT_SIGNATURE || pe->FileHeader.TimeDateStamp != 1794312636) return false;
    wchar_t path[32768]{};
    if (!GetModuleFileNameW(nullptr, path, 32768)) return false;
    HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                              nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    LARGE_INTEGER size{};
    if (!GetFileSizeEx(file, &size) || size.QuadPart != 391613312) { CloseHandle(file); return false; }
    BCRYPT_ALG_HANDLE algorithm{}; BCRYPT_HASH_HANDLE hash{};
    bool ok = BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0) >= 0;
    if (ok) ok = BCryptCreateHash(algorithm, &hash, nullptr, 0, nullptr, 0, 0) >= 0;
    std::array<unsigned char, 65536> block{};
    DWORD got{};
    while (ok) {
        if (!ReadFile(file, block.data(), static_cast<DWORD>(block.size()), &got, nullptr)) { ok=false; break; }
        if (!got) break;
        ok = BCryptHashData(hash, block.data(), got, 0) >= 0;
    }
    unsigned char digest[32]{};
    if (ok) ok = BCryptFinishHash(hash, digest, sizeof(digest), 0) >= 0;
    if (hash) BCryptDestroyHash(hash);
    if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0);
    CloseHandle(file);
    constexpr unsigned char expected[] = {0x46,0x00,0x9b,0xd3,0x1f,0x49,0xd9,0x3a,0x92,0xd7,0x3c,0xbd,0x0b,0x66,0x13,0x1d,0x2b,0x37,0x96,0x4a,0x92,0xda,0x72,0x93,0x16,0x8d,0x3c,0x3a,0x87,0x2d,0x93,0x89};
    return ok && !std::memcmp(digest, expected, sizeof(digest)) &&
           supported_site(site);
}
}

// Without this export the launcher passes its legacy API v1 table, which this DLL rejects.
extern "C" __declspec(dllexport) const uint32_t SwoftyAddonApi = SWOFTY_API_VERSION;
extern "C" __declspec(dllexport) int SwoftyAddonInit(const SwoftyApi* a) {
    if (!a || a->version < 3 || !a->log || !a->resolve || !a->register_mixins) return 0;
    api = a;
    const uintptr_t site=a->resolve(binding_name);
    if (!site) {
        a->log("[myteam-wing] required Swofty binding site_Car_RearWingRenderFraction unavailable");
        return 0;
    }
#ifndef WING_TEST
    if (!verified_image(reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr)),site)) {
        a->log("[myteam-wing] binding resolved but executable/layout or site bytes are unsupported; not registered");
        return 0;
    }
#else
    // Host fixture only: bypass the executable identity, but still check the named site's bytes.
    if (!supported_site(site)) return 0;
#endif
    static const SwoftyMixin mixins[]={
        {binding_name,SWOFTY_MIXIN_SITE,0,reinterpret_cast<void*>(&render),nullptr},
        {nullptr,SWOFTY_MIXIN_ON_LOAD,0,reinterpret_cast<void*>(&loaded),nullptr}
    };
    if (!a->register_mixins(mixins,2)) return 0;
    configure_settings();
    if (a->events && a->events->subscribe) {
        a->events->subscribe("session.start", session_event, nullptr, &event_tokens[0]);
        a->events->subscribe("session.unload", session_event, nullptr, &event_tokens[1]);
    }
    wchar_t exe[32768]{};
    if (GetModuleFileNameW(nullptr,exe,32768))
        start_live(std::filesystem::path(exe).parent_path()/L"swofty"/L"settings.json");
    a->log("[myteam-wing] registered Swofty-managed rear-wing render hook for Steam 26566308; %.0f ms full travel", duration.load()*1000);
    return 1;
}

#ifdef WING_TEST
// Fixture-only exports. These are absent from the distributed DLL.
extern "C" __declspec(dllexport) void WingTestApply(SwoftyContext* ctx, double now) { apply(*ctx, now); }
extern "C" __declspec(dllexport) void WingTestReset() { epoch.fetch_add(1); }
extern "C" __declspec(dllexport) void WingTestDuration(double ms) { SwoftySettingValue v{}; v.type=2; v.slider=ms; setting(nullptr,&v,nullptr); }
extern "C" __declspec(dllexport) void WingTestConfigureSettings(const SwoftyApi* a) { api=a; configure_settings(); }
extern "C" __declspec(dllexport) void WingTestLiveSettings(const wchar_t* startup) { last_poll=0;start_live(startup); }
#endif
