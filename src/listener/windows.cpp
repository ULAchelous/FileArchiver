#ifdef _WIN32
#include "listener.h"

#include <windows.h>
#include <deque>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "registry.h"
#include "app.h"

namespace {

constexpr DWORD kBufBytes = 16 * 1024;

struct Watch {
    HANDLE dir = INVALID_HANDLE_VALUE;
    OVERLAPPED ov{};
    std::vector<char> buffer;
    std::filesystem::path root;
    repo::Repository* owner = nullptr;
};


FSEvent cet(DWORD action) {
    switch (action) {
        case FILE_ACTION_REMOVED:          return FSEvent::REMOVED;
        case FILE_ACTION_RENAMED_NEW_NAME: return FSEvent::RENAMED;
        case FILE_ACTION_ADDED:            return FSEvent::CREATED;
        case FILE_ACTION_MODIFIED:         return FSEvent::MODIFiED;
        default:                           return FSEvent::IGNORED;
    }
}

void dispatch(Watch& w, DWORD bytes) {
    if (bytes == 0) return;
    auto* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(w.buffer.data());
    while (true) {
        std::wstring name(info->FileName, info->FileNameLength / sizeof(WCHAR));
        std::filesystem::path full = w.root / std::filesystem::path(name);

        DWORD attrs = GetFileAttributesW(full.c_str());
        bool is_dir = (attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_DIRECTORY);
        if (!is_dir)
            fa_fs_event_holder(cet(info->Action), full, w.owner);

        if (info->NextEntryOffset == 0) break;
        info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
            reinterpret_cast<BYTE*>(info) + info->NextEntryOffset);
    }
}

BOOL issue(Watch& w) {
    ResetEvent(w.ov.hEvent);
    DWORD ret = 0;
    BOOL ok = ReadDirectoryChangesW(
        w.dir, w.buffer.data(), kBufBytes, TRUE,
        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
        FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE,
        &ret, &w.ov, nullptr);
    if (!ok && GetLastError() != ERROR_IO_PENDING)
        return FALSE;
    return TRUE;
}

}  // namespace

void fa_start_fs_listener(Context& ctx) {
    // deque：push_back 不会让已有元素的地址失效，而 &w.ov 在挂起的 I/O 期间必须保持不动
    std::deque<Watch> watches;
    std::vector<HANDLE> events;

    for (auto& iter : ctx.registries.get_registry<repo::Repository>().data()) {
        repo::Repository* owner = &iter.second;
        for (const auto& src : owner->manifest.get_sources()) {
            HANDLE h = CreateFileW(
                src.c_str(), FILE_LIST_DIRECTORY,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                nullptr, OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);
            if (h == INVALID_HANDLE_VALUE)
                throw std::runtime_error("CreateFileW failed for: " + src.string());

            watches.push_back(Watch{});
            Watch& w = watches.back();
            w.dir = h;
            w.root = src;
            w.owner = owner;
            w.buffer.resize(kBufBytes);
            w.ov.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
            if (!w.ov.hEvent)
                throw std::runtime_error("CreateEvent failed");

            events.push_back(w.ov.hEvent);
            if (!issue(w))
                throw std::runtime_error("ReadDirectoryChangesW failed for: " + src.string());
        }
    }

    while (!events.empty()) {
        DWORD idx = WaitForMultipleObjects(
            static_cast<DWORD>(events.size()), events.data(), FALSE, INFINITE);
        if (idx == WAIT_FAILED || idx < WAIT_OBJECT_0 ||
            idx >= WAIT_OBJECT_0 + events.size())
            break;

        size_t i = idx - WAIT_OBJECT_0;
        DWORD bytes = 0;
        if (GetOverlappedResult(watches[i].dir, &watches[i].ov, &bytes, FALSE))
            dispatch(watches[i], bytes);
        issue(watches[i]);
    }

    for (auto& w : watches) {
        if (w.dir != INVALID_HANDLE_VALUE) CancelIo(w.dir);
        if (w.ov.hEvent) CloseHandle(w.ov.hEvent);
        if (w.dir != INVALID_HANDLE_VALUE) CloseHandle(w.dir);
    }
}

#endif  // _WIN32
