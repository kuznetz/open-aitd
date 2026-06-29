// #include <stdio.h>
// #include <stdint.h>
// #include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "wavwriter.h"
#include "music.h"

#include <vorbis/vorbisenc.h>

const int FINISH_TIME = 10;

void renderMusicWav(std::string outFilename) {
    const int BUFFER_SIZE = 1024;
    int sampleCount = 0;
    int16_t buffer[BUFFER_SIZE*2];

    FILE *f = fopen(outFilename.c_str(), "wb");
    if (!f) return;
    write_wav_header(f, 44100, 2, 16, 0); //BUFFER_SIZE * LEN

    while (true) {
        musicUpdate(0, buffer, BUFFER_SIZE);
        fwrite(buffer, sizeof(int16_t), BUFFER_SIZE*2, f);
        sampleCount += BUFFER_SIZE;
        if (isMusicFinished()) break;
    }
    for (int i = 0; i < FINISH_TIME; i++) {
        musicUpdate(0, buffer, BUFFER_SIZE);
        fwrite(buffer, sizeof(int16_t), BUFFER_SIZE*2, f);
        sampleCount += BUFFER_SIZE;            
    }

    //update_wav_total_samples(f, sampleCount);

    fclose(f);
}

void renderMusicOgg(std::string outFilename, float quality = 0.8f) {
    const int BUFFER_SIZE = 1024;
    int sampleCount = 0;
    int16_t buffer[BUFFER_SIZE * 2];
    
    FILE *f = fopen(outFilename.c_str(), "wb");
    if (!f) return;
    
    // Initialize Ogg/Vorbis structures
    ogg_stream_state os;
    ogg_page og;
    ogg_packet op;
    
    vorbis_info vi;
    vorbis_comment vc;
    vorbis_dsp_state vd;
    vorbis_block vb;
    
    // Setup encoding
    vorbis_info_init(&vi);
    
    // quality from 0.0 to 1.0, where 0.0 - low quality, 1.0 - high quality
    if (vorbis_encode_init_vbr(&vi, 2, 44100, quality) != 0) {
        fclose(f);
        return;
    }
    
    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc, "ENCODER", "open-AITD");
    
    vorbis_analysis_init(&vd, &vi);
    vorbis_block_init(&vd, &vb);
    
    ogg_stream_init(&os, rand()); // Initialize with random serial number
    
    // Prepare and write headers
    {
        ogg_packet header;
        ogg_packet header_comm;
        ogg_packet header_code;
        
        vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
        ogg_stream_packetin(&os, &header);
        ogg_stream_packetin(&os, &header_comm);
        ogg_stream_packetin(&os, &header_code);
        
        while (ogg_stream_flush(&os, &og) != 0) {
            fwrite(og.header, 1, og.header_len, f);
            fwrite(og.body, 1, og.body_len, f);
        }
    }
    
    // Main rendering loop
    while (true) {
        musicUpdate(0, buffer, BUFFER_SIZE);
        
        // Convert int16_t to float for Vorbis
        float** analysis_buffer = vorbis_analysis_buffer(&vd, BUFFER_SIZE);
        
        for (int i = 0; i < BUFFER_SIZE; i++) {
            analysis_buffer[0][i] = buffer[i * 2] / 32768.0f;
            analysis_buffer[1][i] = buffer[i * 2 + 1] / 32768.0f;
        }
        
        vorbis_analysis_wrote(&vd, BUFFER_SIZE);
        sampleCount += BUFFER_SIZE;
        
        if (isMusicFinished()) break;
    }
    
    // Additional finish time
    for (int i = 0; i < FINISH_TIME; i++) {
        musicUpdate(0, buffer, BUFFER_SIZE);
        
        float** analysis_buffer = vorbis_analysis_buffer(&vd, BUFFER_SIZE);
        
        for (int i = 0; i < BUFFER_SIZE; i++) {
            analysis_buffer[0][i] = buffer[i * 2] / 32768.0f;
            analysis_buffer[1][i] = buffer[i * 2 + 1] / 32768.0f;
        }
        
        vorbis_analysis_wrote(&vd, BUFFER_SIZE);
        sampleCount += BUFFER_SIZE;
    }
    
    // Finish encoding
    vorbis_analysis_wrote(&vd, 0);
    
    // Output encoded data
    while (vorbis_analysis_blockout(&vd, &vb) == 1) {
        vorbis_analysis(&vb, NULL);
        vorbis_bitrate_addblock(&vb);
        
        while (vorbis_bitrate_flushpacket(&vd, &op)) {
            ogg_stream_packetin(&os, &op);
            
            while (ogg_stream_pageout(&os, &og) != 0) {
                fwrite(og.header, 1, og.header_len, f);
                fwrite(og.body, 1, og.body_len, f);
            }
        }
    }
    
    // Final page
    while (ogg_stream_flush(&os, &og) != 0) {
        fwrite(og.header, 1, og.header_len, f);
        fwrite(og.body, 1, og.body_len, f);
    }
    
    // Cleanup
    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);
    
    fclose(f);
}

namespace AITDExtractor {
    void renderMusic(const void* musicData, const std::string outFilename, const bool ogg = true, const float quality = 0.8f) {
        initMusicDriver();
        playMusic(0, (char*)musicData);
        if (ogg) {
            renderMusicOgg(outFilename, quality);
        } else {
            renderMusicWav(outFilename);
        }
        destroyMusicDriver();
    }
}