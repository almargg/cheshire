// Copyright 2022 ETH Zurich and University of Bologna.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// Modified version of the CVA6 testbench
// (https://github.com/openhwgroup/cva6,
// 99acdc271b90ce5abeb1b682eff4f999d0977ff4)
//
// Jannis Schönleber

#include "Vcheshire_testharness.h"
#include "verilated.h"
#include <verilated_vcd_c.h>
#if (VERILATOR_VERSION_INTEGER >= 5000000)
// Verilator v5 adds $root wrapper that provides rootp pointer.
#include "Vcheshire_testharness___024root.h"
#endif

#include <fesvr/dtm.h>
#include <cassert>
#include <chrono>
#include <cstring>
#include <ctime>
#include <fesvr/elfloader.h>
#include <fesvr/htif_hexwriter.h>
#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <string>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <chrono>



// This software is heavily based on Rocket Chip
// Checkout this awesome project:
// https://github.com/freechipsproject/rocket-chip/

// This is a 64-bit integer to reduce wrap over issues and
// allow modulus.  You can also use a double, if you wish.
static vluint64_t main_time = 0;
int clk_ratio = 2;
unsigned long rtc_counter = 0;

static void cycle_start(std::shared_ptr<Vcheshire_testharness> top, VerilatedVcdC *m_trace) {
    top->rtc_i = 1;
    top->jtag_tck = 1;
    for (int i = 0; i < clk_ratio; i++) {
        top->clk_i = 1;
        top->eval();
        m_trace->dump(main_time);
        main_time += 2500;
        top->clk_i = 0;
        top->eval();
        m_trace->dump(main_time);
        main_time += 2500;
        ++rtc_counter;
        if (rtc_counter > 3052) {
            top->rtc_i = 1 - top->rtc_i;
            rtc_counter = 0; 
        }
    }
}

static void cycle_end(std::shared_ptr<Vcheshire_testharness> top, VerilatedVcdC *m_trace) {
    top->rtc_i = 0;
    for (int i = 0; i < clk_ratio; i++) {
        top->clk_i = 1;
        top->eval();
        m_trace->dump(main_time);
        main_time += 2500;
        top->clk_i = 0;
        top->eval();
        m_trace->dump(main_time);
        main_time += 2500;
        if (rtc_counter > 3052) {
            top->rtc_i = 1 - top->rtc_i;
            rtc_counter = 0; 
        }
    }
}

static void wait_cycles(std::shared_ptr<Vcheshire_testharness> top, int cycles, VerilatedVcdC *m_trace) {
    for (int i = 0; i < cycles; i++) {
        cycle_start(top, m_trace);
        cycle_end(top, m_trace);
    }
}

int main(int argc, char **argv) {

    std::ofstream myfile;
    myfile.open ("PC.txt");
    using namespace std::literals::chrono_literals;

    auto start = std::chrono::high_resolution_clock::now();


    char ** htif_argv = NULL;
    int htif_argc = 1 + argc - optind;
  	htif_argv = (char **) malloc((htif_argc) * sizeof (char *));
  	htif_argv[0] = argv[0];
  	for (int i = 1; optind < argc;) htif_argv[i++] = argv[optind++];

    Verilated::commandArgs(argc, argv);
    std::shared_ptr<Vcheshire_testharness> top(new Vcheshire_testharness);

    Verilated::traceEverOn(true); // enable trace
    VerilatedVcdC *m_trace = new VerilatedVcdC;
    top->trace(m_trace, 100);
    m_trace->open("waveform.vcd");

    //std::cout << "preparing elf\n" << htif_argv[1] << "\n";
	htif_hexwriter_t htif(0x0, 1, -1);
  	memif_t memif(&htif);
  	reg_t entry;
 	load_elf(htif_argv[1], &memif, &entry);

    // reset
    for (int i = 0; i < 10; i++) {
        top->rst_ni = 0;
        top->rtc_i = 0;
        wait_cycles(top, 5, m_trace);
    }
    top->rst_ni = 1;

    size_t mem_size = 0xFFFF; // max 299695
    memif.read(0x10000000, mem_size, (void *)top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__gen_llc__DOT__i_llc__DOT__i_axi_llc_top_raw__DOT__i_llc_ways__DOT__gen_data_ways__BRA__0__KET____DOT__i_data_way__DOT__i_data_sram__DOT__sram.data());

    std::cout << "\n";
    unsigned long adress = 0x10000000;

    top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__i_regs__DOT__u_scratch_0__DOT__q = adress & 0xFFFFFFFF;
    top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__i_regs__DOT__u_scratch_1__DOT__q = (adress >> 32) & 0xFFFFFFFF;

    top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__i_regs__DOT__u_scratch_2__DOT__q = 2;

    top->rootp->boot_mode_i = 0;


   
    
    unsigned int done = 0;
    int i = 0;
    unsigned long pc_last = 0;

    wait_cycles(top, 5, m_trace);
    do {
        /*if (pc_last != top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__i_core_cva6__DOT__i_frontend__DOT__i_instr_queue__DOT__pc_q) {
            //myfile << std::hex << top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__i_core_cva6__DOT__i_frontend__DOT__i_instr_queue__DOT__pc_q << "\n";
            pc_last = top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__i_core_cva6__DOT__i_frontend__DOT__i_instr_queue__DOT__pc_q;
        }*/
        cycle_start(top, m_trace);
        /*if (pc_last != top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__i_core_cva6__DOT__i_frontend__DOT__i_instr_queue__DOT__pc_q) {
            //myfile << std::hex << top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__i_core_cva6__DOT__i_frontend__DOT__i_instr_queue__DOT__pc_q << "\n";
            pc_last = top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__i_core_cva6__DOT__i_frontend__DOT__i_instr_queue__DOT__pc_q;
        }*/
        done |= top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__i_regs__DOT__u_scratch_2__DOT__q & 1;
        cycle_end(top, m_trace);
        done |= top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__i_regs__DOT__u_scratch_2__DOT__q & 1;
        ++i;
    } while (i < 100000 && !done);
   
 /*
    for (unsigned i = 0; i < 100; ++i) {
        if (pc_last != top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__i_core_cva6__DOT__i_frontend__DOT__i_instr_queue__DOT__pc_q) {
            myfile << std::hex << top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__i_core_cva6__DOT__i_frontend__DOT__i_instr_queue__DOT__pc_q << "\n";
            pc_last = top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__i_core_cva6__DOT__i_frontend__DOT__i_instr_queue__DOT__pc_q;
        }
        cycle_start(top, m_trace);
        if (pc_last != top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__i_core_cva6__DOT__i_frontend__DOT__i_instr_queue__DOT__pc_q) {
            myfile << std::hex << top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__i_core_cva6__DOT__i_frontend__DOT__i_instr_queue__DOT__pc_q << "\n";
            pc_last = top->rootp->cheshire_testharness__DOT__i_cheshire__DOT__i_core_cva6__DOT__i_frontend__DOT__i_instr_queue__DOT__pc_q;
        }
        cycle_end(top, m_trace);
    }
    */

    std::cout << "Programm ran: " << done << "\n";
    myfile.close();
    m_trace->close();
    top->final();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float>  duration = end - start;

    std::cout << "Simulating time " << duration.count() << "\n";
    std::cout << "\nSimulation successful\n";

    return 0;
}
