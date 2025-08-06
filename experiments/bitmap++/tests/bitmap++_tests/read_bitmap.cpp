#include <filesystem>
#include <iostream>
#include "BitmapPlusPlus.h"

using namespace bmp::BitmapPlusPlusHooked;

int test_read_bitmap() {
  try {
    Bitmap image;

    // Load penguin.bmp bitmap
    const auto p0 = std::filesystem::path(ROOT_DIR) / "images" / "penguin.bmp";
    const auto s0 = p0.string();
    image.load(s0.c_str());

    for (std::int32_t y = 0; y < 2; ++y) {
      for (std::int32_t x = 0; x < 2; ++x) {
        image.set(x, y, Black);
      }
    }

    // Save
    const auto p1 = std::filesystem::path(BIN_DIR) / "modified-penguin.bmp";
    const auto s1 = p1.string();
    image.save(s1.c_str());

    return EXIT_SUCCESS;
  } catch (const bmp::Exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
