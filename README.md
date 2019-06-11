# mpeg-ts-demux

mpeg-ts-demux is a simple demultiplexer capable of extracting only media streams from MPEG transport stream and saving them to separate files

# Build
### Platforms supported
Unix only
### Prerequisites
C++14 enabled compiler
*cmake 3.6+*
*boost* (tested with 1.69.0 but should work with any boost)

### How to build
Run *./build.sh*. Executable will be written to *./build/ folder*

# Help

## MPEG-TS documentation

- https://tsduck.io/download/docs/mpegts-introduction.pdf
- https://en.wikipedia.org/wiki/MPEG_transport_stream

## Tool usage

Run *./build/mpeg-ts-demux --help* for options

License
