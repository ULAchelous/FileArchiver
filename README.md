# FileArchiver

基于 CMake 的 C++17 文件归档工具项目。

## 目录结构

```
FileArchiver/
├── CMakeLists.txt        # 顶层构建脚本
├── include/              # 公共头文件
│   └── archiver.hpp
├── src/                  # 源文件（主库 + 可执行入口）
│   ├── archiver.cpp
│   └── main.cpp
├── tests/                # 单元测试
│   ├── CMakeLists.txt
│   └── test_archiver.cpp
└── .gitignore
```

## 构建与运行

```bash
# 配置（首次或 CMakeLists 变更后）
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# 构建
cmake --build build

# 运行
./build/bin/filearchiver archive.fa file1.txt file2.txt

# 测试
ctest --test-dir build --output-on-failure
```

## 说明

- C++17，开启 `-Wall -Wextra -Wpedantic`（MSVC 为 `/W4 /permissive-`）
- 核心逻辑在 `filearchiver_core` 库目标中，`filearchiver` 可执行文件仅做 CLI 入口
- `writeArchive()` 目前为占位实现（逐行写入路径），后续替换为真实归档格式
