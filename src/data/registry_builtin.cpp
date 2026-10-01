#include "data/registry_builtin.h"
#include "data/file.h"
#include "app.h"
#include <vector>
#include <string>

namespace reg{

    const FileType kBuiltinTypes[] = {
        // 文档
        {"text",       {".txt", ".log"},nullptr},
        {"markdown",   {".md", ".markdown"},nullptr},
        {"pdf",        {".pdf"},nullptr},
        {"word",       {".doc", ".docx"},nullptr},
        {"excel",      {".xls", ".xlsx"},nullptr},
        {"powerpoint", {".ppt", ".pptx"},nullptr},

        {"excutable", {".exe",".elf"},nullptr},
        {"installer", {".dmg",".msi"},nullptr},

        // 图片
        {"image",      {".jpg", ".jpeg", ".png", ".gif", ".svg", ".psd", ".bmp", ".webp"},nullptr},

        // 代码与脚本
        {"code",       {".cpp", ".cc", ".cxx", ".h", ".hpp", ".c", ".py", ".js", ".ts", ".java", ".go", ".rs", ".sh", ".json", ".xml"},nullptr},

        // 媒体
        {"media",      {".mp4", ".mov", ".mkv", ".mp3", ".wav", ".flac"},nullptr},

        // 压缩包
        {"archive",    {".zip", ".tar", ".gz", ".7z", ".rar"},nullptr},
    };
    const repo::RepoTemplate kBuiltinTemplates[] = {
        {
            "default",
            "structure:\n  documents:\n    types: [text, markdown, pdf]     \n  images:\n    types: [image]             \n  code:\n    types: [code]                 \n  media:\n    types: [media]                  \n  archives:\n    types: [archive]  \n  excutable:\n    types: [excutable]\n  installer:\n    types: [installer]                          \n",
            "exclude:                 \n    - .DS_Store\n    - Thumbs.db\n    - .manifest.yaml\n",
            "  sources:\n    - ~/Donwloads\n    - ~/Documents\n",
            "filearchiver.log"
        }
    };

    void fa_register_builtin_types(Context& ctx){
        auto& file_registry = ctx.registries.get_registry<FileType>();
        for(const FileType& t : kBuiltinTypes){
            file_registry.register_t(t.id, t);
        }
    }

    void fa_register_builtin_repo_templates(Context& ctx){
        auto& template_registry = ctx.registries.get_registry<repo::RepoTemplate>();
        for(const repo::RepoTemplate& t : kBuiltinTemplates){
            template_registry.register_t(t.name,t);
        }
    }
}
