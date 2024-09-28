# ofdmtvrx – OFDMTV decoder for Linux

## Preface

OFDMTV (COFDMTV for color images) is an interesting ham radio mode, developed by Ahmet Inan and used to send digitally-encoded files (typically images) via any audio channel – typically a FM radio channel.

At least one team operating amateur stratospheric balloons (HAST – High Altitude Stratospheric Team) uses this mode instead of traditionally used analogue SSTV to transmit good quality images captured by the balloon in the stratosphere.

Unfortunately, there was (as now there is) no easy way to decode these images on Linux. There's [Assempix application](https://github.com/aicodix/assempix) developed by Ahmet for Android, which takes input from a microphone (typically held close to radio speakers) and decodes OFDMTV. People have used this application on their PCs with Android emulators, with varying results.

My goal was to create an easy to use application that would automatically decode incoming audio stream, either from a .wav file or from a sound card (using `arecord` or equivalent), and save decoded images to a predefined location. Basically an Assempix equivalent, but for Linux, and operated from a command line.

It's based on work by Ahmet Inan and contains his code, although slightly modified, in `aicodix` directory. I might switch to using Git submodules later.

If you want to read more about practical use of OFDMTV and my first attempts at using Ahmet's [modem decoder](https://github.com/aicodix/modem) and [CRS decoder](https://github.com/aicodix/crs) from a shell script, take a look at this thread (in Polish): http://sp7pki.iq24.pl/default.asp?grupa=230409&temat=638916

## How to build

This one-liner should do the trick on Debian:

`sudo apt-get install g++ scons git && git clone https://github.com/CircuitChaos/ofdmtvrx && cd ofdmtvrx && scons && sudo scons install`

There may be some other dependencies – if you happen to stumble upon them, please let me know.

If the build succeeds, you will have a binary called `ofdmtvrx` in your `/usr/local/bin/` directory. If you don't want to install system-wide, you can skip `sudo scons install` part above – the binary will be located in `build/` directory.

## How to use

Run `ofdmtvrx -h` to see help, or `ofdmtvrx -v` to see version. Typically you should prepare a mono, 16-bit WAV audio recording, and do:

`ofdmtvrx -i <input file> -o <output dir>`

Output directory must already exist – ofdmtvrx won't create it for you, it will just fail with „No such file or directory” on first file save attempt.

If input file is skipped, then `ofdmtvrx` will read from stdin, but there's a check added to make sure you aren't trying to read from a terminal.

If output directory is skipped, then `ofdmtvrx` will create files in current directory.

Example with arecord:

`arecord -f s16_le -r 8000 -c 1 | ofdmtvrx`

Another example, to save incoming stream while decoding and print peak incoming audio level once per second:

`arecord -f s16_le -r 8000 -c 1 | tee file.wav | ofdmtvrx -p`

## How to test

Here's a simple example that needs `yt-dlp` and `sox`. It uses a video created by SP5LOT to test OFDMTV reception.

```
yt-dlp -x --audio-format wav -o /tmp/test.wav https://www.youtube.com/watch?v=f8aWa8uixn8
sox /tmp/test.wav -c 1 /tmp/test2.wav
mkdir /tmp/testpictures
ofdmtvrx -i /tmp/test.wav -o /tmp/testpictures
```

`sox` is used to convert channel count, and also to remove the extra chunk (LIST) added by yt-dlp / ffmpeg. My .wav decoder is very simple and doesn't handle these chunks (perhaps it could be made better one day, or decoding could be delegated to `sox` or something).

## TODO

* Create output directory if it doesn't exist
* Add option to save corrupted files
* Better sound level measurement (now it's only peak detection)
* Better duplicate file cache handling
* Determine file type / extension (now .jpg is assumed, might be incorrect)
* Print audio offset in seconds (useful for working with pre-recorded .wav files)
* Better .wav file handling (with LIST chunks), maybe delegated to `sox` or some library
* Ahmet's decoder creates spectrum, spectrogram, constellation and peak meter – find a way to use them (maybe optionally create X windows?)
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
