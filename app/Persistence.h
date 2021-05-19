#pragma once

#include <filesystem>
#include <fstream>

struct Persistence {
  explicit Persistence(std::filesystem::path path)
    : path_(std::move(path))
  {}

  int save(const uint8_t* sealed_data, const size_t sealed_size) const {
    std::ofstream file(path_, std::ios::out | std::ios::binary);

    if (file.fail()) {
      return 1;
    }

    file.write((const char*) sealed_data, sealed_size);
    file.close();

    return 0;
  }

  int load(uint8_t* sealed_data, const size_t sealed_size) const {
    std::ifstream file(path_, std::ios::in | std::ios::binary);

    if (file.fail()) {
      return 1;
    }

    file.read((char*) sealed_data, sealed_size);
    file.close();

    return 0;
  }

  const bool exists() const {
    return std::filesystem::exists(path_);
  }

  const std::string path() const {
    return path_.string();
  }

private:
  std::filesystem::path path_;
};
