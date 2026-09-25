#pragma once
#include<vector>
#include<filesystem>
#include "../data/file.h"
#include "../data/repo.h"
#include "../registry.h"
std::vector<File> fa_scan(const std::filesystem::path& dir_path,const repo::Repository& repo, reg::Registries* registries);