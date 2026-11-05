// Runs the package smoke test's library (test/consumer/CMakeLists.txt):
//   tts_probe_run [--require <backend>] [<t3.gguf> <s3gen.gguf> ["text"]]

#include <cstdio>
#include <cstring>

extern "C" int tts_probe_devices(const char * require);
extern "C" int tts_probe_speak(const char * t3, const char * s3gen, const char * text);

int main(int argc, char ** argv) {
    const char * require = nullptr;
    int arg = 1;
    if (arg + 1 < argc && std::strcmp(argv[arg], "--require") == 0) {
        require = argv[arg + 1];
        arg += 2;
    }
    if (!tts_probe_devices(require)) {
        std::fprintf(stderr, "tts_probe_run: ggml registered no %s backend\n", require);
        return 1;
    }
    if (arg + 1 < argc) {
        const char * text = arg + 2 < argc ? argv[arg + 2] : "The package reads aloud.";
        return tts_probe_speak(argv[arg], argv[arg + 1], text);
    }
    return 0;
}
