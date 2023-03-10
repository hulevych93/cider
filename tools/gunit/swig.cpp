#include "swig.h"

#include <cppast/cpp_file.hpp>

#include "utils.h"

using namespace cppast;

namespace gunit {
namespace tool {

swig_generator::swig_generator(std::ostream& out,
                               const std::string& module,
                               const MetadataStorage& metadata)
    : m_out(out), m_module(module), m_metadata(metadata) {
  m_out << "%module " << m_module << "\n";
}

void swig_generator::finish() {
  std::vector<FileMetadata> files;
  files.reserve(m_metadata.files.size());
  for (const auto& [name, file] : m_metadata.files) {
    files.emplace_back(file);
  }

  std::sort(files.begin(), files.end(),
            [](const FileMetadata& m1, const FileMetadata& m2) {
              if (m1.name == m2.name) {
                return false;
              }
              for (const auto& imp : m1.imports) {
                if (m2.exports.find(imp) != m2.exports.end()) {
                  return false;
                }
              }
              return true;
            });

  for (const auto& file : files) {
    m_out << "%{ #include \"" << file.name << "\" %}\n";
    m_out << "%include \"" << file.name << "\"\n";
  }
}

void printSwig(
    std::ostream& os,
    const std::string& moduleName,
    const MetadataStorage& metadata,
    const cppast::detail::iteratable_intrusive_list<cppast::cpp_file>& files) {
  swig_generator swig_gen(os, moduleName, metadata);
  for (const auto& file : files) {
    handleFile(swig_gen, file);
  }
  swig_gen.finish();
}

}  // namespace tool
}  // namespace gunit
