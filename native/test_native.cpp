#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <swofty_api.h>
#include <safetyhook.hpp>
#include "smoothing.hpp"
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>
#include <filesystem>
#include <fstream>

template<class T> T symbol(HMODULE m, const char* n) {
    auto p=reinterpret_cast<T>(GetProcAddress(m,n)); assert(p); return p;
}
template<class T> void put(std::vector<unsigned char>& b, size_t at, T v) { std::memcpy(b.data()+at,&v,sizeof(v)); }
struct Car {
    std::vector<unsigned char> physics=std::vector<unsigned char>(0x2000,0xa5);
    std::vector<unsigned char> aero=std::vector<unsigned char>(0x1a08,0x5a);
    std::vector<unsigned char> model=std::vector<unsigned char>(0x1400,0x63);
    explicit Car(uint8_t id) {
        put(physics,0xe8,id); put(aero,8,reinterpret_cast<uintptr_t>(physics.data()));
        uint32_t bits{};const float angle=wing::marker_angle;std::memcpy(&bits,&angle,4);
        put(aero,0x1848,bits^0x12345678u); put(aero,0x1850,0x12345678u);put(model,0x1300,int32_t(2));
    }
    safetyhook::Context context(float fraction) {
        safetyhook::Context c;std::memset(&c,0x37,sizeof(c));
        c.r14=reinterpret_cast<uintptr_t>(aero.data());c.rdi=reinterpret_cast<uintptr_t>(model.data());
        c.xmm13.f32[0]=fraction;return c;
    }
};
void log(const char*,...) {}
SwoftySettingFn watcher{};
void* watcher_user{};
int initial_setting(const char* id,uint32_t type,SwoftySettingValue* out) {
    assert(!std::strcmp(id,"travel_ms") && type==SWOFTY_SETTING_SLIDER);
    out->type=type;out->slider=500;return 1;
}
int watch_setting(const char* id,SwoftySettingFn fn,void* user) {
    assert(!std::strcmp(id,"travel_ms"));watcher=fn;watcher_user=user;return 1;
}
SwoftyFrameFn frame_callback{};
void* frame_user{};
int on_frame(SwoftyFrameFn fn,void* user,uint32_t* token) {
    frame_callback=fn;frame_user=user;*token=7;return 1;
}

int main(int argc,char** argv) {
    assert(argc==3);
    HMODULE dll=LoadLibraryA(argv[1]);assert(dll);
    auto apply=symbol<void(*)(safetyhook::Context*,double)>(dll,"WingTestApply");
    auto reset=symbol<void(*)()>(dll,"WingTestReset");
    auto duration=symbol<void(*)(double)>(dll,"WingTestDuration");
    Car a(0),b(1);
    const auto physics=a.physics,aero=a.aero,model=a.model;
    auto run=[&](Car& car,float input,double time) {
        auto ctx=car.context(input);auto expected=ctx;
        apply(&ctx,time);float result=ctx.xmm13.f32[0];
        expected.xmm13.f32[0]=result;
        assert(!std::memcmp(&ctx,&expected,sizeof(ctx))); // All other register bits preserved.
        return result;
    };
    auto approx=[](float x,float y) {assert(std::fabs(x-y)<0.00001f);};
    approx(run(a,0,0),0);approx(run(a,1,0.1),0.4f);reset(); // Fresh DLL default: 250 ms.
    // Model the launcher's exported-API selection and real setting read/watch path.
    assert(*symbol<const uint32_t*>(dll,"SwoftyAddonApi")==SWOFTY_API_VERSION);
    SwoftyApi settings_api{};settings_api.version=SWOFTY_API_VERSION;settings_api.log=log;
    settings_api.setting=initial_setting;settings_api.watch_setting=watch_setting;
    symbol<void(*)(const SwoftyApi*)>(dll,"WingTestConfigureSettings")(&settings_api);
    assert(watcher);
    approx(run(a,0,0),0);approx(run(a,1,0.1),0.2f); // Initial 500 ms from the saved setting.
    SwoftySettingValue changed{};changed.type=SWOFTY_SETTING_SLIDER;changed.slider=150;
    watcher("travel_ms",&changed,watcher_user);
    reset();approx(run(a,0,0.2),0);approx(run(a,1,0.3),2.0f/3.0f); // Watch update reaches rendering.
    // Follow the real file -> on_frame callback -> rendering path, not a direct settings callback.
    const auto live_root=std::filesystem::current_path()/L"Live Slider \u03a9";
    std::filesystem::create_directories(live_root/L"profiles");
    std::filesystem::create_directories(live_root/L"state"/L"requests");
    const auto profile=live_root/L"profiles"/L"modded-new.json";
    const auto startup=live_root/L"startup.json";
    const auto request=(live_root/L"state"/L"requests"/L"modded-new.json").generic_u8string();
    auto write=[](const std::filesystem::path& path,const std::string& data) {
        const auto tmp=std::filesystem::path(path.wstring()+L".tmp");
        {std::ofstream f(tmp,std::ios::binary);f<<data;assert(f.good());}
        assert(MoveFileExW(tmp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING));
    };
    auto profile_text=[](const std::string& value) {
        return "{\"schema_version\":2,\"name\":\"modded-new\",\"options\":{\"unorthodoxstonendmodding.myteam-macarena-wing\":{\"travel_ms\":"+value+"}}}";
    };
    write(profile,profile_text("450"));
    write(startup,"{\"schema\":1,\"profile\":\"modded-new\",\"requests\":\""+
                  std::string(reinterpret_cast<const char*>(request.data()),request.size())+"\"}");
    SwoftyEvents events{};events.on_frame=on_frame;settings_api.events=&events;
    duration(450);
    symbol<void(*)(const wchar_t*)>(dll,"WingTestLiveSettings")(startup.c_str());assert(frame_callback);
    auto poll=[&] {Sleep(260);frame_callback(0,frame_user);}; // Paused game dt must still poll.
    auto timing=[&](float expected) {reset();approx(run(a,0,0),0);approx(run(a,1,0.1),expected);};
    write(profile,profile_text("500"));poll();timing(0.2f);
    write(profile,profile_text("150"));poll();
    approx(run(a,1,0.2),0.2f+2.0f/3.0f); // Changing speed mid-travel does not reset/snap position.
    timing(2.0f/3.0f);
    write(profile,"{\"schema_version\":2,");poll();timing(2.0f/3.0f); // Partial save keeps last good value.
    write(profile,profile_text("\"1000\""));poll();timing(2.0f/3.0f); // Invalid setting type.
    write(profile,profile_text("550"));poll();timing(2.0f/3.0f); // Invalid range.
    write(profile,profile_text("175"));poll();timing(2.0f/3.0f); // Invalid grid.
    write(profile,profile_text("500"));poll();timing(0.2f); // Recovers after malformed writes.
    write(live_root/L"profiles"/L"another.json",profile_text("150"));poll();timing(0.2f);
    write(profile,"{\"schema_version\":2,\"name\":\"modded-new\",\"options\":{}}");poll();timing(0.4f);
    write(profile,profile_text("0"));poll();reset();approx(run(a,0,0),0);approx(run(a,1,0.001),1);approx(run(a,0,0.002),0);
    write(profile,profile_text("50"));poll();reset();approx(run(a,0,0),0);approx(run(a,1,0.025),0.5f);
    write(profile,"{\"schema_version\":2,\"name\":\"modded-new\",\"options\":{}}");poll();timing(0.4f);
    std::ifstream saved(profile,std::ios::binary);
    const std::string unchanged((std::istreambuf_iterator<char>(saved)),std::istreambuf_iterator<char>());
    assert(unchanged=="{\"schema_version\":2,\"name\":\"modded-new\",\"options\":{}}");
    saved.close();
    write(profile,profile_text("1000")); // An old saved setting can exceed the new range.
    symbol<void(*)(const wchar_t*)>(dll,"WingTestLiveSettings")(startup.c_str());
    write(profile,profile_text("500"));poll();timing(0.2f); // First valid edit must apply, even after invalid startup data.
    duration(450);reset();
    approx(run(a,0,1),0);
    approx(run(a,1,1.1),1.0f/4.5f);
    approx(run(a,1,1.2),2.0f/4.5f);
    approx(run(a,1,1.3),3.0f/4.5f);
    approx(run(a,1,1.4),4.0f/4.5f);
    approx(run(a,1,1.45),1);
    approx(run(a,1,1.45),1); // Duplicate render/mirror pass cannot advance twice.
    approx(run(a,0,1.55),1-1.0f/4.5f);
    approx(run(a,0,1.65),1-2.0f/4.5f);
    approx(run(a,0,1.75),1-3.0f/4.5f);
    approx(run(a,0,1.85),1-4.0f/4.5f);
    approx(run(a,0,1.90),0);
    approx(run(b,0,1.55),0); // Teammate has separate state.
    approx(run(b,1,1.65),1.0f/4.5f);
    reset();approx(run(a,0,1.6),0); // New session.
    approx(run(a,1,3),1); // Long render gap seeds from current state.
    approx(run(a,0,2),0); // Backwards clock.
    duration(500);reset();approx(run(a,0,4),0);approx(run(a,1,4.1),0.2f);
    duration(450);
    put(b.aero,0x1848,uint32_t(0));approx(run(b,1,4.1),1); // Other angles bypass smoothing.
    auto bad=a.context(1);bad.r14=1;apply(&bad,4.1);approx(bad.xmm13.f32[0],1);
    assert(a.physics==physics && a.aero==aero && a.model==model);

    // Exercise the actual SafetyHook register handoff with a synthetic model-update instruction site.
    const unsigned char code[]={0x57,0x41,0x56,0x53,0x48,0x83,0xec,0x20,
        0xc5,0x78,0x29,0x2c,0x24,0x48,0x89,0xcf,0x49,0x89,0xd6,0xc5,0x78,0x28,0xea,
        0x41,0x8b,0x9e,0x48,0x18,0,0,0x41,0x33,0x9e,0x50,0x18,0,0,
        0xf3,0x41,0x0f,0x10,0xc5,0xc5,0x78,0x28,0x2c,0x24,0x48,0x83,0xc4,0x20,0x5b,0x41,0x5e,0x5f,0xc3};
    auto memory=static_cast<unsigned char*>(VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE));assert(memory);
    std::memcpy(memory,code,sizeof(code));FlushInstructionCache(GetCurrentProcess(),memory,sizeof(code));
    using Fn=float(*)(void*,void*,float);auto fn=reinterpret_cast<Fn>(memory);
    approx(fn(a.model.data(),a.aero.data(),1),1);
    auto install=symbol<int(*)(uintptr_t)>(dll,"WingTestHook");auto unhook=symbol<void(*)()>(dll,"WingTestUnhook");
    reset();assert(install(reinterpret_cast<uintptr_t>(memory+23)));
    approx(fn(a.model.data(),a.aero.data(),0),0);
    Sleep(60);
    const float rendered=fn(a.model.data(),a.aero.data(),1);
    assert(rendered>0.05f && rendered<0.9f);
    assert(a.physics==physics && a.aero==aero && a.model==model);
    unhook();approx(fn(a.model.data(),a.aero.data(),1),1);
    assert(!std::memcmp(memory,code,sizeof(code)));VirtualFree(memory,0,MEM_RELEASE);
    FreeLibrary(dll);

    HMODULE release=LoadLibraryA(argv[2]);assert(release);
    assert(*symbol<const uint32_t*>(release,"SwoftyAddonApi")==SWOFTY_API_VERSION);
    assert(!GetProcAddress(release,"WingTestConfigureSettings"));
    assert(!GetProcAddress(release,"WingTestLiveSettings"));
    assert(!GetProcAddress(release,"WingTestHook"));assert(!GetProcAddress(release,"WingTestApply"));
    SwoftyApi api{};api.version=SWOFTY_API_VERSION;api.log=log;
    assert(symbol<int(*)(const SwoftyApi*)>(release,"SwoftyAddonInit")(&api)==1); // Unsupported host image: no hook.
    FreeLibrary(release);
    std::puts("PASS: release/fixture API v9; saved and watched settings; launcher-style atomic profile saves reach rendering through actual on_frame callback at 0/50/150/500 ms; fresh default and reset 250 ms, zero-duration bypass without division, zero paused dt, Unicode path, malformed/type/range/grid rejection and recovery, pinned profile, reset-to-default and read-only files; 450 ms opening/closing, independent cars, duplicate passes, session reset, gaps, non-marker/invalid-pointer bypass; only xmm13.low changes, simulation/model buffers unchanged; real mid-hook/rollback; release has no fixture bypass and refuses unsupported host.");
}
