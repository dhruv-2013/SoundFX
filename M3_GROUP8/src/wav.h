#ifndef WAV_H
#define WAV_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// Define a structure to hold WAV file header information
typedef struct {
    char chunk_id[4];
    uint32_t chunk_size;
    char format[4];
    char subchunk1_id[4];
    uint32_t subchunk1_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char subchunk2_id[4];
    uint32_t subchunk2_size;
} wav_header_m;

// Define a structure to represent a WAV file
typedef struct {
    FILE *file;
    char *FileNameM;
    uint32_t data_size;
    uint32_t sample_rate;
    uint16_t num_channels;
    uint32_t bits_per_sample;
} wav_file_m;

// Function to create a new WAV file
wav_file_m* Wave_Create(const char *FileNameM, uint32_t sample_rate, uint16_t num_channels, size_t num_samples, size_t bits_per_sample);

// Function to write data to the WAV file
void Wave_write(wav_file_m *WaveFile, uint32_t *data, size_t num_samples);

// Function to close the WAV file and update the header
void Wave_close(wav_file_m *WaveFile);

#endif 