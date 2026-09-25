#pragma once
#include<string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include "data/file.h"
#include "registry.h"

namespace repo{
    class Manifest{
        public:
            Manifest(const std::filesystem::path& source_file,reg::Registries* registries);
            std::string get_target_dir(const File& file) const;
            const std::string& get_name() const { return _name; }
            const std::filesystem::path& get_log_file() const { return _log_file; }
            const std::filesystem::path& get_source_file() const { return _source_file; }
            const std::vector<std::filesystem::path>& get_sources() const { return _sources; }
            const std::vector<std::string>& get_exclude() const { return _exclude; }
            const std::unordered_map<const FileType*, std::string>& get_type_to_dir() const { return _type_to_dir; }
        private:
            YAML::Node _structure;
            std::unordered_map<const FileType*,std::string> _type_to_dir;
            std::vector<std::filesystem::path> _sources;
            std::vector<std::string> _exclude;
            std::string _name = "repository";
            std::filesystem::path _log_file;
            std::filesystem::path _source_file;
            reg::Registries* _registries;
            void _recursion_structure(YAML::Node current,std::string name);
    };

    struct Repository{
        Repository(const std::filesystem::path& root,const Manifest& _manifest) : root_dir(root),manifest(_manifest){}
        const std::filesystem::path root_dir;
        const Manifest manifest;
    };
}


repo::Repository fa_load_repository(const std::filesystem::path& repo_path, reg::Registries* registries);
