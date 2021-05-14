# SGX Bootstrapping Example

This project demonstrates the basics of sealing (encrypting) data inside an SGX enclave.

## Setup

Before compiling and running this program, you will need at least the [SGX sdk](https://github.com/intel/linux-sgx). If you wish to build in hardware mode, you will also need to install the [SGX driver](https://github.com/intel/linux-sgx-driver). Installation instructions can be found in each of these repositories. For a fully documented end to end setup please refer to this [getting started guide](https://aaronbedra.com/post/sgx_getting_started/).

## Compile

You can build this program by running `make`. If you do not have your SGX sdk installed at `/opt/intel/sgxsdk`, you will need to run the following:

```sh
source /path/to/sgxsdk/environment
```

You can then run `./bootstrapper`. The first execution will encrypt an integer inside the enclave and save the file `bootstrap.seal` to disk. Subsequent executions will read the `bootstrap.seal` file and send it to the enclave to be decrypted. The program will then display the integer previously encrypted by the enclave.

## Hardware vs Simulation Mode

By default, this program will compile in `SIM`, or Simulation mode. This ensures the program will work regardless of hardware support. If you wish to execute this program against a real SGX enclave, you will need to set `SGX_MODE=HW` when running `make`, or update the `Makefile` accordingly.
