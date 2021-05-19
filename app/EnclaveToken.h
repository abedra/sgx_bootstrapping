#pragma once

#include <filesystem>
#include <iostream>

struct EnclaveToken {
  explicit EnclaveToken(std::filesystem::path path)
    : path_(std::move(path))
  {}

  int read(sgx_launch_token_t *token) {
    FILE* fp = fopen(path_.c_str(), "rb");

    if (fp == NULL && (fp = fopen(path_.c_str(), "wb")) == NULL) {
      std::cout << "Warning: Failed to create/open the launch token file " << path_.string() << std::endl;
    }

    if (fp != NULL) {
      size_t read_num = fread(token, 1, sizeof(sgx_launch_token_t), fp);

      if (read_num != 0 && read_num != sizeof(sgx_launch_token_t)) {
        memset(token, 0x0, sizeof(sgx_launch_token_t));
        std::cout << "Warning: Invalid launch token read from " << path_.string() << std::endl;
      }

      fclose(fp);
    }

    return 0;
  }

  int save(sgx_launch_token_t *token) {
    FILE* fp = fopen(path_.c_str(), "wb");

    if (fp == NULL) {
      return -1;
    }

    size_t write_num = fwrite(token, 1, sizeof(sgx_launch_token_t), fp);

    if (write_num != sizeof(sgx_launch_token_t)) {
      std::cout << "Warning: Failed to save launch token to " << path_.string() << std::endl;
    }

    fclose(fp);

    return 0;
  }

private:
  std::filesystem::path path_;
};
