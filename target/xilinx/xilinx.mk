# Copyright 2022 ETH Zurich and University of Bologna.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0
#
# Nicole Narr <narrn@student.ethz.ch>
# Christopher Reinwardt <creinwar@student.ethz.ch>
# Cyril Koenig <cykoenig@iis.ee.ethz.ch>

CHS_XIL_DIR  ?= $(CHS_ROOT)/target/xilinx
VIVADO       ?= vitis-2020.2 vivado

PROJECT      ?= cheshire
BOARD        ?= vcu128
ip-dir       := $(CHS_XIL_DIR)/xilinx

# Select board specific variables
ifeq ($(BOARD),vcu128)
	XILINX_PART  ?= xcvu37p-fsvh2892-2L-e
	XILINX_BOARD ?= xilinx.com:vcu128:part0:1.0
	ips-names    := xlnx_mig_ddr4 xlnx_clk_wiz xlnx_vio
	ifeq ($(INT_JTAG),1)
		xilinx_targs += -t bscane
	endif
endif
ifeq ($(BOARD),genesys2)
	XILINX_PART  ?= xc7k325tffg900-2
	XILINX_BOARD ?= digilentinc.com:genesys2:part0:1.1
	ips-names    := xlnx_mig_7_ddr3 xlnx_clk_wiz xlnx_vio
	FPGA_PATH    ?= xilinx_tcf/Digilent/200300A8C60DB
endif
ifeq ($(BOARD),zcu102)
	XILINX_PART    ?= xczu9eg-ffvb1156-2-e
	XILINX_BOARD   ?= xilinx.com:zcu102:part0:3.4
	ips-names      := xlnx_mig_ddr4 xlnx_clk_wiz xlnx_vio
endif

# Location of ip outputs
ips := $(addprefix $(CHS_XIL_DIR)/,$(addsuffix .xci ,$(basename $(ips-names))))
# Derive bender args from enabled ips
xilinx_targs += -t fpga -t cv64a6_imafdcsclic_sv39 -t cva6
xilinx_targs += $(foreach ip-name,$(ips-names),$(addprefix -t ,$(ip-name)))
xilinx_targs += $(addprefix -t ,$(BOARD))

# Outputs
out := $(CHS_XIL_DIR)/out
bit := $(out)/$(PROJECT)_top_xilinx.bit
mcs := $(out)/$(PROJECT)_top_xilinx.mcs

# Vivado variables
VIVADOENV ?=  PROJECT=$(PROJECT)            \
              BOARD=$(BOARD)                \
              XILINX_PART=$(XILINX_PART)    \
              XILINX_BOARD=$(XILINX_BOARD)  \
              PORT=$(XILINX_PORT)           \
              HOST=$(XILINX_HOST)           \
              FPGA_PATH=$(FPGA_PATH)        \
              BIT=$(bit)                    \
              IP_PATHS="$(foreach ip-name,$(ips-names),xilinx/$(ip-name)/$(ip-name).srcs/sources_1/ip/$(ip-name)/$(ip-name).xci)"

MODE        ?= gui
VIVADOFLAGS ?= -nojournal -mode $(MODE)

chs-xil-all: $(bit)

# Generate mcs from bitstream
$(mcs): $(bit)
	cd $(CHS_XIL_DIR) && $(VIVADOENV) $(VIVADO) $(VIVADOFLAGS) -source scripts/write_cfgmem.tcl -tclargs $@ $^

# Compile bitstream
$(bit): $(ips) $(CHS_XIL_DIR)/scripts/add_sources.tcl
	@mkdir -p $(out)
	cd $(CHS_XIL_DIR) && $(VIVADOENV) $(VIVADO) $(VIVADOFLAGS) -source scripts/prologue.tcl -source scripts/run.tcl
	cp $(CHS_XIL_DIR)/$(PROJECT).runs/impl_1/*.bit $(out)
	cp $(CHS_XIL_DIR)/$(PROJECT).runs/impl_1/*.ltx $(out)

# Generate ips
%.xci:
	@echo $@
	@echo $(CHS_XIL_DIR)
	@echo "Generating IP $(basename $@)"
	IP_NAME=$(basename $(notdir $@)) ; cd $(ip-dir)/$$IP_NAME && $(MAKE) clean && $(VIVADOENV) VIVADO="$(VIVADO)" $(MAKE)
	IP_NAME=$(basename $(notdir $@)) ; cp $(ip-dir)/$$IP_NAME/$$IP_NAME.srcs/sources_1/ip/$$IP_NAME/$$IP_NAME.xci $@

chs-xil-gui:
	@echo "Starting $(vivado) GUI"
	cd $(CHS_XIL_DIR) && $(VIVADOENV) $(VIVADO) -nojournal -mode gui $(PROJECT).xpr &

chs-xil-program: #$(bit)
	@echo "Programming board $(BOARD) ($(XILINX_PART))"
	cd $(CHS_XIL_DIR) && $(VIVADOENV) $(VIVADO) $(VIVADOFLAGS) -source scripts/program.tcl

chs-xil-clean:
	cd $(CHS_XIL_DIR) && rm -rf scripts/add_sources.tcl* *.log *.jou *.str *.mif *.xci *.xpr .Xil/ $(out) $(PROJECT).srcs $(PROJECT).cache $(PROJECT).hw $(PROJECT).ioplanning $(PROJECT).ip_user_files $(PROJECT).runs $(PROJECT).sim

# Re-compile only top and not ips
chs-xil-rebuild-top:
	${MAKE} chs-xil-clean
	find $(CHS_XIL_DIR)/xilinx -wholename "**/*.srcs/**/*.xci" | xargs -n 1 -I {} cp {} $(CHS_XIL_DIR)
	${MAKE} $(bit)

chs-xil-flash: $(CHS_SW_DIR)/boot/linux.gpt.bin
	$(VIVADOENV) $(VIVADO) $(VIVADOFLAGS) FILE=$(CHS_SW_DIR)/boot/linux.gpt.bin OFFSET=0 -source $(CHS_XIL_DIR)/scripts/program.tcl

# Bender script
$(CHS_XIL_DIR)/scripts/add_sources.tcl: Bender.yml
	$(BENDER) script vivado $(xilinx_targs) > $@

.PHONY: chs-xil-gui chs-xil-program chs-xil-clean chs-xil-rebuild-top chs-xil-all