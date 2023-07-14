# Cheshire

Cheshire is a minimal Linux-capable host platform built around the RISC-V [CVA6]() core. Its goal is to provide a *lightweight*, *configurable*, *autonomously booting* host to systems that need one, from minimal Linux-capable SoCs to manycore compute accelerators.

Cheshire is developed as part of the PULP project, a joint effort between ETH Zurich and the University of Bologna.

## Getting started

* To learn how to build and use Cheshire, see [Getting Started](https://pulp-platform.github.io/cheshire/gs/).
* To learn about available simulation, FPGA, and ASIC targets, see [Targets](https://pulp-platform.github.io/cheshire/tg).
* For detailed information on Cheshire's inner workings, consult the [User Manual](https://pulp-platform.github.io/cheshire/um/).

If you are impatient and have all needed [dependencies](https://pulp-platform.github.io/cheshire/gs/#dependencies), you can run `make all`, then start QuestaSim in `target/sim/vsim` and run:

```
set BINARY ../../../sw/tests/helloworld.spm.elf
source compile.tcl
source start.cheshire_soc.tcl
run -all
```

## License

Unless specified otherwise in the respective file headers, all code checked into this repository is made available under a permissive license. All hardware sources and tool scripts are licensed under the Solderpad Hardware License 0.51 (see `LICENSE`) with the exception of generated register file code (e.g. `hw/regs/*.sv`), which is generated by a fork of lowRISC's [`regtool`](https://github.com/lowRISC/opentitan/blob/master/util/regtool.py) and licensed under Apache 2.0. All software sources are licensed under Apache 2.0.


## Publication

If you use Cheshire in your work, you can cite us:

```
@misc{ottaviano2023cheshire,
      title         = {Cheshire: A Lightweight, Linux-Capable RISC-V Host
                       Platform for Domain-Specific Accelerator Plug-In},
      author        = {Alessandro Ottaviano and Thomas Benz and
                       Paul Scheffler and Luca Benini},
      year          = {2023},
      eprint        = {2305.04760},
      archivePrefix = {arXiv},
      primaryClass  = {cs.AR}
}
```
