/*******************************************************************************
# Copyright (c) 2018, Xilinx, Inc.
# All rights reserved.
# Author: Yaman Umuroglu
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
#
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software
# without specific prior written permission.
#
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# *******************************************************************************/

#include <ap_int.h>
#include <hls_stream.h>
#include <stdint.h>
#include "BISMOInstruction.hpp"

void FetchInstrGen(
  hls::stream<ap_uint<BISMO_MMDESCR_BITS>> & in,
  hls::stream<ap_uint<128>> & out
) {
  #pragma HLS INTERFACE ap_ctrl_none port=return
  #pragma HLS INTERFACE axis port=out
  #pragma HLS INTERFACE axis port=in

  BISMOFetchRunInstruction fetch;
  BISMOSyncInstruction sync;

  // set the invariants (values do not depend on loop iter)
  sync.targetStage = stgFetch;
  fetch.targetStage = stgFetch;
  fetch.isRunCfg = 1;
  sync.isRunCfg = 0;
  // read the descriptor
  SingleMMDescriptor ins_in;
  ins_in.fromRaw(in.read());

  // start by acquiring buffer to fill
  // receive token from execute stage
  sync.isSendToken = 0;
  sync.chanID = 0;
  out.write(sync.asRaw());

  // number of lhs and rhs tiles
  const uint32_t lhs_tiles = ins_in.tiles_m * ins_in.tiles_k;
  const uint32_t rhs_tiles = ins_in.tiles_n * ins_in.tiles_k;

  // TODO make these part of HW template:
  const int exec_to_fetch_width_ratio = 1;
  const int d_m = 2;
  const int d_k = 128;
  const int d_n = 2;
  const int first_lhs_id = 0;
  const int first_rhs_id = d_m;
  const int bytes_per_lhs_tile = (d_m * d_k) / 8;
  const int bytes_per_rhs_tile = (d_n * d_k) / 8;

  // prepare fetch instruction for LHS matrix
  fetch.bram_addr_base = ins_in.base_l * exec_to_fetch_width_ratio;
  fetch.bram_id_start = first_lhs_id;
  fetch.bram_id_range = d_m - 1;
  // how many DRAM data words are copied before the
  // fetch interconnect starts targeting the next BRAM
  fetch.tiles_per_row = ins_in.tiles_k * exec_to_fetch_width_ratio;
  // DRAM base address for LHS
  fetch.dram_base = ins_in.dram_lhs;
  // bytes to read in each contiguous block
  fetch.dram_block_size_bytes = lhs_tiles * bytes_per_lhs_tile;
  // TODO partial matrices will need multiple blocks here
  fetch.dram_block_count = 1;
  fetch.dram_block_offset_bytes = 0;
  // emit fetch instruction for LHS matrix
  out.write(fetch.asRaw());

  // prepare fetch instruction for RHS matrix
  fetch.bram_addr_base = ins_in.base_r * exec_to_fetch_width_ratio;
  fetch.bram_id_start = first_rhs_id;
  fetch.bram_id_range = d_n - 1;
  // how many DRAM data words are copied before the
  // fetch interconnect starts targeting the next BRAM
  fetch.tiles_per_row = ins_in.tiles_k * exec_to_fetch_width_ratio;
  // DRAM base address for LHS
  fetch.dram_base = ins_in.dram_rhs;
  // bytes to read in each contiguous block
  fetch.dram_block_size_bytes = rhs_tiles * bytes_per_rhs_tile;
  // TODO partial matrices will need multiple blocks here
  fetch.dram_block_count = 1;
  fetch.dram_block_offset_bytes = 0;
  // emit fetch instruction for RHS matrix
  out.write(fetch.asRaw());

  // signal that buffer is now filled
  // send token to execute stage
  sync.isSendToken = 1;
  sync.chanID = 0;
  out.write(sync.asRaw());
}
