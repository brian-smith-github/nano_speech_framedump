# 16-bit Speech 'feature extractor' for Arduino Nano.

![test](arduino_nano_with_max4466_mic.jpg)


This is really just an upload of a proof-of-concept for an audio recording
and feature-extrator for the Arduino Nano (Atmel 328p) 8-bit microcrontroller
with 2K RAM, 32K flash, running at 16MHz.

It is based around suprising results from experiments in seeing how
computationally lightweight an alternative  feature-extractor front-end 
for the 'Whisper' open-source speech-to-text application could be.

(See https://github.com/brian-smith-github/mel80_64_int_top5)

The results suggested that is should be possible to product accurate 
transcriptions using only integer maths at very low resolutions (<16bit),
and an (unrolled, optimized) FFT algorithm,
which prompted this test implementation. 

(see https://github.com/brian-smith-github/intfft128_unrolled
and https://github.com/brian-smith-github/intfft128_unrolled_2bit)

# How it works
- a timer is set up to generate an interupt 6400 times/second. 
On receiving an interrupt the ADC is read and the next ADC conversion started.
The samples have a 1.0 pre-emphasis (essentially delta-coding) to flatten
the spectrum and remove any DC level.
- every 64 samples (10ms), a buffer of the last 128 samples is scaled to 4-bit rrange, and a 5-bit Hann-like window is applied (based on a window function 
from the LPCNet codec).
- a 128-wide FFT is then applied to the data to generate 65 (64 excluding 0Hz)
spectrum, maintained within a 16-bit range for the real/imaginary compnents.
- Rather than go through the slow process of calculating 32-bit magnitude
of each of the 65 16-bit complex frequencies of the FFT output
 (i.e. real*real+imag*imag), a lightweight
heuristic is used to determine the 'peaks' in the spectrum 
(based on https://dspguru.com/dsp/tricks/magnitude-estimator/)
- The 'top 5' (bounded) peaks (corresponding to formant locations) as
(FFT bin,real component,imaginary component), along with the overall log2 
energy of the frame, is written out over the serial port for future processing.
(which generally means conversion to the format-style of feature data used
by the 'Whisper' back-end.

Just watching the frame data scroll by in the Arduino-IDE Serial Monitor can be interesting, seeing the particular formant frequencies appear,  particularly for vowel sounds.

# Extra directories
serial_read/ contains a simple Linux C program to read the frame data coming
from /dev/ttyUSB0 and write it to a file (/tmp/a.raw).

whisper/ contains some C and shell-scripts to convert this frame data into 
the '80 Mel bins at 100fps'-style format that Whisper uses, and runs
the neural-net transcription back-end on that data
to produce the final text output.

# Shortcommings
Unfortunately the microphone setup I'm currently using
(MAX4466 board with analog-out going straight into pin A7,
3V3 and GND for power, no external filtering components at all)
produces very poor
quality noisy audio which hinders the results, improved mic setup is being
explored.

Running the same 16-bit integer code on my Linux desktop with 'good quality'
6400 samples/sec  audio clips produces superiour results.

For now, this was just more a test to see
whether the Arduno Nano has enough horsepower to do the same amount of 
processing (i.e. 128-wide/20ms FFT at 100fps) in realtime -
which seems to be true.

# Thanks.
This project is inspired by work by Peter Balch:
"Speech Recognition With an Arduino Nano"

https://www.instructables.com/Speech-Recognition-With-an-Arduino-Nano/

Also uses examples on Arduino interrupts by amandaghassaei:

https://www.instructables.com/Arduino-Timer-Interrupts/

# See also: (prior project)
"Simple Speech-To-Text on the '10 cents' CH32V003 Microcontroller

https://github.com/brian-smith-github/ch32v003_stt
