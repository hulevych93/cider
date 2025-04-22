#include <filesystem>
#include <iostream>
#include "BitmapPlusPlus.h"

using namespace bmp::BitmapPlusPlusHooked;

int test_chess_board() {
  try {
    // 8x8 chess board
    Bitmap image(640, 640);
    const std::size_t board_dims = 8;
    const std::int32_t rect_w = image.width() / board_dims;
    const std::int32_t rect_h = image.height() / board_dims;

    // Iterate over rects
    bool is_white = true;
    for (std::size_t x = 0; x < image.width(); x += rect_w) {
      for (std::size_t y = 0; y < image.height(); y += rect_h) {
        const Pixel color = is_white ? White : Black;
        // Fill rect
        image.fill_rect(x, y, rect_w, rect_h, color);
        // Next rect in will be the opposite color
        is_white = !is_white;
      }
      is_white = !is_white;
    }

    // Save bitmap to file
    const auto p1 = std::filesystem::path(BIN_DIR) / "chess_board.bmp";
    const auto s1 = p1.string();
    image.save(s1.c_str());

    return EXIT_SUCCESS;
  } catch (const bmp::Exception& e) {
    std::cerr << "[BMP ERROR]: " << e.what() << '\n';
    return EXIT_FAILURE;
  }
}
