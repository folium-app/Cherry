//
//  bridge.cpp
//  Cherry
//
//  Created by Jarrod Norwell on 2/7/2026.
//

#include "bridge.h"
#include "mesence.h"

#include "Shared/EmuSettings.h"
#include "Shared/MessageManager.h"
#include "Utilities/FolderUtilities.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <thread>

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "Cherry-Swift.h"
using namespace Cherry;

struct cntnr_c {
    CherryCommon cherryCommon{CherryCommon::init()};
    CherrySystem cherrySystem{CherrySystem::init()};
    
    std::unique_ptr<Emulator> emulator;
    std::unique_ptr<CVInput> input;
    std::unique_ptr<iOSRenderer> renderer;
    std::unique_ptr<iOSSink> sink;
    
    CvConfig config;
    
    std::condition_variable_any cv;
    std::mutex mutex;
    std::atomic<bool> paused, running;
    std::jthread thread;
    
    uint32_t height, width;
    
    std::filesystem::path cherry_path, debugger_path, firmware_path;
    std::filesystem::path hd_packs_path, recent_games_path, saves_path;
    std::filesystem::path save_states_path, screenshots_path, system_data_path;
} cntnr_c;

void cherry::print_about(void) {
    printf("Welcome to Cherry\n");
    printf("ColecoVision emulation provided by MesenCE\n");
}

void cherry::initialize_paths(void) {
    auto cherryDirectoryURL{cntnr_c.cherryCommon.getCherryDirectoryURL()};
    if (cherryDirectoryURL.isSome()) {
        auto cherry_path{std::filesystem::path{cherryDirectoryURL.get()}};
        
        cntnr_c.cherry_path = cherry_path;
        cntnr_c.debugger_path = cherry_path / "debugger";
        cntnr_c.firmware_path = cherry_path / "firmware";
        cntnr_c.hd_packs_path = cherry_path / "hd_packs";
        cntnr_c.recent_games_path = cherry_path / "recent_games";
        cntnr_c.saves_path = cherry_path / "saves";
        cntnr_c.save_states_path = cherry_path / "save_states";
        cntnr_c.screenshots_path = cherry_path / "screenshots";
        cntnr_c.system_data_path = cherry_path / "system_data";
    }
}

void cherry::initialize_system(void) {
    auto mm{std::make_unique<iOSMessageManager>()};
    MessageManager::SetOptions(false, true);
    MessageManager::RegisterMessageManager(mm.get());
    
    cntnr_c.emulator = std::make_unique<Emulator>();
    cntnr_c.emulator->Initialize(false);
    
    cntnr_c.input = std::make_unique<CVInput>();
    cntnr_c.renderer = std::make_unique<iOSRenderer>(cntnr_c.emulator, 144, 160);
    cntnr_c.sink = std::make_unique<iOSSink>(cntnr_c.emulator, 48000);
    
    cntnr_c.config = cntnr_c.emulator->GetSettings()->GetCvConfig();
    for (int i = 0; i < 3; i++)
        cntnr_c.config.ChannelVolumes[i] = 100;
    cntnr_c.config.Port1.Type = ControllerType::ColecoVisionController;
    cntnr_c.config.Port2.Type = ControllerType::ColecoVisionController;
    cntnr_c.emulator->GetSettings()->SetCvConfig(cntnr_c.config);
}


void cherry::destroy_system(void) {
    cherry::initialize_system();
}


void cherry::insert_disc(std::string path) {
    FolderUtilities::SetHomeFolder(cntnr_c.cherry_path.string());
    FolderUtilities::SetFolderOverrides({}, {}, {}, cntnr_c.system_data_path);
    
    cntnr_c.emulator->LoadRom({path}, {});
    cntnr_c.emulator->RegisterInputProvider(cntnr_c.input.get());
}


bool cherry::is_paused(bool change, bool set_paused) {
    if (change)
        cntnr_c.paused.store(set_paused);
    
    if (change)
        set_paused ? cntnr_c.emulator->Pause() : cntnr_c.emulator->Resume();
    
    if (change && !set_paused)
        cntnr_c.cv.notify_one();
    
    return cntnr_c.paused.load();
}

bool cherry::is_running(bool change, bool set_running) {
    if (change)
        cntnr_c.running.store(set_running);
    return cntnr_c.running.load();
}


void cherry::start(void) {
    cntnr_c.thread = std::jthread([&](std::stop_token token) {
        using namespace std::chrono;
        
        const auto frameDuration = duration<double>(1.0 / 60.0);
        
        while (!token.stop_requested()) {
            {
                std::unique_lock lock(cntnr_c.mutex);
                cntnr_c.cv.wait(lock, token, []() {
                    return !cntnr_c.paused.load();
                });
                
                if (token.stop_requested())
                    break;
            }
            
            auto frameStart = steady_clock::now();
            
            std::vector<uint32_t> data{0};
            if (cntnr_c.renderer->GetFrameIfReady(data, cntnr_c.height, cntnr_c.width))
                cherry::video_callback(cherry::context, data.data(), 0);

            // Limit FPS
            auto frameEnd = steady_clock::now();
            auto elapsed = frameEnd - frameStart;
            if (elapsed < frameDuration)
                std::this_thread::sleep_for(frameDuration - elapsed);
        }
    });
}

void cherry::stop(void) {
    cntnr_c.emulator->Stop(false, true);
    
    cntnr_c.thread.request_stop();
    if (cntnr_c.thread.joinable())
        cntnr_c.thread.join();
    
    cntnr_c.paused.store(false);
    cntnr_c.running.store(false);
}


int cherry::framebuffer_height(void) {
    return cntnr_c.height;
}

int cherry::framebuffer_width(void) {
    return cntnr_c.width;
}


void cherry::audio_buffer_callback(cherry::AudioVideoBufferCallback callback) {
    cherry::audio_callback = callback;
}

void cherry::video_buffer_callback(cherry::AudioVideoBufferCallback callback) {
    cherry::video_callback = callback;
}


void cherry::press_button(uint32_t button) {
    cntnr_c.input->keys |= button;
}

void cherry::release_button(uint32_t button) {
    cntnr_c.input->keys &= ~button;
}


void cherry::set_context(void* context) {
    cherry::context = context;
}
