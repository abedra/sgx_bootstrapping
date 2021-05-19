# SGX Bootstrapping Example

This project demonstrates the basics of sealing (encrypting) data inside an SGX enclave.

## Setup

Before compiling and running this program, you will need at least the [SGX sdk](https://github.com/intel/linux-sgx). If you wish to build in hardware mode, you will also need to install the [SGX driver](https://github.com/intel/linux-sgx-driver). Installation instructions can be found in each of these repositories. For a fully documented end to end setup please refer to this [getting started guide](https://aaronbedra.com/post/sgx_getting_started/).

## Compile

You can build this program by running `make`. If you do not have your SGX sdk installed at `/opt/intel/sgxsdk`, you will need to run the following:

```sh
source /path/to/sgxsdk/environment
```

## Hardware vs Simulation Mode

By default, this program will compile in `SIM`, or Simulation mode. This ensures the program will work regardless of hardware support. If you wish to execute this program against a real SGX enclave, you will need to set `SGX_MODE=HW` when running `make`, or update the `Makefile` accordingly.

## Execution

There are two programs created during compilation, `provision`, and `unseal`. Running `unseal` without provisioning will result in an error because there's no sealed data to provide to the enclave. You may run `provision` as much as you want. Running it will overwrite any existing sealed data files. To run `provision`, you will need to push the data you wish to seal to `stdin`. This program assumes you will provide an integer and will fail if one is not provided. You can use the linux shell variable `RANDOM` to provide this value under test. It should go without saying that this should never be used as a source of true randomness in a production setting.

```sh
echo $RANDOM | ./provision
persistence.seal does not exist, creating
persistence.seal saved with value: 28739
```

Once you have provisioned your secret, you can run `unseal` to retrieve it:

```sh
./unseal
persistence.seal unsealed to: 28739
```

In a real world scenario, we would never export our secrets. We would instead operate on them inside the enclave and return some computation that required them. This example serves to demonstrate the in and out process only.
