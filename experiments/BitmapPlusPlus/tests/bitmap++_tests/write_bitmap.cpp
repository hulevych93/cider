#include <filesystem>
#include <iostream>
#include <random>
#include "BitmapPlusPlus.h"

using namespace bmp::BitmapPlusPlusHooked;

static Pixel random_color() {
  static std::random_device seed{};
  static std::default_random_engine engine{seed()};
  std::uniform_int_distribution<std::int32_t> dist(0, 255);
  Pixel color{};
  color.r = dist(engine);
  color.g = dist(engine);
  color.b = dist(engine);
  return color;
}

int test_write_bitmap() {
  try {
    // Create a 8x8 bitmap
    Bitmap image(2, 2);

    // Assign a random color to each pixel in the image
    const auto size = image.size();
    for (int i = 0; i < size; ++i) {
      image[i] = random_color();
    }

    // Save bitmap to new file image.bmp
    auto p1 = std::filesystem::path(BIN_DIR) / "image.bmp";
    const auto s1 = p1.string();
    image.save(s1.c_str());

    // And Voila!
    return EXIT_SUCCESS;
  } catch (const bmp::Exception& e) {
    std::cerr << "[BMP ERROR]: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
