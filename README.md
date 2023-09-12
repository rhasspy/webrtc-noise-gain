# webrtc-noise-gain

Tiny Python wrapper around [webrtc-audio-processing](https://gitlab.freedesktop.org/pulseaudio/webrtc-audio-processing/) for noise suppression and auto gain only.


## Install

``` sh
pip install webrtc-noise-gain
```


## Usage


``` python
import numpy as np
import sys

from webrtc_noise_gain import AudioProcessor

# 0 = disable
auto_gain_dbfs = 3  # [0, 31]

# 0 = disable, 4 is max suppression
noise_suppression_level = 2  # [0, 4]

# 16 Khz mono with 16-bit samples only
p = AudioProcessor(auto_gain_dbfs, noise_suppression_level)

# Operates on 10ms of audio at a time (160 samples @ 16Khz)
audio_bytes_10ms = ...  # 160 samples (320 bytes)
data_in = np.frombuffer(audio_bytes_10ms, dtype=np.int16)
data_out = np.zeros(shape=(160,), dtype=np.int16)

# Returns True if VAD detected speech
is_speech = p.Process10ms(data_in, data_out)

# data_out contains clean audio
```

