#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

// TODO:
// create a spectrogram of the entire file
// run a maximum filter on the spectrogram
// Compare amplituddes in the filtered spectrogram and the original spectrogram to recover original
// peaks Discard peaks

typedef float complex Z;
typedef long long unsigned int u64;
typedef unsigned int u32;
typedef unsigned char u8;

void write_png(const char* path, u8* pixels, int width, int height) {
  stbi_write_png(path, width, height, 3, pixels, width * 3);
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

void compute_fft(float* input, int input_size, float** output, int* output_size) {
  int N = powf(2, ceilf(log2f(input_size)));  // The input size must be a power of 2
  Z* bins = (Z*)calloc(N, sizeof(Z));         // Frequency bins

  // Use the bit-reversal permutation algorithm to split the samples into even/odd parts
  // The first half will be [samples[0], samples[2], samples[4], ...]
  // The second half will be [samples[1], samples[3], samples[5], ...]
  int bits_needed = log2(N);
  for (int i = 0; i < N; i++) {
    u32 idx = i, reversed = 0;
    for (int j = 0; j < bits_needed; j++) {
      reversed <<= 1;
      reversed |= (idx & 1);
      idx >>= 1;
    }
    bins[reversed] = input[i] + 0.0I;
  }

  // Recursing bottom-up
  for (int len = 2; len <= N; len *= 2) {  // Each recursion level
    int half = len / 2;
    for (int i = 0; i < N; i += len) {  // Each block of the length
      for (int k = 0; k < half; k++) {  // Sum the even/odd portions in place
        Z root = cexpf(-I * 2.0 * M_PI * k / (float)len);
        Z even = bins[i + k];
        Z odd = bins[i + k + half];

        bins[i + k] = even + root * odd;
        bins[i + k + half] = even - root * odd;
      }
    }
  }

  *output_size = N / 2;  // The last half is just a mirror of the first half
  *output = (float*)malloc(*output_size * sizeof(float));

  // Convert the output to amplitudes (in decibels)
  for (int i = 0; i < *output_size; i++) {
    float real = crealf(bins[i]), imag = cimagf(bins[i]);
    float raw = sqrtf(real * real + imag * imag);
    (*output)[i] = 20.0 * log10f(raw + 1e-9);
  }
  free(bins);
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
    int amplitudes_size = 0;
    float* amplitudes = NULL;

    memcpy(sliding_window, samples + i, window_size * sizeof(float));
    apply_hamming_window(sliding_window, window_size);
    compute_fft(sliding_window, window_size, &amplitudes, &amplitudes_size);

    // Frequency: idx * sample_rate / (amplitudes_size * 2);
    free(amplitudes);
  }

  printf("Ran FFT...\n");

  free(sliding_window);
  ma_free(samples, NULL);
  return 0;
}
