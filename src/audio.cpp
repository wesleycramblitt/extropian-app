#include <exd/app/audio.hpp>
#include <exd/core/logging.hpp>
#include <cstring>
#include <algorithm>

namespace exd::app {

// ── Construction ────────────────────────────────────────────────

Audio::Audio() {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        exd::core::log_error("SDL audio init failed: %s", SDL_GetError());
        return;
    }

    SDL_AudioSpec spec{SDL_AUDIO_S16, 2, 44100};
    stream_ = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                        &spec, callback, this);
    if (!stream_) {
        exd::core::log_error("Audio device open failed: %s", SDL_GetError());
        return;
    }

    SDL_ResumeAudioStreamDevice(stream_);
}

Audio::~Audio() {
    if (stream_) {
        SDL_DestroyAudioStream(stream_);
    }
}

// ── Clip loading ────────────────────────────────────────────────

bool Audio::load_clip(const std::string& name, const std::string& wav_path) {
    auto clip = std::make_unique<Clip>();
    if (!SDL_LoadWAV(wav_path.c_str(), &clip->spec, &clip->data, &clip->len)) {
        exd::core::log_error("Audio: failed to load '%s': %s",
                               wav_path.c_str(), SDL_GetError());
        return false;
    }
    clips_[name] = std::move(clip);
    return true;
}

// ── Playback ────────────────────────────────────────────────────

void Audio::play(const std::string& name, float volume) {
    auto it = clips_.find(name);
    if (it == clips_.end()) return;

    std::lock_guard<std::recursive_mutex> lock(mutex_);
    Active a;
    a.clip     = it->second.get();
    a.position = 0;
    a.volume   = std::clamp(volume, 0.0f, 1.0f);
    a.loop     = false;
    active_.push_back(a);
}

void Audio::play_loop(const std::string& name, float volume) {
    auto it = clips_.find(name);
    if (it == clips_.end()) return;

    std::lock_guard<std::recursive_mutex> lock(mutex_);

    // Remove any existing loop of the same name
    stop_loop(name);

    Active a;
    a.clip     = it->second.get();
    a.position = 0;
    a.volume   = std::clamp(volume, 0.0f, 1.0f);
    a.loop     = true;
    active_.push_back(a);
}

void Audio::stop_loop(const std::string& name) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto it = clips_.find(name);
    if (it == clips_.end()) return;

    const Clip* target = it->second.get();
    for (auto& a : active_) {
        if (a.clip == target && a.loop) {
            a.finished = true;
        }
    }
}

void Audio::stop_all() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    active_.clear();
}

// ── Mixing (called from SDL audio thread) ──────────────────────

void SDLCALL Audio::callback(void* userdata, SDL_AudioStream* /*stream*/,
                              int additional, int /*total*/) {
    auto* audio = static_cast<Audio*>(userdata);
    if (additional <= 0) return;

    int frames = additional / static_cast<int>(sizeof(Sint16) * 2); // stereo S16
    if (frames <= 0) return;

    std::vector<Sint16> buffer(static_cast<size_t>(frames) * 2, 0);
    audio->mix_frames(buffer.data(), frames);

    SDL_PutAudioStreamData(audio->stream_, buffer.data(),
                           static_cast<int>(buffer.size() * sizeof(Sint16)));
}

void Audio::mix_frames(Sint16* output, int frames) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    // Remove finished sounds
    active_.erase(std::remove_if(active_.begin(), active_.end(),
        [](const Active& a) { return a.finished; }), active_.end());

    for (auto& a : active_) {
        if (!a.clip || a.clip->len < 2) continue;

        // Convert clip length to sample frames
        int clip_frames = a.clip->len / SDL_AUDIO_FRAMESIZE(a.clip->spec);

        for (int f = 0; f < frames; ++f) {
            Uint32 pos = a.position + static_cast<Uint32>(f);

            if (pos >= static_cast<Uint32>(clip_frames)) {
                if (a.loop) {
                    a.position = 0;
                    pos = static_cast<Uint32>(f);  // restart from current frame offset
                    // Re-read clip_frames in case clip data changed
                    clip_frames = a.clip->len / SDL_AUDIO_FRAMESIZE(a.clip->spec);
                } else {
                    a.finished = true;
                    break;
                }
            }

            // Read a stereo S16 frame from clip
            const auto* src = reinterpret_cast<const Sint16*>(a.clip->data)
                              + pos * 2;   // 2 channels
            Sint16 left  = static_cast<Sint16>(src[0] * a.volume);
            Sint16 right = static_cast<Sint16>(src[1] * a.volume);

            // Mix into output (clamped)
            int out_l = output[f * 2] + left;
            int out_r = output[f * 2 + 1] + right;
            output[f * 2]     = static_cast<Sint16>(std::clamp(out_l, -32768, 32767));
            output[f * 2 + 1] = static_cast<Sint16>(std::clamp(out_r, -32768, 32767));
        }
    }
}

} // namespace exd::app
