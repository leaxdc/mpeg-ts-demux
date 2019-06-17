# mpeg-ts-demux

mpeg-ts-demux is a simple demultiplexer capable of extracting only media streams from MPEG transport stream and saving them to separate files which should be playable with ffplay: https://ffmpeg.org/ffplay.html

## Build

### Platforms supported
Unix only

### Prerequisites
- C++17 enabled compiler
- *cmake 3.6+*
- *boost 1.66.0+*

### How to build
Run *./build.sh*. Executable will be written to *./build/ folder*

## Help

### Tool usage
Run *./build/mpeg-ts-demux --help* for options

### MPEG-TS documentation
- https://tsduck.io/download/docs/mpegts-introduction.pdf
- https://en.wikipedia.org/wiki/MPEG_transport_stream

## License
https://opensource.org/licenses/MIT
