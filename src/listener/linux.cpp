#if defined(__linux__)
#include "listener.h"

#include <sys/inotify.h>
#include <unistd.h>
#include <cerrno>
#include <climits>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "registry.h"

namespace {

constexpr size_t kBufLen = 16 * 1024;
constexpr uint32_t kWatchMask =
    IN_CREATE | IN_DELETE | IN_MODIFY | IN_ATTRIB |
    IN_MOVED_FROM | IN_MOVED_TO | IN_MOVE_SELF | IN_DELETE_SELF;

struct WatchDir {
    std::filesystem::path dir;
    repo::Repository* repo;
};

using WdMap = std::unordered_map<int, WatchDir>;

FSEvent cet(uint32_t mask) {
    if (mask & (IN_DELETE | IN_MOVED_FROM))    return FSEvent::REMOVED;
    if (mask & IN_MOVED_TO)                    return FSEvent::RENAMED;
    if (mask & IN_CREATE)                      return FSEvent::CREATED;
    if (mask & (IN_MODIFY | IN_ATTRIB))        return FSEvent::MODIFiED;
    return FSEvent::IGNORED;
}

void add_watch(int fd, const std::filesystem::path& dir, repo::Repository* repo, WdMap& wd_map) {
    int wd = inotify_add_watch(fd, dir.c_str(), kWatchMask);
    if (wd >= 0)
        wd_map[wd] = WatchDir{dir, repo};
}

void add_watches_recursive(int fd, const std::filesystem::path& root, repo::Repository* repo, WdMap& wd_map) {
    add_watch(fd, root, repo, wd_map);
    std::error_code ec;
    for (auto it = std::filesystem::recursive_directory_iterator(root, ec), end{};
         it != end; it.increment(ec)) {
        if (ec) break;
        if (it->is_directory(ec))
            add_watch(fd, it->path(), repo, wd_map);
    }
}

}  // namespace

void fa_start_fs_listener(reg::Registries* registries) {
    int fd = inotify_init();
    if (fd < 0)
        throw std::runtime_error("inotify_init failed: " + std::string(strerror(errno)));

    WdMap wd_map;
    for (auto& iter : registries->get_registry<repo::Repository>().data()) {
        repo::Repository* repo = &iter.second;
        for (const auto& src : repo->manifest.get_sources())
            add_watches_recursive(fd, src, repo, wd_map);
    }

    std::vector<char> buf(kBufLen);
    while (true) {
        ssize_t len = read(fd, buf.data(), buf.size());
        if (len <= 0) {
            if (errno == EINTR) continue;
            break;
        }
        for (char* ptr = buf.data(); ptr < buf.data() + len;) {
            auto* ev = reinterpret_cast<inotify_event*>(ptr);
            ptr += sizeof(inotify_event) + ev->len;

            if (ev->mask & IN_Q_OVERFLOW) continue;
            if (ev->len == 0) continue;

            auto wit = wd_map.find(ev->wd);
            if (wit == wd_map.end()) continue;
            std::filesystem::path full = wit->second.dir / ev->name;

            if (ev->mask & IN_ISDIR) {
                // 新建的子目录继承父 watch 所属的仓库
                if (ev->mask & (IN_CREATE | IN_MOVED_TO))
                    add_watches_recursive(fd, full, wit->second.repo, wd_map);
                continue;
            }

            fa_fs_event_holder(cet(ev->mask), full, wit->second.repo);
        }
    }

    close(fd);
}

#endif  // __linux__
