#include <exd/app/audio.hpp>

#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <fstream>

namespace exd::app {
namespace {

/// Write a minimal valid WAV file (1 second of 44100 Hz stereo S16 silence).
static std::string write_silent_wav() {
    std::string path = "/tmp/exd_test_silence.wav";
    FILE* f = std::fopen(path.c_str(), "wb");
    REQUIRE(f != nullptr);

    int sample_rate = 44100;
    int num_channels = 2;
    int bits_per_sample = 16;
    int data_size = sample_rate * num_channels * (bits_per_sample / 8); // 1 sec

    // RIFF header
    std::fwrite("RIFF", 1, 4, f);
    int chunk_size = 36 + data_size;
    std::fwrite(&chunk_size, 4, 1, f);
    std::fwrite("WAVE", 1, 4, f);

    // fmt subchunk
    std::fwrite("fmt ", 1, 4, f);
    int subchunk1_size = 16;
    std::fwrite(&subchunk1_size, 4, 1, f);
    short audio_format = 1; // PCM
    std::fwrite(&audio_format, 2, 1, f);
    std::fwrite(&num_channels, 2, 1, f);
    std::fwrite(&sample_rate, 4, 1, f);
    int byte_rate = sample_rate * num_channels * bits_per_sample / 8;
    std::fwrite(&byte_rate, 4, 1, f);
    short block_align = num_channels * bits_per_sample / 8;
    std::fwrite(&block_align, 2, 1, f);
    std::fwrite(&bits_per_sample, 2, 1, f);

    // data subchunk
    std::fwrite("data", 1, 4, f);
    std::fwrite(&data_size, 4, 1, f);
    std::vector<char> silence(data_size, 0);
    std::fwrite(silence.data(), 1, data_size, f);

    std::fclose(f);
    return path;
}

TEST_CASE("Audio: construction and destruction", "[audio]") {
    CHECK_NOTHROW(Audio());
}

TEST_CASE("Audio: load_clip with nonexistent file returns false", "[audio]") {
    Audio audio;
    CHECK_FALSE(audio.load_clip("bad", "/nonexistent/file.wav"));
}

TEST_CASE("Audio: load_clip with valid WAV", "[audio]") {
    Audio audio;
    std::string path = write_silent_wav();
    CHECK(audio.load_clip("silence", path));
    std::remove(path.c_str());
}

TEST_CASE("Audio: play nonexistent clip does not crash", "[audio]") {
    Audio audio;
    CHECK_NOTHROW(audio.play("nonexistent"));
    CHECK_NOTHROW(audio.play_loop("nonexistent"));
    CHECK_NOTHROW(audio.stop_loop("nonexistent"));
}

TEST_CASE("Audio: play loaded clip does not crash", "[audio]") {
    Audio audio;
    std::string path = write_silent_wav();
    REQUIRE(audio.load_clip("silence", path));
    CHECK_NOTHROW(audio.play("silence"));
    CHECK_NOTHROW(audio.play("silence", 0.5f));
    CHECK_NOTHROW(audio.play_loop("silence", 0.3f));
    std::remove(path.c_str());
}

TEST_CASE("Audio: stop_loop after play_loop", "[audio]") {
    Audio audio;
    std::string path = write_silent_wav();
    REQUIRE(audio.load_clip("silence", path));
    CHECK_NOTHROW(audio.play_loop("silence", 0.5f));
    CHECK_NOTHROW(audio.stop_loop("silence"));
    std::remove(path.c_str());
}

TEST_CASE("Audio: stop_all", "[audio]") {
    Audio audio;
    std::string path = write_silent_wav();
    REQUIRE(audio.load_clip("silence", path));
    audio.play("silence");
    audio.play("silence");
    audio.play_loop("silence");
    CHECK_NOTHROW(audio.stop_all());
    std::remove(path.c_str());
}

TEST_CASE("Audio: volume clamping", "[audio]") {
    Audio audio;
    std::string path = write_silent_wav();
    REQUIRE(audio.load_clip("silence", path));
    // Out-of-range volumes should not crash
    CHECK_NOTHROW(audio.play("silence", 2.0f));
    CHECK_NOTHROW(audio.play("silence", -1.0f));
    CHECK_NOTHROW(audio.play_loop("silence", 5.0f));
    std::remove(path.c_str());
}

TEST_CASE("Audio: multiple clips", "[audio]") {
    Audio audio;
    std::string path1 = write_silent_wav();
    std::string path2 = "/tmp/exd_test_silence2.wav";
    // Copy the same WAV under a second name
    {
        std::ifstream src(path1, std::ios::binary);
        std::ofstream dst(path2, std::ios::binary);
        dst << src.rdbuf();
    }
    REQUIRE(audio.load_clip("a", path1));
    REQUIRE(audio.load_clip("b", path2));
    CHECK_NOTHROW(audio.play("a"));
    CHECK_NOTHROW(audio.play("b"));
    CHECK_NOTHROW(audio.play_loop("a"));
    CHECK_NOTHROW(audio.stop_loop("a"));
    std::remove(path1.c_str());
    std::remove(path2.c_str());
}

TEST_CASE("Audio: replaced loop", "[audio]") {
    // Calling play_loop twice with the same name replaces the loop
    Audio audio;
    std::string path = write_silent_wav();
    REQUIRE(audio.load_clip("silence", path));
    CHECK_NOTHROW(audio.play_loop("silence", 0.5f));
    CHECK_NOTHROW(audio.play_loop("silence", 0.8f)); // should replace
    std::remove(path.c_str());
}

} // namespace
} // namespace exd::app
