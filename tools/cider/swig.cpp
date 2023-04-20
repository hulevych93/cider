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

}  // namespace

namespace cider {
namespace tool {

swig_generator::swig_generator(std::ostream& out,
                               const std::string& outPath,
                               const std::string& module,
                               const MetadataStorage& metadata)
    : m_out(out), m_outPath(outPath), m_module(module), m_metadata(metadata) {
  m_out << "%module " << m_module << "\n";
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

void printSwig(
    std::ostream& os,
    const std::string& outPath,
    const std::string& moduleName,
    const MetadataStorage& metadata,
    const cppast::detail::iteratable_intrusive_list<cppast::cpp_file>& files) {
  swig_generator swig_gen(os, outPath, moduleName, metadata);
  for (const auto& file : files) {
    handleFile(swig_gen, file);
  }
  swig_gen.finish();
}

}  // namespace tool
}  // namespace cider
