#!/usr/bin/env python3
import numpy as np
import sys

from webrtc_noise_gain import AudioProcessor

p = AudioProcessor(0, 0)
data_in = np.random.randint(-(2**15), 2**15, size=(160,), dtype=np.int16)
data_out = np.zeros(shape=(160,), dtype=np.int16)
is_speech = p.Process10ms(data_in, data_out)
print("OK")
