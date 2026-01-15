#ifndef __UVM_SHA3_H__
#define __UVM_SHA3_H__

#define HWPE_SHA3_BASE_ADDR    0x1a10c100

// HWPE registers
#define SHA3_TRIGGER             HWPE_SHA3_BASE_ADDR + 0x00
#define SHA3_SOFT_CLEAR          HWPE_SHA3_BASE_ADDR + 0x14
#define SHA3_ENGINE_CTRL         HWPE_SHA3_BASE_ADDR + 0x20
#define SHA3_TEXT_SIZE           HWPE_SHA3_BASE_ADDR + 0x24

#define SHA3_BLOCK_SIZE     64

struct __packed sha3_hwpe_reg_t 
{
    uint32_t src_base_addr;
    uint32_t sink_base_addr;
    uint32_t src_transf_size;
    uint32_t sink_transf_size;
    uint32_t src_line_length;
    uint32_t src_line_stride;
    uint32_t src_feat_length;
    uint32_t src_feat_stride;
    uint32_t sink_line_length;
    uint32_t sink_line_stride;
    uint32_t sink_feat_length;
    uint32_t sink_feat_stride;
};

volatile struct sha3_hwpe_reg_t *sha3_hwpe_reg = (volatile struct sha3_hwpe_reg_t *)(HWPE_SHA3_BASE_ADDR + 0x40);

void hwpe_run_sha3(int bytes, int src_add, int sink_add)
{
  plp_hwme_enable();
  int msg_size = ((bytes - 1) / 4) + 1;
  
  // soft clear
  pulp_write32(SHA3_SOFT_CLEAR, 0);
  rt_time_wait_us(50);
  
  // source base addresses
  sha3_hwpe_reg->src_base_addr = src_add;
  // sink base addresses
  sha3_hwpe_reg->sink_base_addr = sink_add;
  
  // trans size - both source and sink
  sha3_hwpe_reg->src_transf_size = msg_size;
  sha3_hwpe_reg->sink_transf_size = 16; // 512 bytes (16 x 32 bits)
  
  //*** SOURCE PARAMETERS ***//
  // source line length
  sha3_hwpe_reg->src_line_length = 0x1;
  // source line stride
  sha3_hwpe_reg->src_line_stride = 0x4;
  // source feat length
  sha3_hwpe_reg->src_feat_length = msg_size;
  // source feat stride
  sha3_hwpe_reg->src_feat_stride = 0x0;
  
  //*** SINK PARAMETERS ***//
  // sink line length
  sha3_hwpe_reg->sink_line_length = 0x1;
  // sink line stride
  sha3_hwpe_reg->sink_line_stride = 0x4;
  // sink feat length
  sha3_hwpe_reg->sink_feat_length = 16;
  // sink feat stride
  sha3_hwpe_reg->sink_feat_stride = 0x0;
  
  // ENGINE controller parameters
  pulp_write32(SHA3_TEXT_SIZE, bytes); // TEXT_SIZE must be defined before reset
  pulp_write32(SHA3_ENGINE_CTRL, 0x1); // resetting sha core
  pulp_write32(SHA3_ENGINE_CTRL, 0x0); 
  
  // trigger
  pulp_write32(SHA3_TRIGGER, 0);
  
  soc_eu_fcEventMask_setEvent(ARCHI_SOC_EVENT_FCHWPE1);
  __rt_periph_wait_event(ARCHI_SOC_EVENT_FCHWPE1, 1);

  plp_hwme_disable();
  return;
}

#endif // __UVM_SHA3_H__
