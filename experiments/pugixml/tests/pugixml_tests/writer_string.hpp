#ifndef HEADER_TEST_WRITER_STRING_HPP
#define HEADER_TEST_WRITER_STRING_HPP

#include "pugixml.h"

#include <string>

struct xml_writer_string : public pugi::xml_writer {
  std::string contents;

  virtual void write(const void* data, size_t size) PUGIXML_OVERRIDE;

  std::string as_narrow() const;
  std::basic_string<wchar_t> as_wide() const;
  std::basic_string<pugi::char_t> as_string() const;
};

std::string save_narrow(const pugi::PugixmlHooked::xml_document& doc,
                        unsigned int flags,
                        pugi::PugixmlHooked::xml_encoding encoding);
bool test_save_narrow(const pugi::PugixmlHooked::xml_document& doc,
                      unsigned int flags,
                      pugi::PugixmlHooked::xml_encoding encoding,
                      const char* expected,
                      size_t length);

std::string write_narrow(pugi::PugixmlHooked::xml_node node,
                         unsigned int flags,
                         pugi::PugixmlHooked::xml_encoding encoding);
bool test_write_narrow(pugi::PugixmlHooked::xml_node node,
                       unsigned int flags,
                       pugi::PugixmlHooked::xml_encoding encoding,
                       const char* expected,
                       size_t length);

std::basic_string<wchar_t> write_wide(
    pugi::PugixmlHooked::xml_node node,
    unsigned int flags,
    pugi::PugixmlHooked::xml_encoding encoding);

#endif
