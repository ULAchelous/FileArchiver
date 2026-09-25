# FileArchiver

基于 CMake 的 C++17 文件归档工具项目。

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
