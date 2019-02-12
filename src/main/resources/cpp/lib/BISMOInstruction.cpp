#include "BISMOInstruction.hpp"

#ifndef __SYNTHESIS__
#include <iostream>
#include <iomanip>

std::ostream& operator<<(std::ostream& os, const BISMOSyncInstruction& dt)
{
    os << "sync " << (dt.isSendToken ? "send" : "receive");
    os << " chanID="<< dt.chanID << std::endl;
    return os;
}

std::ostream& operator<<(std::ostream& os, const BISMOFetchRunInstruction& r)
{
  os << "Fetch config ============================" << std::endl;
  os << "bram_addr_base: " << r.bram_addr_base << std::endl;
  os << "bram_id_start: " << r.bram_id_start << std::endl;
  os << "bram_id_range: " << r.bram_id_range << std::endl;
  os << "tiles_per_row: " << r.tiles_per_row << std::endl;
  os << "dram_base: " << (uint64_t) r.dram_base << std::endl;
  os << "dram_block_offset_bytes: " << r.dram_block_offset_bytes << std::endl;
  os << "dram_block_size_bytes: " << r.dram_block_size_bytes << std::endl;
  os << "dram_block_count: " << r.dram_block_count << std::endl;
  os << "========================================" << std::endl;
  return os;
}

std::ostream& operator<<(std::ostream& os, const BISMOExecRunInstruction& r)
{
  os << "Exec config ============================" << std::endl;
  os << "lhsOffset: " << r.lhsOffset << std::endl;
  os << "rhsOffset: " << r.rhsOffset << std::endl;
  os << "negate: " << r.negate << std::endl;
  os << "numTiles: " << r.numTiles << std::endl;
  os << "shiftAmount: " << r.shiftAmount << std::endl;
  os << "clear_before_first_accumulation: " << r.clear_before_first_accumulation << std::endl;
  os << "writeEn: " << r.writeEn << std::endl;
  os << "writeAddr: " << r.writeAddr << std::endl;
  os << "========================================" << std::endl;
}

std::ostream& operator<<(std::ostream& os, const BISMOResultRunInstruction& r)
{
  os << "Result config ============================" << std::endl;
  os << "dram_base: " << r.dram_base << std::endl;
  os << "dram_skip: " << r.dram_skip << std::endl;
  os << "resmem_addr: " << r.resmem_addr << std::endl;
  os << "nop: " << r.nop << std::endl;
  os << "waitCompleteBytes: " << r.waitCompleteBytes << std::endl;
  os << "========================================" << std::endl;
}


std::ostream& operator<<(std::ostream& os, const BISMOInstruction& dt)
{
    /*os << dt << std::endl;
    os.fill('0');*/
    os << "raw " << dt.to_string(16) << std::endl;
    BISMOSyncInstruction sync;
    sync.fromRaw(dt);
    os << "targetStage " << sync.targetStage << " runcfg? " << sync.isRunCfg << std::endl;
    if(sync.isRunCfg == 0) {
      os << sync;
    } else {
      if(sync.targetStage == 0) {
        BISMOFetchRunInstruction fetch;
        fetch.fromRaw(dt);
        os << fetch;
      } else if(sync.targetStage == 1) {
        BISMOExecRunInstruction exec;
        exec.fromRaw(dt);
        os << exec;
      } else if(sync.targetStage == 2) {
        BISMOResultRunInstruction res;
        res.fromRaw(dt);
        os << res;
      } else {
        os << "illegal target stage";
      }
    }
    return os;
}

#endif