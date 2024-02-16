#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE 0

#include <string.h>  // because Borland's STL is braindead, we have to include <string.h> _before_ <string> in order to get memcpy

#include "test.hpp"

#include "writer_string.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <sstream>

#include <algorithm>
#include <string>

#ifndef PUGIXML_NO_EXCEPTIONS
#include <stdexcept>
#endif

// for unlink
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

using namespace pugi::PugixmlHooked;

static bool load_file_in_memory(const char* path, char*& data, size_t& size) {
  FILE* file = fopen(path, "rb");
  if (!file)
    return false;

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  CHECK(length >= 0);
  size = static_cast<size_t>(length);
  data = new char[size];

  CHECK(fread(data, 1, size, file) == size);
  fclose(file);

  return true;
}

static bool test_file_contents(const char* path,
                               const char* data,
                               size_t size) {
  char* fdata;
  size_t fsize;
  if (!load_file_in_memory(path, fdata, fsize))
    return false;

  bool result = (size == fsize && memcmp(data, fdata, size) == 0);

  delete[] fdata;

  return result;
}

TEST(document_create_empty) {
  xml_document doc;
  CHECK_NODE(doc, STR(""));
}

TEST(document_create) {
  xml_document doc;
  doc.append_child().set_name(STR("node"));
  CHECK_NODE(doc, STR("<node/>"));
}

TEST(document_load_string) {
  xml_document doc;

  CHECK(doc.load_string(STR("<node/>")).status == status_ok);
  CHECK_NODE(doc, STR("<node/>"));
}

TEST(document_load_file) {
  xml_document doc;

  CHECK(doc.load_file("tests/data/small.xml").status == status_ok);
  CHECK_NODE(doc, STR("<node/>"));
}

TEST(document_load_file_empty) {
  xml_document doc;

  CHECK(doc.load_file("tests/data/empty.xml").status ==
        status_no_document_element);
  CHECK(!doc.first_child());
}

TEST(document_load_file_large) {
  xml_document doc;

  CHECK(doc.load_file(L"tests/data/large.xml").status == status_ok);

  std::basic_string<char_t> str;
  str += STR("<node>");
  for (int i = 0; i < 10000; ++i)
    str += STR("<node/>");
  str += STR("</node>");

  CHECK_NODE(doc, str.c_str());
}

TEST(document_load_file_error) {
  xml_document doc;

  CHECK(doc.load_file("filedoesnotexist").status == status_file_not_found);
}

TEST(document_load_file_out_of_memory) {
  test_runner::_memory_fail_threshold = 1;

  xml_document doc;
  CHECK_ALLOC_FAIL(CHECK(doc.load_file("tests/data/small.xml").status ==
                         status_out_of_memory));
}

TEST(document_load_file_out_of_memory_file_leak) {
  test_runner::_memory_fail_threshold = 1;

  xml_document doc;

  for (int i = 0; i < 256; ++i)
    CHECK_ALLOC_FAIL(CHECK(doc.load_file("tests/data/small.xml").status ==
                           status_out_of_memory));

  test_runner::_memory_fail_threshold = 0;

  CHECK(doc.load_file("tests/data/small.xml").status == status_ok);
  CHECK_NODE(doc, STR("<node/>"));
}

TEST(document_load_file_wide_out_of_memory_file_leak) {
  test_runner::_memory_fail_threshold = 256;

  xml_document doc;

  for (int i = 0; i < 256; ++i)
    CHECK_ALLOC_FAIL(CHECK(doc.load_file("tests/data/small.xml").status ==
                           status_out_of_memory));

  test_runner::_memory_fail_threshold = 0;

  CHECK(doc.load_file("tests/data/small.xml").status == status_ok);
  CHECK_NODE(doc, STR("<node/>"));
}

TEST(document_load_file_error_previous) {
  xml_document doc;
  CHECK(doc.load_string(STR("<node/>")).status == status_ok);
  CHECK(!doc.first_child().empty());

  CHECK(doc.load_file("filedoesnotexist").status == status_file_not_found);
  CHECK(!doc.first_child());
}

TEST(document_load_file_wide_ascii) {
  xml_document doc;

  CHECK(doc.load_file("tests/data/small.xml").status == status_ok);
  CHECK_NODE(doc, STR("<node/>"));
}

#if !defined(__DMC__) && !defined(__MWERKS__) &&          \
    !(defined(__MINGW32__) && defined(__STRICT_ANSI__) && \
      !defined(__MINGW64_VERSION_MAJOR)) &&               \
    !defined(__BORLANDC__)
TEST(document_load_file_wide_unicode) {
  xml_document doc;

  CHECK(doc.load_file(L"tests/data/\x0442\x0435\x0441\x0442.xml").status ==
        status_ok);
  CHECK_NODE(doc, STR("<node/>"));
}
#endif

TEST(document_load_file_wide_out_of_memory) {
  test_runner::_memory_fail_threshold = 1;

  xml_document doc;

  xml_parse_result result;
  result.status = status_out_of_memory;
  CHECK_ALLOC_FAIL(result = doc.load_file("tests/data/small.xml"));

  CHECK(result.status == status_out_of_memory ||
        result.status == status_file_not_found);
}

#if defined(__linux__) || defined(__APPLE__)
TEST(document_load_file_special_folder) {
  xml_document doc;
  xml_parse_result result = doc.load_file(".");
  CHECK(result.status == status_io_error);
}
#endif

#if defined(__linux__)
TEST(document_load_file_special_device) {
  xml_document doc;
  xml_parse_result result = doc.load_file("/dev/tty");
  CHECK(result.status == status_file_not_found ||
        result.status == status_io_error);
}
#endif

TEST_XML(document_save_bom, "<n/>") {
  unsigned int flags = format_no_declaration | format_raw | format_write_bom;

  // specific encodings
  CHECK(test_save_narrow(doc, flags, encoding_utf8, "\xef\xbb\xbf<n/>", 7));
  CHECK(test_save_narrow(doc, flags, encoding_utf16_be,
                         "\xfe\xff\x00<\x00n\x00/\x00>", 10));
  CHECK(test_save_narrow(doc, flags, encoding_utf16_le,
                         "\xff\xfe<\x00n\x00/\x00>\x00", 10));
  CHECK(test_save_narrow(
      doc, flags, encoding_utf32_be,
      "\x00\x00\xfe\xff\x00\x00\x00<\x00\x00\x00n\x00\x00\x00/\x00\x00\x00>",
      20));
  CHECK(test_save_narrow(
      doc, flags, encoding_utf32_le,
      "\xff\xfe\x00\x00<\x00\x00\x00n\x00\x00\x00/\x00\x00\x00>\x00\x00\x00",
      20));
  CHECK(test_save_narrow(doc, flags, encoding_latin1, "<n/>", 4));

  // encodings synonyms
  CHECK(save_narrow(doc, flags, encoding_utf16) ==
        save_narrow(
            doc, flags,
            (is_little_endian() ? encoding_utf16_le : encoding_utf16_be)));
  CHECK(save_narrow(doc, flags, encoding_utf32) ==
        save_narrow(
            doc, flags,
            (is_little_endian() ? encoding_utf32_le : encoding_utf32_be)));

  size_t wcharsize = sizeof(wchar_t);
  CHECK(save_narrow(doc, flags, encoding_wchar) ==
        save_narrow(doc, flags,
                    (wcharsize == 2 ? encoding_utf16 : encoding_utf32)));
}

struct temp_file {
  char path[512];

  temp_file() {
    static int index = 0;

#if __cplusplus >= 201103 || \
    defined(__APPLE__)  // Xcode 14 warns about use of sprintf in C++98 builds
    snprintf(path, sizeof(path), "%stempfile%d", test_runner::_temp_path,
             index++);
#else
    sprintf(path, "%stempfile%d", test_runner::_temp_path, index++);
#endif
  }

  ~temp_file() {
#ifndef _WIN32_WCE
    CHECK(unlink(path) == 0);
#endif
  }
};

TEST_XML(document_save_file, "<node/>") {
  temp_file f;

  CHECK(doc.save_file(f.path));

  CHECK(doc.load_file(f.path, parse_default | parse_declaration).status ==
        status_ok);
  CHECK_NODE(doc, STR("<?xml version=\"1.0\"?><node/>"));
}

TEST_XML(document_save_file_wide, "<node/>") {
  temp_file f;

  // widen the path
  wchar_t wpath[sizeof(f.path)];
  std::copy(f.path, f.path + strlen(f.path) + 1, wpath + 0);

  CHECK(doc.save_file(wpath));

  CHECK(doc.load_file(f.path, parse_default | parse_declaration).status ==
        status_ok);
  CHECK_NODE(doc, STR("<?xml version=\"1.0\"?><node/>"));
}

TEST_XML(document_save_file_error, "<node/>") {
  CHECK(!doc.save_file("tests/data/unknown/output.xml"));
}

TEST_XML(document_save_file_text, "<node/>") {
  temp_file f;

  CHECK(doc.save_file(f.path, STR(""),
                      format_no_declaration | format_save_file_text));
  CHECK(test_file_contents(f.path, "<node />\n", 9) ||
        test_file_contents(f.path, "<node />\r\n", 10));

  CHECK(doc.save_file(f.path, STR(""), format_no_declaration));
  CHECK(test_file_contents(f.path, "<node />\n", 9));
}

TEST_XML(document_save_file_wide_text, "<node/>") {
  temp_file f;

  // widen the path
  wchar_t wpath[sizeof(f.path)];
  std::copy(f.path, f.path + strlen(f.path) + 1, wpath + 0);

  CHECK(doc.save_file(wpath, STR(""),
                      format_no_declaration | format_save_file_text));
  CHECK(test_file_contents(f.path, "<node />\n", 9) ||
        test_file_contents(f.path, "<node />\r\n", 10));

  CHECK(doc.save_file(wpath, STR(""), format_no_declaration));
  CHECK(test_file_contents(f.path, "<node />\n", 9));
}

TEST_XML(document_save_file_leak, "<node/>") {
  temp_file f;

  for (int i = 0; i < 256; ++i)
    CHECK(doc.save_file(f.path));
}

TEST_XML(document_save_file_wide_leak, "<node/>") {
  temp_file f;

  // widen the path
  wchar_t wpath[sizeof(f.path)];
  std::copy(f.path, f.path + strlen(f.path) + 1, wpath + 0);

  for (int i = 0; i < 256; ++i)
    CHECK(doc.save_file(wpath));
}

TEST(document_parse_result_bool) {
  xml_parse_result result;

  result.status = status_ok;
  CHECK(result.status == status_ok);

  for (int i = 1; i < 20; ++i) {
    result.status = static_cast<xml_parse_status>(i);
    CHECK(result.status != status_ok);
  }
}

TEST(document_parse_result_description) {
  xml_parse_result result;

  for (int i = 0; i < 20; ++i) {
    result.status = static_cast<xml_parse_status>(i);
  }
}

TEST(document_load_fail) {
  xml_document doc;
  CHECK(doc.load_string(STR("<foo><bar/>")).status != status_ok);
  CHECK(!doc.child(STR("foo")).child(STR("bar")).empty());
}

inline void check_utftest_document(const xml_document& doc) {
  // ascii text
  CHECK_STRING(doc.last_child().first_child().name(), STR("English"));

  // check that we have parsed some non-ascii text
  CHECK(static_cast<unsigned int>(doc.last_child().last_child().name()[0]) >=
        0x80);

  // check magic string
  const char_t* v =
      doc.last_child().child(STR("Heavy")).previous_sibling().child_value();

#ifdef PUGIXML_WCHAR_MODE
  CHECK(v[0] == 0x4e16 && v[1] == 0x754c && v[2] == 0x6709 && v[3] == 0x5f88 &&
        v[4] == 0x591a && v[5] == wchar_cast(0x8bed) &&
        v[6] == wchar_cast(0x8a00));

  // last character is a surrogate pair
  size_t wcharsize = sizeof(wchar_t);

  CHECK(wcharsize == 2
            ? (v[7] == wchar_cast(0xd852) && v[8] == wchar_cast(0xdf62))
            : (v[7] == wchar_cast(0x24b62)));
#else
  // unicode string
  CHECK_STRING(v,
               "\xe4\xb8\x96\xe7\x95\x8c\xe6\x9c\x89\xe5\xbe\x88\xe5\xa4\x9a"
               "\xe8\xaf\xad\xe8\xa8\x80\xf0\xa4\xad\xa2");
#endif
}

TEST(document_load_file_convert_auto) {
  const char* files[] = {"tests/data/utftest_utf16_be.xml",
                         "tests/data/utftest_utf16_be_bom.xml",
                         "tests/data/utftest_utf16_be_nodecl.xml",
                         "tests/data/utftest_utf16_le.xml",
                         "tests/data/utftest_utf16_le_bom.xml",
                         "tests/data/utftest_utf16_le_nodecl.xml",
                         "tests/data/utftest_utf32_be.xml",
                         "tests/data/utftest_utf32_be_bom.xml",
                         "tests/data/utftest_utf32_be_nodecl.xml",
                         "tests/data/utftest_utf32_le.xml",
                         "tests/data/utftest_utf32_le_bom.xml",
                         "tests/data/utftest_utf32_le_nodecl.xml",
                         "tests/data/utftest_utf8.xml",
                         "tests/data/utftest_utf8_bom.xml",
                         "tests/data/utftest_utf8_nodecl.xml"};

  xml_encoding encodings[] = {
      encoding_utf16_be, encoding_utf16_be, encoding_utf16_be,
      encoding_utf16_le, encoding_utf16_le, encoding_utf16_le,
      encoding_utf32_be, encoding_utf32_be, encoding_utf32_be,
      encoding_utf32_le, encoding_utf32_le, encoding_utf32_le,
      encoding_utf8,     encoding_utf8,     encoding_utf8};

  for (unsigned int i = 0; i < sizeof(files) / sizeof(files[0]); ++i) {
    xml_document doc;
    xml_parse_result res = doc.load_file(files[i]);

    CHECK(res.status == status_ok);
    CHECK(res.encoding == encodings[i]);
    check_utftest_document(doc);
  }
}

TEST(document_load_file_convert_specific) {
  const char* files[] = {"tests/data/utftest_utf16_be.xml",
                         "tests/data/utftest_utf16_be_bom.xml",
                         "tests/data/utftest_utf16_be_nodecl.xml",
                         "tests/data/utftest_utf16_le.xml",
                         "tests/data/utftest_utf16_le_bom.xml",
                         "tests/data/utftest_utf16_le_nodecl.xml",
                         "tests/data/utftest_utf32_be.xml",
                         "tests/data/utftest_utf32_be_bom.xml",
                         "tests/data/utftest_utf32_be_nodecl.xml",
                         "tests/data/utftest_utf32_le.xml",
                         "tests/data/utftest_utf32_le_bom.xml",
                         "tests/data/utftest_utf32_le_nodecl.xml",
                         "tests/data/utftest_utf8.xml",
                         "tests/data/utftest_utf8_bom.xml",
                         "tests/data/utftest_utf8_nodecl.xml"};

  xml_encoding encodings[] = {
      encoding_utf16_be, encoding_utf16_be, encoding_utf16_be,
      encoding_utf16_le, encoding_utf16_le, encoding_utf16_le,
      encoding_utf32_be, encoding_utf32_be, encoding_utf32_be,
      encoding_utf32_le, encoding_utf32_le, encoding_utf32_le,
      encoding_utf8,     encoding_utf8,     encoding_utf8};

  for (unsigned int i = 0; i < sizeof(files) / sizeof(files[0]); ++i) {
    for (unsigned int j = 0; j < sizeof(files) / sizeof(files[0]); ++j) {
      xml_encoding encoding = encodings[j];

      xml_document doc;
      xml_parse_result res = doc.load_file(files[i], parse_default, encoding);

      if (encoding == encodings[i]) {
        CHECK(res.status == status_ok);
        CHECK(res.encoding == encoding);
        check_utftest_document(doc);
      } else {
        // should not get past first tag
        CHECK(!doc.first_child());
      }
    }
  }
}

TEST(document_load_file_convert_native_endianness) {
  const char* files[2][6] = {{
                                 "tests/data/utftest_utf16_be.xml",
                                 "tests/data/utftest_utf16_be_bom.xml",
                                 "tests/data/utftest_utf16_be_nodecl.xml",
                                 "tests/data/utftest_utf32_be.xml",
                                 "tests/data/utftest_utf32_be_bom.xml",
                                 "tests/data/utftest_utf32_be_nodecl.xml",
                             },
                             {
                                 "tests/data/utftest_utf16_le.xml",
                                 "tests/data/utftest_utf16_le_bom.xml",
                                 "tests/data/utftest_utf16_le_nodecl.xml",
                                 "tests/data/utftest_utf32_le.xml",
                                 "tests/data/utftest_utf32_le_bom.xml",
                                 "tests/data/utftest_utf32_le_nodecl.xml",
                             }};

  xml_encoding encodings[] = {encoding_utf16, encoding_utf16, encoding_utf16,
                              encoding_utf32, encoding_utf32, encoding_utf32};

  for (unsigned int i = 0; i < sizeof(files[0]) / sizeof(files[0][0]); ++i) {
    const char* right_file = files[is_little_endian()][i];
    const char* wrong_file = files[!is_little_endian()][i];

    for (unsigned int j = 0; j < sizeof(encodings) / sizeof(encodings[0]);
         ++j) {
      xml_encoding encoding = encodings[j];

      // check file with right endianness
      {
        xml_document doc;
        xml_parse_result res =
            doc.load_file(right_file, parse_default, encoding);

        if (encoding == encodings[i]) {
          CHECK(res.status == status_ok);
          check_utftest_document(doc);
        } else {
          // should not get past first tag
          CHECK(!doc.first_child());
        }
      }

      // check file with wrong endianness
      {
        xml_document doc;
        doc.load_file(wrong_file, parse_default, encoding);
        CHECK(!doc.first_child());
      }
    }
  }
}

struct file_data_t {
  const char* path;
  xml_encoding encoding;

  char* data;
  size_t size;
};

#ifndef PUGIXML_NO_EXCEPTIONS
TEST(document_load_exceptions) {
  bool thrown = false;

  try {
    xml_document doc;
    if (doc.load_string(STR("<node attribute='value")).status != status_ok)
      throw std::bad_alloc();

    CHECK_FORCE_FAIL("Expected parsing failure");
  } catch (const std::bad_alloc&) {
    thrown = true;
  }

  CHECK(thrown);
}
#endif

TEST_XML_FLAGS(document_element,
               "<?xml version='1.0'?><node><child/></node><!---->",
               parse_default | parse_declaration | parse_comments) {
  CHECK(doc.document_element() == doc.child(STR("node")));
}

TEST_XML_FLAGS(document_element_absent,
               "<!---->",
               parse_comments | parse_fragment) {
  CHECK(doc.document_element() == xml_node());
}

TEST_XML(document_reset, "<node><child/></node>") {
  CHECK(!doc.first_child().empty());

  doc.reset();
  CHECK(!doc.first_child());
  CHECK_NODE(doc, STR(""));

  doc.reset();
  CHECK(!doc.first_child());
  CHECK_NODE(doc, STR(""));

  CHECK(doc.load_string(STR("<node/>")).status == status_ok);
  CHECK(!doc.first_child().empty());
  CHECK_NODE(doc, STR("<node/>"));

  doc.reset();
  CHECK(!doc.first_child());
  CHECK_NODE(doc, STR(""));
}

TEST(document_reset_empty) {
  xml_document doc;

  doc.reset();
  CHECK(!doc.first_child());
  CHECK_NODE(doc, STR(""));
}

TEST_XML(document_reset_copy, "<node><child/></node>") {
  xml_document doc2;

  CHECK_NODE(doc2, STR(""));

  doc2.reset(doc);

  CHECK_NODE(doc2, STR("<node><child/></node>"));
  doc.first_child() != doc2.first_child();

  doc.reset(doc2);

  CHECK_NODE(doc, STR("<node><child/></node>"));
  doc.first_child() != doc2.first_child();

  CHECK(doc.first_child().offset_debug() == -1);
}

TEST_XML(document_reset_copy_self, "<node><child/></node>") {
  CHECK_NODE(doc, STR("<node><child/></node>"));

  doc.reset(doc);

  CHECK(!doc.first_child());
  CHECK_NODE(doc, STR(""));
}
