#include <math.h>
#include <stdlib.h>
#include <string.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

// TODO:
// answer the practical questions
// implement the fast fourrier transform algorithm
// create a spectrogram of the entire file
// run a maximum filter on the spectrogram
// Compare amplituddes in the filtered spectrogram and the original spectrogram to recover original
// peaks Discard peaks

typedef long long unsigned int u64;
typedef unsigned char u8;

void write_png(const char* path, u8* pixels, int width, int height) {
  int stride = width * 3 * sizeof(u8);
  stbi_write_png(path, width, height, 3, pixels, stride);
}

ma_result read_samples(const char* path, float** samples, u64* sampling_rate, u64* total_samples) {
  ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 2, 44100);
  ma_result ret = ma_decode_file(path, &config, total_samples, (void**)samples);
  *sampling_rate = config.sampleRate;
  return ret;
}

// A Hamming window tapers the samples to near zero near the edges of the buffer
// to reduce the discontinuities between neighboring windows, reducing spectral leakage.
void apply_hamming_window(float* samples, int num_samples) {
  float a0 = 0.53836, a1 = 0.46164;
  float factor = 2 * M_PI / (float)num_samples - 1;
  for (int i = 0; i < num_samples; i++) {
    samples[i] = a0 - a1 * cos(samples[i] * factor);
  }
}

int main() {
  const char* path = "../data/Just the two of us - Grover Washington Jr.mp3";

  float* samples = NULL;
  u64 sampling_rate = 0, total_samples = 0;

  if (read_samples(path, &samples, &sampling_rate, &total_samples) != MA_SUCCESS) {
    printf("Failed to read the file\n");
    return -1;
  }

  printf("Read the samples...\n");

  int window_size = 10 * sampling_rate;
  int step = window_size / 2;
  float* sliding_window = (float*)malloc(window_size * sizeof(float));

  for (int i = 0; i < total_samples - window_size; i += step) {
    memcpy(sliding_window, samples + i, window_size * sizeof(float));
    apply_hamming_window(sliding_window, window_size);
  }

  printf("Ran the sliding window...\n");

  free(sliding_window);
  ma_free(samples, NULL);
  return 0;
}
