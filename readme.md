
I listen to music a lot, so it would be cool to build something similar to Shazaam
in order to recognize which song is playing.

Goals:
- Implement each step of Shazaam's audio fingerprinting algorithmn
- Optionally visualize the spectrograms and constellation maps of an audio file
- Learn how to use a database outside of a web dev context
- Capture microphone audio and test with music streaming from another device
- Benchmark the fingerprinting algorithm and learn how to use a flamegraph
- Learn about SIMD, and optimize the code. Target at least a 20% performance improvement

Tech stack:
- C++
- miniaudio.h to read audio samples from a file
- stb_image.h to write a png file
- sqlite to store hashes in a database

Resources:
- [The original paper](https://www.ee.columbia.edu/~dpwe/papers/Wang03-shazam.pdf)
- [Very nice explanation of the paper](https://www.cameronmacleod.com/blog/how-does-shazam-work)
