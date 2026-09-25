#pragma once
#include <string>
#include "data/file.h"
#include "plugin/types.h"
#include "logger.h"
#include <unordered_map>
#include <utility>
#include <stdexcept>

namespace reg{

    template<typename T>
    class Registry{
        public:
            void register_t(const std::string& id,T type){
                if(_map.find(id) != _map.end())
                    throw std::runtime_error("Type with id '" + id + "' is already registered.");
                _map.emplace(id, std::move(type));
            };
            const T* get_type(const std::string& id) const{
                auto it = _map.find(id);
                if(it != _map.end()){
                    return &(it->second);
                }
                return nullptr;
            };
        private:
            std::unordered_map<std::string,T> _map;
    };

    template<>
    class Registry<FileType>{
        public:
            void register_t(const std::string& id,FileType type){
                if(type.id == "" || type.exts.empty())
                    throw std::runtime_error("Invalid FileType: id and exts must be non-empty.");
                if(_map.find(id) != _map.end())
                    throw std::runtime_error("Type with id '" + id + "' is already registered.");

                LOGGER.info("Registering FileType '" + id + "' with " + std::to_string(type.exts.size()) + " extensions");
                _map.emplace(id, std::move(type));
                for(const auto& ext : _map.at(id).exts){
                    _map_ext[ext].push_back(&(_map.at(id)));
                }
            };
            const FileType* get_type(const std::string& id) const{
                auto it = _map.find(id);
                if(it != _map.end()){
                    LOGGER.info("FileType lookup by id: '" + id + "' -> hit");
                    return &(it->second);
                }
                LOGGER.info("FileType lookup by id: '" + id + "' -> miss");
                return nullptr;
            };
            const std::vector<FileType*>& get_types_by_ext(const std::string& ext){
                LOGGER.info("FileType lookup by extension: '" + ext + "'");
                if(_map_ext.find(ext) == _map_ext.end())
                    throw std::runtime_error("No FileTypes registered for extension: " + ext);
                LOGGER.info("Extension '" + ext + "' matched " + std::to_string(_map_ext[ext].size()) + " candidates");
                return _map_ext[ext];
            }
        private:
            std::unordered_map<std::string,FileType> _map;
            std::unordered_map<std::string,std::vector<FileType*>> _map_ext;
    };

    class Registries{
        public:
            template<typename T>
            Registry<T>& get_registry(){
                if constexpr (std::is_same_v<T, FileType>) {
                    return _file_type_registry;
                } else if constexpr (std::is_same_v<T, fa_plugin>) {
                    return _plugin_registry;
                } else {
                    static_assert(!sizeof(T*), "unknown registry type");
                }
            };
        private:
            Registry<FileType> _file_type_registry;
            Registry<fa_plugin> _plugin_registry;
    };
}
