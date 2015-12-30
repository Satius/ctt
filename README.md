# ctt
Test task for Cinemo

## Conditions
1. application is called with pathname as argument, all WAV-files contained directly in that folder are to be encoded to MP3
2. use all available CPU cores for the encoding process in an efficient way by utilizing multi-threading
3. statically link to lame encoder library
4. application should be compilable and runnable on Windows and Linux
5. the resulting MP3 files are to be placed within the same directory as the source WAV files, the filename extension should be changed appropriately to .MP3
6. non-WAV files in the given folder shall be ignored
7. multithreading shall be implemented by means of using Posix Threads
8. the Boost library shall not be used
9. the LAME encoder should be used with reasonable standard settings

## Linux
Buildable on any more-or-less modern distribution (gcc 4.8+) as easy as "cmake . && make". Static libmp3lame library must be installed to /usr/local/lib prior to building ctt.

## MinGW
In addition to standard mingw32-base, following packages must be installed:
* mingw32-gcc-g++
* mingw32-pthreads-w32
Afterwards, generate MinGW makefile with cmake and build using mingw32-make. Prebuilt static libmp3lame is available in lib directory.

## VS2015
Latest cmake does not properly work with latest Visual Studio. Project must be built manually:
* Create WIN32 console project from existing source
* Add include directory to compiler configuration
* Add lib directory to linker configuration
* Additionally link with pthreadVSE1.lib, libmp3lame-static.lib, libmpghip-static.lib

## Remarks
* Code is little-endian and will not work on big-endian architectures
* Proper support for wav format is too big, so I ignore many chunk types (i.e. wavl, slnt)
* I've implemented support for 16 and 8 bit wavs, 24 and 32 bit wavs would not be encoded
* Only mono and stereo wavs are supported, no multichannel support
* C++11 standard implementation is broken in VS2015, this is sad
* I'd prefer to use C++11 threads and mutexes instead of pthreads
