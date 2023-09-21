#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <webrtc-audio-processing-1/modules/audio_processing/include/audio_processing.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

#define BYTES_10_MS 320

namespace py = pybind11;

// ----------------------------------------------------------------------------

struct ProcessedAudioChunk {
  py::bytes audio;
  bool is_speech;

  ProcessedAudioChunk() : audio("\0", BYTES_10_MS), is_speech(false) {}
};

class AudioProcessor {
private:
  webrtc::AudioProcessing *apm = NULL;
  webrtc::AudioProcessing::Config audio_config;
  webrtc::StreamConfig stream_config;

public:
  AudioProcessor(int auto_gain, int noise_suppression);
  ~AudioProcessor();

  std::unique_ptr<ProcessedAudioChunk> Process10ms(py::bytes audio);
};

// auto_gain: 0 = disabled, max 31
// noise_suppression: 0 = disabled, max 4
AudioProcessor::AudioProcessor(int auto_gain, int noise_suppression)
    : stream_config(16000, 1, false) {
  this->apm = webrtc::AudioProcessingBuilder().Create();

  this->audio_config.echo_canceller.enabled = false;

  if (auto_gain > 0) {
    this->audio_config.gain_controller1.enabled = true;
    this->audio_config.gain_controller1.target_level_dbfs = auto_gain;
    this->audio_config.gain_controller2.enabled = true;
  }

  this->audio_config.high_pass_filter.enabled = true;
  this->audio_config.voice_detection.enabled = true;

  if (noise_suppression > 0) {
    this->audio_config.noise_suppression.enabled = true;
    if (noise_suppression == 1) {
      this->audio_config.noise_suppression.level =
          webrtc::AudioProcessing::Config::NoiseSuppression::Level::kLow;
    } else if (noise_suppression == 2) {
      this->audio_config.noise_suppression.level =
          webrtc::AudioProcessing::Config::NoiseSuppression::Level::kModerate;
    } else if (noise_suppression == 3) {
      this->audio_config.noise_suppression.level =
          webrtc::AudioProcessing::Config::NoiseSuppression::Level::kHigh;
    } else if (noise_suppression >= 4) {
      this->audio_config.noise_suppression.level =
          webrtc::AudioProcessing::Config::NoiseSuppression::Level::kVeryHigh;
    }
  }

  this->apm->ApplyConfig(audio_config);
}

std::unique_ptr<ProcessedAudioChunk>
AudioProcessor::Process10ms(py::bytes audio) {
  auto processed_chunk = std::make_unique<ProcessedAudioChunk>();

  py::buffer_info buffer_input(py::buffer(audio).request());
  py::buffer_info buffer_output(py::buffer(processed_chunk->audio).request());

  if ((buffer_input.size != BYTES_10_MS) ||
      (buffer_output.size != BYTES_10_MS)) {
    throw std::runtime_error(
        "Input and output buffers must be 320 bytes (10 ms)");
  }

  this->apm->ProcessStream(static_cast<int16_t *>(buffer_input.ptr),
                           this->stream_config, this->stream_config,
                           static_cast<int16_t *>(buffer_output.ptr));

  processed_chunk->is_speech =
      this->apm->GetStatistics().voice_detected.value_or(false);

  return processed_chunk;
}

AudioProcessor::~AudioProcessor() { delete this->apm; }

// ----------------------------------------------------------------------------

PYBIND11_MODULE(webrtc_noise_gain_cpp, m) {
  m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------

        .. currentmodule:: webrtc_noise_gain_cpp

        .. autosummary::
           :toctree: _generate

           AudioProcessor
    )pbdoc";

  py::class_<AudioProcessor>(m, "AudioProcessor")
      .def(py::init<int, int>())
      .def("Process10ms", &AudioProcessor::Process10ms);

  py::class_<ProcessedAudioChunk>(m, "ProcessedAudioChunk")
      .def_readonly("audio", &ProcessedAudioChunk::audio)
      .def_readonly("is_speech", &ProcessedAudioChunk::is_speech);

#ifdef VERSION_INFO
  m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
  m.attr("__version__") = "dev";
#endif
}
