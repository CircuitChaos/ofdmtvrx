# ofdmtvrx – COFDMTV decoder for Linux

## Preface

COFDMTV is an interesting ham radio mode, developed by Ahmet Inan and used to send digitally-encoded files (typically images) via any audio channel – typically a FM radio channel. [Here's](https://www.aicodix.de/cofdmtv/) a brief description.

At least one team operating amateur stratospheric balloons (HAST – High Altitude Stratospheric Team) uses this mode instead of traditionally used analogue SSTV to transmit good quality images captured by the balloon floating in the stratosphere.

Unfortunately, there was (as now there is) no easy way to decode these images on Linux. There's [Assempix application](https://github.com/aicodix/assempix) developed by Ahmet for Android, which takes input from a microphone (typically held close to radio speakers) and decodes COFDMTV. People have used this application on their PCs with Android emulators, with varying results.

My goal was to create an easy to use application that would automatically decode incoming audio stream, either from a .wav file or from a sound card (using `arecord` or equivalent), and save decoded images to a predefined location. Basically an Assempix equivalent, but for Linux, and operated from a command line.

It's based on work by Ahmet Inan and contains his code, although slightly modified, in `aicodix` directory. I might switch to using Git submodules later.

If you want to read more about practical use of COFDMTV and my first attempts at using Ahmet's [modem decoder](https://github.com/aicodix/modem) and [CRS decoder](https://github.com/aicodix/crs) from a shell script, take a look at this thread (in Polish): http://sp7pki.iq24.pl/default.asp?grupa=230409&temat=638916

## How to build

You can build `ofdmtvrx` either with support for X11, or without it, as a fully console-based program. If support for X is compiled-in, the application will still be able to run as a console application (without X) by using `-n` command line switch, although in this case libX11 must exist for it to build and start.

X11 support creates windows with oscilloscope, spectrum, spectrogram, constellation, and peak meter.

This command should do the trick on Debian:

```
sudo apt-get install g++ scons git libx11-dev && \
git clone https://github.com/CircuitChaos/ofdmtvrx && \
cd ofdmtvrx && scons WITH_X=1 && sudo scons install
```

Or, if you don't need X support:

```
sudo apt-get install g++ scons git && \
git clone https://github.com/CircuitChaos/ofdmtvrx && \
cd ofdmtvrx && scons WITH_X=0 && sudo scons install
```

There may be some other dependencies – if you happen to stumble upon them, please let me know.

If the build succeeds, you will have a binary called `ofdmtvrx` in your `/usr/local/bin/` directory. If you don't want to install system-wide, you can skip `sudo scons install` part above – the binary will be located in `build/` directory.

## How to use

Run `ofdmtvrx -h` to see help, or `ofdmtvrx -v` to see version. Typically you should prepare a mono, 16-bit WAV audio recording, and do:

`ofdmtvrx -i <input file> -o <output dir>`

Output directory must already exist – `ofdmtvrx` won't create it for you, it will just fail with „No such file or directory” on first file save attempt.

If input file is skipped, then `ofdmtvrx` will read from stdin, but there's a check added to make sure you aren't trying to read from a terminal.

If output directory is skipped, then `ofdmtvrx` will create files in current directory.

If you need a simple, text-based peak audio level meter, add `-p`.

Example with arecord:

`arecord -f s16_le -r 8000 -c 1 | ofdmtvrx`

Another example, to save incoming stream while decoding and print peak incoming audio level once per second:

`arecord -f s16_le -r 8000 -c 1 | tee file.wav | ofdmtvrx -p`

If you compiled the application with X support, but aren't running X, add `-n` option. Otherwise the application will not run.

If you're planning to do some weird things wih this application, know that it has to read the WAV file header before it does anything else (initializes the decoder, creates X windows, etc.). This is by design.

## How to test

Here's a simple example that needs `yt-dlp` and `sox`. It uses a video created by SP5LOT to test COFDMTV reception.

```
yt-dlp -x --audio-format wav -o /tmp/test.wav https://www.youtube.com/watch?v=f8aWa8uixn8
sox /tmp/test.wav -c 1 /tmp/test2.wav
mkdir /tmp/testpictures
ofdmtvrx -i /tmp/test2.wav -o /tmp/testpictures
```

`sox` is used to convert channel count, and also to remove the extra chunk (LIST) added by yt-dlp / ffmpeg. My .wav decoder is very simple and doesn't handle these chunks (perhaps it could be made better one day, or decoding could be delegated to `sox` or something).

## X11 support

When running with X, three windows are created:

* Oscilloscope, containing current signal in a time domain
* Spectrum and spectrogram, containing current signal in a frequency domain (the same as in Assempix)
* Constellation and peak meter, the same as in Assempix

Oscilloscope window can be resized, other windows can't. Each window can be closed. If you closed a window by accident, send `SIGUSR1` to reopen all windows (`killall -USR1 ofdmtvrx`).

Window with last received image is *not* created – it's a TODO.

## TODO

* Input processing
  * Better sound level measurement (now it's only peak detection)
  * Print audio offset in seconds (useful for working with pre-recorded .wav files)
  * Better .wav file handling (with LIST chunks), maybe delegated to `sox` or some library
* Output file handling
  * Create output directory if it doesn't exist
  * Add option to save corrupted files
  * Better duplicate file cache handling
  * Determine file type / extension (now .jpg is assumed, might be incorrect)
* X11 integration
  * Create window with a last received image
  * Windows are created in the middle of the screen, on top of each other; do something about it
* Miscellaneous
  * Import Ahmet's code with Git submodules

## License

Large part of the code base (in `aicodix` directory) is taken from Assempix, written by Ahmet Inan, and his other two repositories (`code` and `dsp`). See his licenses here:

* https://github.com/aicodix/assempix (commit hash used: daa52e52ca72d4cd5ec49cf28f2fb7b3462c4834)
* https://github.com/aicodix/code (commit hash used: 86f1020d54ac0901142d1ed7a0006e329e71cbca)
* https://github.com/aicodix/dsp (commit hash used: c0adfaef64e7657a08442a5d6467d9523350b57e)

It's been slightly modified by me (two cosmetic changes to make things build better and one bug fix).

When it comes to my part (in `src` directory) – I don't care, do what you want with it if it makes the world better, just don't hurt yourself (or at least don't hold me accountable). If you need a formal license, you can use the same as in Assempix.

## Contact with author

Please use GitHub issue tracker or email: circuitchaos (at) interia.com
