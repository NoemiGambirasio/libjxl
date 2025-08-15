#include "lib/jxl/image_bundle.h"
#include "lib/jxl/enc_frame.h"    
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION

jxl::ImageBundle LoadPNGToImageBundle(const char* filename, JxlMemoryManager* memory_manager) {
    int w, h, c;
    unsigned char* data = stbi_load(filename, &w, &h, &c, 4); // force RGBA
    if (!data) throw std::runtime_error("Failed to load PNG");

    jxl::ImageMetadata metadata; // set width, height, extra channels etc. if needed
    jxl::ImageBundle ib(memory_manager, &metadata);

    auto color = jxl::Image3F::Create(memory_manager, w, h).value();
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = (y * w + x) * 4;
            color.Plane(0).Row(y)[x] = data[idx + 0] / 255.0f; // R
            color.Plane(1).Row(y)[x] = data[idx + 1] / 255.0f; // G
            color.Plane(2).Row(y)[x] = data[idx + 2] / 255.0f; // B
        }
    }

    ib.SetFromImage(std::move(color), jxl::ColorEncoding::SRGB());
    stbi_image_free(data);
    return ib;
}


bool LoadPNGToImageBundle(const std::string& filename, ImageBundle& img_bundle) {
    JxlMemoryManager* memory_manager = img_bundle.memory_manager();
    try {
        ImageBundle bundle = LoadPngIntoImageBundle(filename, memory_manager);
        img_bundle = std::move(bundle);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading PNG: " << e.what() << std::endl;
        return false;
    }
}

// Example usage of encoding two images as one JXL file

bool EncodeTwoImagesAsOneJXL(const std::string& png1, const std::string& png2,
                             const std::string& out_filename) {
    using namespace jxl;

    ThreadPool* pool = nullptr; // Use default, or create with RunOnPool
    CompressParams cparams;

    JxlMemoryManager memory_manager;
    CodecMetadata metadata;
    ImageBundle img1(&memory_manager, &metadata.m);
    ImageBundle img2(&memory_manager, &metadata.m);
    BitWriter writer(&memory_manager);


    if (!LoadPNGToImageBundle(png1, img1) || !LoadPNGToImageBundle(png2, img2)) {
        std::cerr << "Failed to load PNGs\n";
        return false;
    }

    // Step 1: Encode first image
    FrameInfo fi1;
    fi1.blend = false;
    fi1.is_last = false;

    if (!EncodeFrame(&memory_manager, cparams, fi1, &metadata,
                     img1, JxlCmsInterface(), pool, &writer, nullptr)) {
        std::cerr << "Failed to encode first frame\n";
        return false;
    }

    // Step 2: Encode second image, adding it
    FrameInfo fi2;
    fi2.blend = true;
    fi2.blendmode = BlendMode::kAdd; // <- addition
    fi2.is_last = true;

    if (!EncodeFrame(&memory_manager, cparams, fi2, &metadata,
                     img2, JxlCmsInterface(), pool, &writer, nullptr)) {
        std::cerr << "Failed to encode second frame\n";
        return false;
    }

    // Write to file
    std::ofstream out(out_filename, std::ios::binary);
    if (!out) return false;
    auto data = writer.GetSpan();
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

int main() {
    if (EncodeTwoImagesAsOneJXL("../../low_res.png", "../../high_res.png", "result.jxl")) {
        std::cout << "Successfully encoded added image.jxl\n";
    } else {
        std::cerr << "Failed to encode\n";
    }
}
