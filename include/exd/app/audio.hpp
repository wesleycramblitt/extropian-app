#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace exd::app {

/// Simple audio player backed by SDL3.
///
/// Loads WAV files, plays one-shot sound effects and
/// looping background music with independent volume control.
///
/// Usage:
/// @code
///   Audio audio;
///   audio.load_clip("click", "sfx/click.wav");
///   audio.load_clip("theme", "music/theme.wav");
///   audio.play_loop("theme", 0.4f);
///   audio.play("click");
/// @endcode
class Audio {
public:
    Audio();
    ~Audio();

    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    /// Load a WAV file and cache it under `name`. Returns false on failure.
    bool load_clip(const std::string& name, const std::string& wav_path);

    /// Play a loaded clip once (sound effect). Does nothing if name not found.
    void play(const std::string& name, float volume = 1.0f);

    /// Start looping a loaded clip (music). Replaces any existing loop of the
    /// same name. Does nothing if name not found.
    void play_loop(const std::string& name, float volume = 0.5f);

    /// Stop a looping clip by name. One-shot sounds cannot be stopped.
    void stop_loop(const std::string& name);

    /// Stop all playback (sounds and loops).
    void stop_all();

private:
    struct Clip {
        Uint8* data = nullptr;
        Uint32 len  = 0;
        SDL_AudioSpec spec{};
        ~Clip() { SDL_free(data); }
    };

    struct Active {
        const Clip* clip;
        Uint32      position;   // sample-frame position
        float       volume;
        bool        loop;
        bool        finished = false;
    };

    std::unordered_map<std::string, std::unique_ptr<Clip>> clips_;
    std::vector<Active> active_;
    std::recursive_mutex mutex_;   // audio callback runs on SDL thread

    SDL_AudioStream* stream_ = nullptr;

    static void SDLCALL callback(void* userdata, SDL_AudioStream* stream,
                                 int additional, int total);
    void mix_frames(Sint16* output, int frames);
};

} // namespace exd::app
