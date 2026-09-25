#pragma once
#include<vector>
#include "data/file.h"
#include "data/repo.h"

void fa_archive(const std::vector<File>& pendding_files,const repo::Repository& repo);

void fa_archive_one(const repo::Repository& repo,const File& file);
