# ofdmtvrx – cross-platform COFDMTV decoder

## Preface

COFDMTV is an interesting ham radio mode, developed by Ahmet Inan and used to send digitally-encoded files (typically images) via any audio channel – typically a FM radio channel. [Here's](https://www.aicodix.de/cofdmtv/) a brief description.

At least one team operating amateur stratospheric balloons (HAST – High Altitude Stratospheric Team) uses this mode instead of traditionally used analogue SSTV to transmit good quality images captured by the balloon floating in the stratosphere.

Unfortunately, there was (as now there is) no easy way to decode these images on a PC. There's [Assempix application](https://github.com/aicodix/assempix) developed by Ahmet for Android, which takes input from a microphone (typically held close to radio speakers) and decodes COFDMTV. People have used this application on their PCs with Android emulators, with varying results.

My goal was to create an easy to use application that would automatically decode incoming audio stream, either from a .wav file or from a sound card (using `arecord` or equivalent), and save decoded images to a predefined location. Basically an Assempix equivalent, but running on a PC and operated from a command line.

The program was initially created for Linux, but due to requests, now it's also possible to build and use it on MS Windows. It should be doable to port it to other platforms as well.

It's based on work by Ahmet Inan and contains his code, although slightly modified, in `aicodix` directory. I might switch to using Git submodules later.

If you want to read more about practical use of COFDMTV and my first attempts at using Ahmet's [modem decoder](https://github.com/aicodix/modem) and [CRS decoder](https://github.com/aicodix/crs) from a shell script, take a look at [this thread](http://sp7pki.iq24.pl/default.asp?grupa=230409&temat=638916) (in Polish).

The program is available as:

* Source code for Linux
* Source code for Windows
* Binary (.exe) file for Windows

## How to build on Linux

You can build `ofdmtvrx` either with support for X11, or without it, as a fully console-based program. If support for X is compiled-in, the application will still be able to run as a console application (without X) by using `-n` command line switch, although in this case libX11 must exist for it to build and start.

X11 support creates windows with oscilloscope, spectrum, spectrogram, constellation, and peak meter.

This command should do the trick on Debian:

```
sudo apt-get install g++ scons git libx11-dev && \
git clone https://github.com/CircuitChaos/ofdmtvrx && \
cd ofdmtvrx && scons WITH_X=1 && sudo scons install WITH_X=1
```

Or, if you don't need X support:

```
sudo apt-get install g++ scons git && \
git clone https://github.com/CircuitChaos/ofdmtvrx && \
cd ofdmtvrx && scons WITH_X=0 && sudo scons install WITH_X=0
```

There may be some other dependencies. If you happen to stumble upon them, please let me know.

If the build succeeds, you will have a binary called `ofdmtvrx` in your `/usr/local/bin` directory. If you don't want to install system-wide, you can skip `sudo scons install` part above – the binary will be located in `build` directory.

## How to build on MS Windows

Note that you don't **have to** build the program yourself. You can do it if you want some cutting-edge changes that are not versioned yet, or you want to modify the code. If you want to use the pre-built version, just download the newest `.exe` file from the `dist` directory.

Here's a step-by-step guide I tested on Windows 7. You have to be familiar with Windows command prompt (cmd) to build and use it.

First you need to download some tools:

* MinGW
* SCons
* Git

MinGW can be found [here](https://github.com/niXman/mingw-builds-binaries/releases) (pick the `x86_64-…-release-win32-seh-msvcrt-rt_v12-rev0.7z` file – at the time of writing latest version is 14.2.0, but it should work with any newer one as well).

Extract it to a convenient place (if you can't, then you also need some program to handle 7-Zip files, one example is [here](https://7-zip.org/download.html)). From now on I'll assume it's in `C:\ofdm\mingw64`, but it can be anywhere, just adjust the `set PATH` command below.

SCons can be found [here](https://sourceforge.net/projects/scons/files/scons-local/). Pick the newest version and download the zip file. Unzip it to `C:\ofdm\scons` (or somewhere else, and adjust `set PATH` below).

You also need Python, [here](https://www.python.org/downloads/windows/). Pick the „Windows embeddable package (64-bit)”, download, unzip to `C:\ofdm\python` (or somewhere else, and adjust `set PATH` below).

If you're on Windows 7, use [Python 3.8.7](https://www.python.org/ftp/python/3.8.7/python-3.8.7-embed-amd64.zip). Newer versions won't work (there's a DLL missing).

Last program is Git. Click [here](https://git-scm.com/downloads/win), select „64-bit Git for Windows Portable” and run it. It will ask where to extract the files. Type `C:\ofdm\git` (or, as above, something else, and adjust `set PATH`).

If an error appears saying that GetSystemTimePreciseAsFileTime symbol (entry point) is not found in KERNEL32.dll, ignore it.

Finally, you can go to command line (Windows+R, cmd, Enter) and type these commands:

```
cd c:\ofdm
set PATH=%PATH%;c:\ofdm\git\bin;c:\ofdm\mingw64\bin;c:\ofdm\scons;c:\ofdm\python
git clone https://github.com/CircuitChaos/ofdmtvrx
cd ofdmtvrx
scons
```

File called `ofdmtvrx.exe` will be created in the `build` directory. Copy it where you want and use.

Note that there's no `WITH_X` argument on Windows.

Also note that typically Windows programs use slash to specify arguments (like /i, /o, etc.), but this program uses minus (-i, -o, etc.), like on Linux.

GUI support for Windows version of this program was written by [Grzegorz Niemirowski, SP5GND](https://www.grzegorz.net/) – thank you!

## How to use

Run `ofdmtvrx -h` to see help, or `ofdmtvrx -v` to see version. Typically you should prepare a 16-bit WAV audio recording (mono or stereo, doesn't matter, the program internally downmixes to mono), and do:

`ofdmtvrx -i <input file> -o <output dir>`

Output directory must already exist – `ofdmtvrx` won't create it for you, it will just fail with „No such file or directory” on first file save attempt.

If input file is skipped, then `ofdmtvrx` will read from stdin, but there's a check added to make sure you aren't trying to read from a terminal. This check doesn't work on Windows, so if you just run `ofdmtvrx.exe` without arguments, it will do nothing (will look hanged). A message will be shown to indicate that it's not a bug. If this happens to you, just press Ctrl-C.

If output directory is skipped, then `ofdmtvrx` will create files in current directory.

If you need a simple, text-based peak audio level meter, add `-p`.

Example with arecord on Linux:

`arecord -f s16_le -r 8000 -c 1 | ofdmtvrx`

Another example, to save incoming stream while decoding and print peak incoming audio level once per second:

`arecord -f s16_le -r 8000 -c 1 | tee file.wav | ofdmtvrx -p`

If you figure an easy way to pipe input stream from sound card on Windows, please let me know. [Some suggestions](https://superuser.com/questions/1024792/is-there-a-windows-equivalent-to-the-linux-aplay-that-will-accept-a-bitstream).

This MIGHT work (I didn't test it):

`sox -d -e signed -b 16 -r 8k -c 1 - | ofdmtvrx`

Or:

`sox -t waveaudio 0 -e signed -b 16 -r 8k -c 1 - | ofdmtvrx`

If you compiled the program with X support, but aren't running X (no DISPLAY set, etc.), add `-n` option. Otherwise the program will not run.

If you're planning to do some weird things wih this program, know that it has to read the WAV file header before it does anything else (initializes the decoder, creates X windows, etc.). This is by design.

## How to test

Here's a simple example for Linux that needs `yt-dlp`. It uses a video created by SP5LOT to test COFDMTV reception.

```
yt-dlp -x --audio-format wav -o /tmp/test.wav https://www.youtube.com/watch?v=f8aWa8uixn8
mkdir /tmp/testpictures
ofdmtvrx -i /tmp/test2.wav -o /tmp/testpictures
```

## X11 support

Below applies only for Linux.

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
* Output file handling
  * Create output directory if it doesn't exist
  * Don't save files if output directory is not specified
  * Better duplicate file cache handling
  * Determine file type / extension (now .jpg is assumed, might be incorrect)
* X11 integration
  * Create window with a last received image
  * Windows are created in the middle of the screen, on top of each other; do something about it
* MS Windows
  * Figure a way to read directly from a sound card, like with `arecord` on Linux
  * Check if it works with files larger than 2 GiB (there's no `O_LARGEFILE`)
* Other platforms
  * Port to other platforms, as needed – BSD, MacOS, etc.
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
