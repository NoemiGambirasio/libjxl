#include <jxl/encode.h>
#include <iostream>

int main() {
    JxlEncoder* encoder = JxlEncoderCreate(nullptr);
    
    JxlBasicInfo *info = new JxlBasicInfo;
    JxlEncoderInitBasicInfo(info);

    info->have_container = 0;
    info->xsize = 800;
    info->ysize = 600;
    info->bits_per_sample = 8;
    info->exponent_bits_per_sample = 0;
    info->num_color_channels = 3;
    info->num_extra_channels = 0;
    info->alpha_bits = 0;
    


    JxlEncoderSetBasicInfo(encoder, info);


    // frame creation routine
	// creates new frame settings
	auto frame_settings = JxlEncoderFrameSettingsCreate(encoder, NULL);

	// sets the option to upscale at decode (use for low_res)
	JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_RESAMPLING, 2);
	JxlEncoderFrameSettingsSetOption(frame_settings, JXL_ENC_FRAME_SETTING_ALREADY_DOWNSAMPLED, 1);

	/* create JxlPixelFormat object and set right values */

	// init the JxlBlendInfo
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

    // set the pixel format
    JxlPixelFormat *pixel_format = new JxlPixelFormat;
    std::cout << "JxlPixelFormat initialized." << std::endl;

    char buffer[1024];
    size_t size = sizeof(buffer);

	// read a frame from the buffer with pixel_format format using the settings
	JxlEncoderAddImageFrame(frame_settings, pixel_format, buffer, size);
    std::cout << "Image frame added." << std::endl;

    // deallocate everything
    JxlEncoderDestroy(encoder);
    std::cout << "Encoder destroyed." << std::endl;
}