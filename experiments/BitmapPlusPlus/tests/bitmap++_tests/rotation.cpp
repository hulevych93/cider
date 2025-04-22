#include <iostream>
#include "BitmapPlusPlus.h"

using namespace bmp::BitmapPlusPlusHooked;

int test_rotation(void) {
  try {
    Bitmap image;

    // Load the original bitmap

    const auto p1 = std::filesystem::path(ROOT_DIR) / "images" / "penguin.bmp";
    const auto s1 = p1.string();
    image.load(s1.c_str());

    // Test vertical flip
    Bitmap flipped_v = image.flip_v();

    const auto p2 = std::filesystem::path(ROOT_DIR) / "images" / "rotated" /
                    "penguin_flipped_v.bmp";
    const auto s2 = p2.string();
    flipped_v.save(s2.c_str());
    std::cout << "Vertical flip saved as penguin_flipped_v.bmp" << std::endl;

    // Test horizontal flip
    Bitmap flipped_h = image.flip_h();
    const auto p3 = std::filesystem::path(ROOT_DIR) / "images" / "rotated" /
                    "penguin_flipped_h.bmp";
    const auto s3 = p3.string();
    flipped_h.save(s3.c_str());
    std::cout << "Horizontal flip saved as penguin_flipped_h.bmp" << std::endl;

    // Test rotate 90 degrees to the right
    Bitmap rotated_right = image.rotate_90_right();
    const auto p4 = std::filesystem::path(ROOT_DIR) / "images" / "rotated" /
                    "penguin_rotated_right.bmp";
    const auto s4 = p4.string();
    rotated_right.save(s4.c_str());
    std::cout << "Rotated 90 degrees right saved as penguin_rotated_right.bmp"
              << std::endl;

    // Test rotate 90 degrees to the left
    Bitmap rotated_left = image.rotate_90_left();
    const auto p5 = std::filesystem::path(ROOT_DIR) / "images" / "rotated" /
                    "penguin_rotated_left.bmp";
    const auto s5 = p5.string();
    rotated_left.save(s5.c_str());
    std::cout << "Rotated 90 degrees left saved as penguin_rotated_left.bmp"
              << std::endl;

    return EXIT_SUCCESS;
  } catch (const bmp::Exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
