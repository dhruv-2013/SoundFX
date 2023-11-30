#ifndef WAV_H
#define WAV_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>


typedef struct {
    char chunk_id[4];
    uint32_t chunk_size;
    char type[4];
    char Chunk1_id[4];
    uint32_t Chunk1_size;
    uint16_t audio;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t ByteRate;
    uint16_t Align;
    uint16_t bps;
    char Chunk2_id[4];
    uint32_t Chunk2_size;
} wav_header_m;


typedef struct {
    FILE *file;
    char *FileNameM;
    uint32_t DataSize;
    uint32_t sample_rate;
    uint16_t num_channels;
    uint32_t bps;
} wav_file_m;


wav_file_m* Wave_Create(const char *FileNameM, uint32_t sample_rate, uint16_t num_channels, size_t num_samples, size_t bps);


void Wave_write(wav_file_m *WaveFile, uint32_t *data, size_t num_samples);


void Wave_close(wav_file_m *WaveFile);

#endif 

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "audio_i2s.h"



#define NUM_CHANNELS 1  
#define BPS 24
#define SAMPLE_RATE 48000
#define RECORD_DURATION 10
#define TRANSFER_RUNS 2000


void bin(uint8_t n) {
    //uint8_t i;
    // for (i = 1 << 7; i > 0; i = i >> 1)
    //     (n & i) ? printf("1") : printf("0");
    // for (i = 0; i < 8; i++) // LSB first
    //     (n & (1 << i)) ? printf("1") : printf("0");
}

void parsemem(void* virtual_address, int word_count) {
    //uint32_t *p = (uint32_t *)virtual_address;
    char *b = (char*)virtual_address;
    int offset;

    //uint32_t sample_count = 0;
    //uint32_t sample_value = 0;
    for (offset = 0; offset < word_count; offset++) {
        //sample_value = p[offset] & ((1<<18)-1);
        //sample_count = p[offset] >> 18;

        for (int i = 0; i < 4; i++) {
            bin(b[offset*4+i]);
            // printf(" ");
        }
        // printf(" -> [%d]: %02x (%dp)\n", sample_count, sample_value, sample_value*100/((1<<18)-1));
    }

}

int main() {
    printf("Entered main\n");

    uint32_t frames[TRANSFER_RUNS][TRANSFER_LEN] = {0};

    audio_i2s_t my_config;

    printf("mmapped address: %p\n", my_config.v_baseaddr);
    printf("Before writing to CR: %08x\n", audio_i2s_get_reg(&my_config, AUDIO_I2S_CR));
    audio_i2s_set_reg(&my_config, AUDIO_I2S_CR, 0x1);
    printf("After writing to CR: %08x\n", audio_i2s_get_reg(&my_config, AUDIO_I2S_CR));
    printf("SR: %08x\n", audio_i2s_get_reg(&my_config, AUDIO_I2S_SR));
    printf("Key: %08x\n", audio_i2s_get_reg(&my_config, AUDIO_I2S_KEY));
    printf("Before writing to gain: %08x\n", audio_i2s_get_reg(&my_config, AUDIO_I2S_GAIN));
    audio_i2s_set_reg(&my_config, AUDIO_I2S_GAIN, 0x1);
    printf("After writing to gain: %08x\n", audio_i2s_get_reg(&my_config, AUDIO_I2S_GAIN));
    printf("Initialized audio_i2s\n");
    printf("Starting audio_i2s_recv\n");
    for (int i = 0; i < TRANSFER_RUNS; i++) {
        int32_t *samples = audio_i2s_recv(&my_config);
        memcpy(frames[i], samples, TRANSFER_LEN*sizeof(int32_t));
    }

    for (int i = 0; i < TRANSFER_RUNS; i++) {
        // printf("Frame %d:\n", i);
        parsemem(frames[i], TRANSFER_LEN);
    }
    
    wav_file_m* WaveFile = Wave_Create("mic.wav", SAMPLE_RATE, NUM_CHANNELS, TRANSFER_LEN * TRANSFER_RUNS, BPS);
    for (int i = 0; i < TRANSFER_RUNS; i++) {
        Wave_write(WaveFile, frames[i], TRANSFER_LEN);
    }
    Wave_close(WaveFile);

    audio_i2s_release(&my_config);
    return 0;
}


wav_file_m* Wave_Create(const char *FileNameM, uint32_t sample_rate, uint16_t num_channels, size_t num_samples, size_t bps) {
   
    wav_file_m *WaveFile = (wav_file_m *)malloc(sizeof(wav_file_m));
    WaveFile->FileNameM = strdup(FileNameM);
    WaveFile->sample_rate = sample_rate;
    WaveFile->num_channels = num_channels;
    WaveFile->bps = bps;  // Set to 32 bits per sample
    WaveFile->DataSize = 0;

   
    WaveFile->file = fopen(FileNameM, "wb");

    // Write the initial WAV header with placeholders for final values
    wav_header_m header;
    memset(&header, 0, sizeof(wav_header_m));
    memcpy(header.chunk_id, "RIFF", 4);
    memcpy(header.type, "WAVE", 4);
    memcpy(header.Chunk1_id, "fmt ", 4);
    header.Chunk1_size = 16; 
    header.audio = 1; 
    header.num_channels = num_channels;
    header.sample_rate = sample_rate;
    header.ByteRate = sample_rate * num_channels * (WaveFile->bps/8);  
    header.Align = num_channels * (WaveFile->bps/8); 
    header.bps = bps;  // 32 bits per sample
    header.Chunk2_id[0] = 'd';
    header.Chunk2_id[1] = 'a';
    header.Chunk2_id[2] = 't';
    header.Chunk2_id[3] = 'a';
    header.Chunk2_size = num_channels * (WaveFile->bps/8) * num_samples; 
    header.chunk_size = 4 + (header.Chunk1_size + 8) + (header.Chunk2_size + 8);  

    size_t written = fwrite(&header, 1, sizeof(wav_header_m), WaveFile->file);
    if (written != sizeof(wav_header_m)) {
        fprintf(stderr, "Error writing WAV header to %s\n", FileNameM);
        fclose(WaveFile->file);
        free(WaveFile->FileNameM);
        free(WaveFile);
        return NULL;
    }

    return WaveFile;
}

void Wave_write(wav_file_m *WaveFile, uint32_t *data, size_t num_samples) {
 

    size_t written = fwrite(data, WaveFile->bps/8, num_samples - 2, WaveFile->file);
    if (written != num_samples) {
        fprintf(stderr, "Failed to write audio data\n");
    } else {
       
        WaveFile->DataSize += written * sizeof(int32_t);
    }
}

void Wave_close(wav_file_m *WaveFile) {
    fseek(WaveFile->file, 0, SEEK_SET);

   

   // size_t written = fwrite(&header, 1, sizeof(wav_header_m), WaveFile->file);
    fclose(WaveFile->file);

    free(WaveFile->FileNameM);
    free(WaveFile);
}