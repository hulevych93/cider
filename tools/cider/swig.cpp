#include "swig.h"

#include <cppast/cpp_class.hpp>
#include <cppast/cpp_entity_kind.hpp>
#include <cppast/cpp_enum.hpp>
#include <cppast/cpp_file.hpp>

#include "options.h"
#include "utils.h"

#include <list>

using namespace cppast;

namespace {

static class NullBuffer : public std::streambuf {
 public:
  int overflow(int c) { return c; }
} null_buffer;

static std::ostream null_stream(&null_buffer);

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
                               const std::string& swigDir,
                               const std::string& module,
                               const std::string& genScope,
                               const MetadataStorage& metadata)
    : m_out(out),
      m_outPath(outPath),
      m_swigDir(swigDir),
      m_module(module),
      m_metadata(metadata),
      m_namespaces(genScope) {
  m_out << "%module " << m_module << "\n";
  m_out << "%include <" << m_swigDir << "/lua/main.swig>\n\n";
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

void swig_generator::handleNamespace(const cpp_entity& e, const bool enter) {
  if (enter) {
    m_namespaces.push(null_stream, e.name());
  } else {
    m_namespaces.pop(null_stream);
  }
}

void swig_generator::handleEnum(const cppast::cpp_enum& e, const bool enter) {
  if (enter) {
    const auto fullName = m_namespaces.nativeScope() +
                          "::" + m_namespaces.genScope() + "::" + e.name();
    m_out << "%template(" + e.name() + ") cider::TypedValue<" + fullName +
                 ">;\n";
    m_out << "decl_enum_based(" + fullName + ")\n";
  }
}

void printSwig(std::ostream& os,
               const std::string& outPath,
               const std::string& swigDir,
               const std::string& moduleName,
               const std::string& genScope,
               const MetadataStorage& metadata,
               const std::vector<const cppast::cpp_file*>& files) {
  swig_generator swig_gen(os, outPath, swigDir, moduleName, genScope, metadata);
  for (const auto& file : files) {
    handleFile(swig_gen, *file);
  }
  swig_gen.finish();
}

}  // namespace tool
}  // namespace cider
