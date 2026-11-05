// The package smoke test's shared library (test/consumer/CMakeLists.txt): what
// Basu's engine library calls, linked against an installed tts-cpp package.

#include "ggml-backend.h"
#include "tts-cpp/chatterbox/engine.h"

#include <cstdio>
#include <cstring>
#include <exception>

#if defined(_WIN32)
#  define PROBE_API extern "C" __declspec(dllexport)
#else
#  define PROBE_API extern "C" __attribute__((visibility("default")))
#endif

// Lists ggml's registered backends and their devices. Returns 1 when `require`
// is null or names a registered backend, else 0.
PROBE_API int tts_probe_devices(const char * require) {
    int found = require == nullptr;
    for (size_t i = 0; i < ggml_backend_reg_count(); ++i) {
        ggml_backend_reg_t reg = ggml_backend_reg_get(i);
        const char * name = ggml_backend_reg_name(reg);
        std::printf("backend %s: %zu device(s)\n", name, ggml_backend_reg_dev_count(reg));
        if (require != nullptr && std::strcmp(name, require) == 0) {
            found = 1;
        }
    }
    for (size_t i = 0; i < ggml_backend_dev_count(); ++i) {
        ggml_backend_dev_t dev = ggml_backend_dev_get(i);
        ggml_backend_dev_props props;
        ggml_backend_dev_get_props(dev, &props);
        std::printf("device %s on %s: %s, id %s\n", props.name,
                    ggml_backend_reg_name(ggml_backend_dev_backend_reg(dev)), props.description,
                    props.device_id != nullptr ? props.device_id : "(none)");
    }
    return found;
}

// Opens the model on the first graphics device and reads `text`. Returns 0 when
// it made audio. Called only with model files, but always linked, so the
// library pulls in the whole Engine.
PROBE_API int tts_probe_speak(const char * t3, const char * s3gen, const char * text) {
    try {
        tts_cpp::chatterbox::set_vulkan_device(0);
        tts_cpp::chatterbox::EngineOptions opts;
        opts.t3_gguf_path    = t3;
        opts.s3gen_gguf_path = s3gen;
        opts.n_gpu_layers    = 99;  // all of it, as Basu's engine asks
        tts_cpp::chatterbox::Engine engine(opts);
        engine.set_max_tokens(400);
        engine.clear_cancel();
        const tts_cpp::chatterbox::SynthesisResult result = engine.synthesize(text);
        std::printf("%s: %zu samples at %d Hz, stopped by itself: %s\n", engine.backend_name().c_str(),
                    result.pcm.size(), result.sample_rate, engine.last_stopped() ? "yes" : "no");
        return result.pcm.empty() ? 1 : 0;
    } catch (const std::exception & e) {
        std::fprintf(stderr, "tts_probe_speak: %s\n", e.what());
        return 1;
    }
}
