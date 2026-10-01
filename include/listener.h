#pragma once
#include <vector>
#include <filesystem>

struct Context;

namespace repo{
    struct Repository;
}

enum class FSEvent{
    CREATED,
    REMOVED,
    MODIFiED,
    RENAMED,
    IGNORED
};

struct Event{
    FSEvent type;
    std::filesystem::path path;
};

void fa_fs_event_holder(FSEvent event,const std::filesystem::path& path,repo::Repository* repo);
void fa_fs_on_created(const std::filesystem::path& path,const repo::Repository* repo);
void fa_fs_on_renamed(const std::filesystem::path& path,const repo::Repository* repo);
void fa_fs_on_removed(const std::filesystem::path& path,const repo::Repository* repo);
void fa_fs_on_modified(const std::filesystem::path& path,const repo::Repository* repo);
void fa_start_fs_listener(Context& ctx);