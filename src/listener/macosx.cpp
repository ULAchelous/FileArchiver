#include <CoreServices/CoreServices.h>
#include <vector>
#include<string>
#include <filesystem>

#include "listener.h"
#include "logger.h"
#include "registry.h"

FSEvent cet(FSEventStreamEventFlags f){
    if (f & kFSEventStreamEventFlagItemRemoved)  return FSEvent::REMOVED;
    if (f & kFSEventStreamEventFlagItemRenamed)  return FSEvent::RENAMED;
    if (f & kFSEventStreamEventFlagItemCreated)  return FSEvent::CREATED;
    if (f & kFSEventStreamEventFlagItemModified) return FSEvent::MODIFiED;
    return FSEvent::IGNORED;
}

static void callback(ConstFSEventStreamRef,void* user_data,size_t event_count,void* event_paths,const FSEventStreamEventFlags flags[],const FSEventStreamEventId[]) {
    CFArrayRef paths = static_cast<CFArrayRef>(event_paths);
    
    for(size_t i =0 ;i< event_count;i ++){
        char buf[PATH_MAX];
        if(flags[i] & kFSEventStreamEventFlagItemIsDir) continue;
        CFStringRef cf = (CFStringRef)CFArrayGetValueAtIndex(paths,(CFIndex)i);
        if(CFStringGetCString(cf,buf,sizeof(buf),kCFStringEncodingUTF8)){
            fa_fs_event_holder(cet(flags[i]),buf,static_cast<repo::Repository*>(user_data));
        }
    }
}

void fa_start_fs_listener(reg::Registries* registries){
    std::vector<CFArrayRef> cf_array; 
    std::vector<FSEventStreamRef> fs_event_stream;
    for(auto& iter : registries->get_registry<repo::Repository>().data()){
        std::vector<const void*> srcs;
        for(const auto& src : iter.second.manifest.get_sources()){
            const void* ptr = CFStringCreateWithCString(nullptr,src.c_str(),kCFStringEncodingUTF8);
            if(ptr == nullptr)
                throw std::runtime_error("Failed to create CFString");
            srcs.push_back(ptr);
        }
        CFArrayRef paths = CFArrayCreate(nullptr,srcs.data(),static_cast<CFIndex>(srcs.size()),&kCFTypeArrayCallBacks);

        if(paths == nullptr)
            throw std::runtime_error("Failed to create CFArrayRef");
        
        FSEventStreamContext context{};
        context.info = &iter.second;
        auto stream = FSEventStreamCreate(
            nullptr,
            callback,
            &context,
            paths,
            kFSEventStreamEventIdSinceNow,
            0.5,
            kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagIgnoreSelf | kFSEventStreamCreateFlagNoDefer | kFSEventStreamCreateFlagUseCFTypes
        );
        if(!stream)
            throw std::runtime_error("Failed to create FSEventStream");
        CFRelease(paths);
        for(auto ptr : srcs)
            CFRelease(ptr);
        fs_event_stream.push_back(stream);
    }
    for(const FSEventStreamRef& stream : fs_event_stream){
        FSEventStreamScheduleWithRunLoop(stream,CFRunLoopGetCurrent(),kCFRunLoopDefaultMode);
        if(!FSEventStreamStart(stream))
            throw std::runtime_error("Failed to start CFStream");
    }
    
    CFRunLoopRun();//开始堵塞线程

    for(const FSEventStreamRef& stream : fs_event_stream){
        FSEventStreamStop(stream);
        FSEventStreamInvalidate(stream);
        FSEventStreamRelease(stream);
    }
}