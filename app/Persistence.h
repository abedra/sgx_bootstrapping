#pragma once

#include <fstream>

struct Persistence {
  explicit Persistence(std::string file_name)
    : file_name_(std::move(file_name))
  {}

  int save(const uint8_t* sealed_data, const size_t sealed_size) const {
    std::ofstream file(file_name_, std::ios::out | std::ios::binary);

    if (file.fail()) {
      return 1;
    }

    file.write((const char*) sealed_data, sealed_size);
    file.close();

    return 0;
  }

  int load(uint8_t* sealed_data, const size_t sealed_size) const {
    std::ifstream file(file_name_, std::ios::in | std::ios::binary);

    if (file.fail()) {
      return 1;
    }

    file.read((char*) sealed_data, sealed_size);
    file.close();

    return 0;
  }

  bool exists(void) const {
    std::ifstream file(file_name_, std::ios::in | std::ios::binary);

    if (file.fail()) {
      return false;
    }

    file.close();

    return true;
  }

  const std::string& file_name() const {
    return file_name_;
  }

private:
  std::string file_name_;
};
