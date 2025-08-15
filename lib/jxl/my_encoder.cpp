#include "../include/jxl/encode.h"
#include <vector>
#include <fstream>
#include <iostream>

int main() {
    const size_t width = 256;
    const size_t height = 256;
    const size_t num_channels = 3;
    const size_t bytes_per_pixel = 1;
    const size_t frame_bytes = width * height * num_channels * bytes_per_pixel;

    // Allocate two frames: red and green
    std::vector<uint8_t> frame1(frame_bytes);
    std::vector<uint8_t> frame2(frame_bytes);

    // Fill frame1 with red
    for (size_t i = 0; i < width * height; ++i) {
        frame1[i * 3 + 0] = 255; // R
        frame1[i * 3 + 1] = 0;   // G
        frame1[i * 3 + 2] = 0;   // B
    }

    // Fill frame2 with green
    for (size_t i = 0; i < width * height; ++i) {
        frame2[i * 3 + 0] = 0;
        frame2[i * 3 + 1] = 255;
        frame2[i * 3 + 2] = 0;
    }

    // Create encoder
    JxlEncoder* enc = JxlEncoderCreate(nullptr);

    // Basic image info
    JxlBasicInfo basic_info;
    JxlEncoderInitBasicInfo(&basic_info);
    basic_info.xsize = width;
    basic_info.ysize = height;
    basic_info.bits_per_sample = 8;
    basic_info.uses_original_profile = JXL_FALSE;
    basic_info.have_animation = false;
    basic_info.animation.tps_numerator = 1;
    basic_info.animation.tps_denominator = 1;
    basic_info.animation.num_loops = 0;
    JxlEncoderSetBasicInfo(enc, &basic_info);

    // Color encoding
    JxlColorEncoding color_encoding;
    JxlColorEncodingSetToSRGB(&color_encoding, JXL_FALSE);
    JxlEncoderSetColorEncoding(enc, &color_encoding);

    // Pixel format
    JxlPixelFormat pixel_format = {num_channels, JXL_TYPE_UINT8, JXL_NATIVE_ENDIAN, 0};

    // Add first frame
    {
        JxlEncoderFrameSettings* frame_settings = JxlEncoderFrameSettingsCreate(enc, nullptr);
        JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_RESAMPLING, 2);
        JxlEncoderAddImageFrame(frame_settings, &pixel_format,
                                frame1.data(), frame1.size());
    }

    // Add second frame
    {
        JxlEncoderFrameSettings* frame_settings = JxlEncoderFrameSettingsCreate(enc, nullptr);
        JxlBlendInfo *blend_info = new JxlBlendInfo;
        JxlEncoderInitBlendInfo(blend_info);
        std::cout << "JxlBlendInfo initialized." << std::endl;
        
        blend_info->blendmode = JxlBlendMode::JXL_BLEND_ADD;
        
        // init JxlFrameHeader
        JxlFrameHeader *frame_header = new JxlFrameHeader;
        JxlEncoderInitFrameHeader(frame_header);
        std::cout << "JxlFrameHeader initialized." << std::endl;

        frame_header->layer_info.blend_info = *blend_info;
        /* set frame header */

        // set frame_header as the header to use for the current frame
        JxlEncoderSetFrameHeader(frame_settings, frame_header);
        JxlEncoderAddImageFrame(frame_settings, &pixel_format,
                                frame2.data(), frame2.size());
    }

    // Close input so encoding can finish
    JxlEncoderCloseInput(enc);

    // Prepare output buffer
    std::vector<uint8_t> compressed(1024);
    uint8_t* next_out = compressed.data();
    size_t avail_out = compressed.size();

    // Process output
    JxlEncoderStatus status;
    do {
        status = JxlEncoderProcessOutput(enc, &next_out, &avail_out);
        if (status == JXL_ENC_NEED_MORE_OUTPUT) {
            size_t offset = next_out - compressed.data();
            compressed.resize(compressed.size() * 2);
            next_out = compressed.data() + offset;
            avail_out = compressed.size() - offset;
        }
    } while (status == JXL_ENC_NEED_MORE_OUTPUT);

    if (status != JXL_ENC_SUCCESS) {
        std::cerr << "Encoding failed!" << std::endl;
        JxlEncoderDestroy(enc);
        return 1;
    }

    // Save file
    size_t compressed_size = next_out - compressed.data();
    std::ofstream out("two_frames.jxl", std::ios::binary);
    out.write(reinterpret_cast<const char*>(compressed.data()), compressed_size);
    out.close();

    std::cout << "Saved two_frames.jxl with size " << compressed_size << " bytes\n";

    // Clean up
    JxlEncoderDestroy(enc);
    return 0;
}
