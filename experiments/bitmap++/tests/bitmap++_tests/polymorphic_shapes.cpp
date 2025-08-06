#include <filesystem>
#include <iostream>
#include "BitmapPlusPlus.h"

using namespace bmp::BitmapPlusPlusHooked;

namespace {
struct Shape {
  virtual ~Shape() = default;

  int x, y;
  Pixel color;

  Shape(int x, int y, Pixel color) : x(x), y(y), color(color) {}

  virtual void draw(Bitmap& image) = 0;
};

struct Rectangle : Shape {
  int width, height;

  Rectangle(int x, int y, int w, int h, Pixel color)
      : width(w), height(h), Shape(x, y, color) {}

  void draw(Bitmap& image) override {
    image.fill_rect(x, y, width, height, color);
  }
};

struct Triangle : Shape {
  int x2, y2, x3, y3;

  Triangle(int x1, int y1, int x2, int y2, int x3, int y3, Pixel color)
      : x2(x2), y2(y2), x3(x3), y3(y3), Shape(x1, y1, color) {}

  void draw(Bitmap& image) override {
    image.fill_triangle(x, y, x2, y2, x3, y3, color);
  }
};

struct Circle : Shape {
  int radius;

  Circle(int x, int y, int radius, Pixel color)
      : radius(radius), Shape(x, y, color) {}

  void draw(Bitmap& image) override { image.fill_circle(x, y, radius, color); }
};

}  // namespace

int test_polymorphic_shapes() {
  try {
    Bitmap image(640, 256);
    Pixel background_color{Silver};
    image.clear(background_color);

    std::vector<Shape*> shapes{
        new Rectangle(20, 20, 180, 180, makePixel(0xa31d3a)),
        new Triangle(310, 20, 230, 200, 400, 200, makePixel(0x1a5096)),
        new Circle(500, 110, 90, makePixel(0x228035))};

    for (Shape* shape : shapes) {
      shape->draw(image);
      delete shape;
    }

    auto p1 = std::filesystem::path(BIN_DIR) / "polymorphic_shapes.bmp";
    const auto s1 = p1.string();
    image.save(s1.c_str());

    return EXIT_SUCCESS;
  } catch (const bmp::Exception& e) {
    std::cerr << "[BMP ERROR]: " << e.what() << '\n';
    return EXIT_FAILURE;
  }
}
