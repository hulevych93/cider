#include "swig.h"

#include <cppast/cpp_file.hpp>

#include "options.h"
#include "utils.h"

#include <list>

using namespace cppast;

namespace {

using FilesList = std::list<cider::tool::FileMetadata>;

template <typename Functor>
void traverse(FilesList& files,
              const FilesList::const_iterator iter,
              Functor&& func) {
  const auto& file = *iter;
  for (const auto& imp : file.imports) {
    auto it = std::find_if(files.begin(), files.end(), [imp](const auto& m) {
      return m.exports.find(imp) != m.exports.end();
    });
    if (it != files.end()) {
      traverse(files, it, func);
    }
  }
  func(*iter);
  files.erase(iter);
}

const char* includes = R"(
%include <std_string.i>
%include <std_vector.i>
%include <stdint.i>

%rename(equalOp) operator =;
%rename(plusEqualOp) operator +=;
%rename(minusEqualOp) operator -=;
%rename(multiplyEqualOp) operator *=;
%rename(divideEqualOp) operator /=;
%rename(percentEqualOp) operator %=;
%rename(plusOp) operator +;
%rename(minusOp) operator -;
%rename(multiplyOp) operator *;
%rename(divideOp) operator /;
%rename(percentOp) operator %;
%rename(notOp) operator !;
%rename(indexIntoConstOp) operator[](unsigned idx) const;
%rename(indexIntoOp) operator[](unsigned idx);
%rename(functorOp) operator ();
%rename(equalEqualOp) operator ==;
%rename(notEqualOp) operator !=;
%rename(lessThanOp) operator <;
%rename(lessThanEqualOp) operator <=;
%rename(greaterThanOp) operator >;
%rename(greaterThanEqualOp) operator >=;
%rename(andOp) operator &&;
%rename(orOp) operator ||;
%rename(plusPlusPrefixOp) operator++();
%rename(plusPlusPostfixOp) operator++(int);
%rename(minusMinusPrefixOp) operator--();
%rename(minusMinusPostfixOp) operator--(int);

%rename(toBool) *::operator bool;
%rename(toFloat) *::operator float;
%rename(toDouble) *::operator double;
%rename(toLongDouble) *::operator long double;
%rename(toChar) *::operator char;
%rename(toUnsignedChar) *::operator unsigned char;
%rename(toShort) *::operator short;
%rename(toInt) *::operator int;
%rename(toLong) *::operator long;
%rename(toLongLong) *::operator long long;
%rename(toUnsignedShort) *::operator unsigned short;
%rename(toUnsignedInt) *::operator unsigned int;
%rename(toUnsignedLong) *::operator unsigned long;
%rename(toUnsignedLongLong) *::operator unsigned long long;
%rename(toString) *::operator std::string;
%rename(toConstCharString) *::operator const char*;

)";

}  // namespace

namespace cider {
namespace tool {

swig_generator::swig_generator(std::ostream& out,
                               const std::string& outPath,
                               const std::string& module,
                               const MetadataStorage& metadata)
    : m_out(out), m_outPath(outPath), m_module(module), m_metadata(metadata) {
  m_out << "%module " << m_module << "\n";
  m_out << includes;
}

void swig_generator::finish() {
  FilesList files;
  for (const auto& [name, file] : m_metadata.files) {
    files.emplace_back(file);
  }

  const auto printFile = [&](const FileMetadata& file) {
    const auto& path = getOutputFilePathWithoutExtension(file.name, m_outPath);

    m_out << "%{ #include \"" << path << ".h\" %}\n";
    m_out << "%include \"" << path << ".h\"\n";
  };

  auto it = files.begin();
  while (!files.empty()) {
    traverse(files, it, printFile);
    it = files.begin();
  }
}

void printSwig(std::ostream& os,
               const std::string& outPath,
               const std::string& moduleName,
               const MetadataStorage& metadata,
               const std::vector<const cppast::cpp_file*>& files) {
  swig_generator swig_gen(os, outPath, moduleName, metadata);
  for (const auto& file : files) {
    handleFile(swig_gen, *file);
  }
  swig_gen.finish();
}

}  // namespace tool
}  // namespace cider
