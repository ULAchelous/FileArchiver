#include "data/registry_builtin.h"
#include "data/file.h"
#include <vector>
#include <string>

namespace reg{

namespace {

    struct BuiltinType{
        const char* id;
        std::vector<std::string> exts;
    };

    const BuiltinType kBuiltinTypes[] = {
        // 文档
        {"text",       {".txt", ".log"}},
        {"markdown",   {".md", ".markdown"}},
        {"pdf",        {".pdf"}},
        {"word",       {".doc", ".docx"}},
        {"excel",      {".xls", ".xlsx"}},
        {"powerpoint", {".ppt", ".pptx"}},

        // 图片
        {"image",      {".jpg", ".jpeg", ".png", ".gif", ".svg", ".psd", ".bmp", ".webp"}},

        // 代码与脚本
        {"code",       {".cpp", ".cc", ".cxx", ".h", ".hpp", ".c", ".py", ".js", ".ts", ".java", ".go", ".rs", ".sh", ".json", ".xml"}},

        // 媒体
        {"media",      {".mp4", ".mov", ".mkv", ".mp3", ".wav", ".flac"}},

        // 压缩包
        {"archive",    {".zip", ".tar", ".gz", ".7z", ".rar"}},
    };
}

void fa_register_builtin_types(Registries& registries){
    auto& file_registry = registries.get_registry<FileType>();
    for(const BuiltinType& t : kBuiltinTypes){
        file_registry.register_t(t.id, FileType(t.id, t.exts, nullptr));
    }
}
}
