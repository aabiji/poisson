I listen to music a lot, so it would be cool to build something similar to Shazaam
in order to recognize which song is playing.

Goals:
- Implement each step of Shazaam's audio fingerprinting algorithmn
- Optionally visualize the spectrograms and constellation maps of an audio file
- Learn how to use a database outside of a web dev context
- Capture microphone audio and test with music streaming from another device
- **Benchmark the fingerprinting algorithm and learn how to use a flamegraph**
- **Learn about SIMD, and optimize the code. Target at least a 20% performance improvement**

Tech stack:
- C++
- ffmpeg to read audio samples **TODO: switch from miniaudio to ffmpeg**
- stb_image.h to write a png file
- sqlite to store hashes in a database

Resources:
- [The original paper](https://www.ee.columbia.edu/~dpwe/papers/Wang03-shazam.pdf)
- [Very nice explanation of the paper](https://www.cameronmacleod.com/blog/how-does-shazam-work)
- [SIMD for C++ developers](http://const.me/articles/simd/simd.pdf)

Steps to register a song:
- **Should we downsample the audio? How many channels do we use?**
- Slide an N second sample chunk across the audio samples. For each chunk:
  - [x] Apply a Hamming window to reduce the frequency jumps we'd have at the start/end of the frequency graph
  - [ ] Apply the Fourier transform to convert samples from the time domain to the frequency domain
  - [ ] Create an image of a spectrogram. The x-axis represetnts time, the y-axis represents
        frequency, the color intensity represents the amplitude.
        **Wait, the spectrogram would represent the entire file, not just the chunk**
  - [ ] Apply a maximum filter on the spectrogram to local detect peaks
  - [ ] Compare amplituddes in the filtered spectrogram and the original spectrogram to recover original peaks
  - [ ] Discard peaks. **Should we take the top N peaks based on the song length or only take peaks above a certain threshold?**
  - [ ] Pair up peak points, hash them and place them into the database.
        ex: (point A frequency, point B frequency, time delta, point A time, track ID)

Steps to match a song:
 - **How much of the song will we have to process before we can say we have match?**
 - [ ] Same steps as above
 - [ ] Select all matching hashes from the database, group by track id
 - [ ] Pick the most accurate track
