#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    char riff[4];        // "RIFF"
    uint32_t size;
    char wave[4];        // "WAVE"
    char fmt[4];         // "fmt "
    uint32_t fmt_size;   // 16
    uint16_t format;     // 1 = PCM
    uint16_t channels;   // 2
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits;
    char data[4];        // "data"
    uint32_t data_size;
} wav_header;

// Function to create and write WAV file header
inline void write_wav_header(FILE *f, int sample_rate, int channels, int bits_per_sample, int total_samples=0) {
    wav_header header;
    
    // Fill identifiers
    memcpy(header.riff, "RIFF", 4);
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt,  "fmt ", 4);
    memcpy(header.data, "data", 4);
    
    // Calculate parameters
    header.fmt_size    = 16;
    header.format      = 1;  // PCM
    header.channels    = channels;
    header.sample_rate = sample_rate;
    header.bits        = bits_per_sample;
    header.block_align = channels * bits_per_sample / 8;
    header.byte_rate   = sample_rate * header.block_align;
    header.data_size   = total_samples * header.block_align;
    header.size        = header.data_size + sizeof(wav_header) - 8;
    
    // Write header to file
    fwrite(&header, sizeof(header), 1, f);
}

inline void update_wav_total_samples(FILE *f, int total_samples) {
    wav_header header;
    long current_pos = ftell(f);
    
    // Go to beginning of file to read existing header
    fseek(f, 0, SEEK_SET);
    fread(&header, sizeof(header), 1, f);
    
    // Update fields related to total_samples
    header.data_size = total_samples * header.block_align;
    header.size = header.data_size + sizeof(wav_header) - 8;
    
    // Write updated header back to file
    fseek(f, 0, SEEK_SET);
    fwrite(&header, sizeof(header), 1, f);
    
    // Return to previous position in file
    fseek(f, current_pos, SEEK_SET);
}