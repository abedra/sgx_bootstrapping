#include <fstream>

struct Bootstrap {
  explicit Bootstrap(std::string bootstrap_file)
    : bootstrap_file_(std::move(bootstrap_file))
  {}

  int save(const uint8_t* sealed_data, const size_t sealed_size) const {
    std::ofstream file(bootstrap_file_, std::ios::out | std::ios::binary);

    if (file.fail()) {
      return 1;
    }

    file.write((const char*) sealed_data, sealed_size);
    file.close();

    return 0;
  }

  int load(uint8_t* sealed_data, const size_t sealed_size) const {
    std::ifstream file(bootstrap_file_, std::ios::in | std::ios::binary);

    if (file.fail()) {
      return 1;
    }

    file.read((char*) sealed_data, sealed_size);
    file.close();

    return 0;
  }

  bool is_bootstrap_file(void) const {
    std::ifstream file(bootstrap_file_, std::ios::in | std::ios::binary);

    if (file.fail()) {
      return false;
    }

    file.close();

    return true;
  }

private:
  std::string bootstrap_file_;
};
