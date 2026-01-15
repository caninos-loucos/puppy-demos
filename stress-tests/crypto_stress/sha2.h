#ifndef __UVM_SHA_H__
#define __UVM_SHA_H__

#define HWPE_SHA2_BASE_ADDR    0x1a10c200

// HWPE registers
#define SHA2_TRIGGER             HWPE_SHA2_BASE_ADDR + 0x00
#define SHA2_SOFT_CLEAR          HWPE_SHA2_BASE_ADDR + 0x14
#define SHA2_ENGINE_CTRL         HWPE_SHA2_BASE_ADDR + 0x20
#define SHA2_TEXT_SIZE           HWPE_SHA2_BASE_ADDR + 0x24

#define BLOCK_SIZE    64    // 512 bits or 64 bytes

struct __packed sha2_hwpe_reg_t 
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

volatile struct sha2_hwpe_reg_t *sha2_hwpe_reg = (volatile struct sha2_hwpe_reg_t *)(HWPE_SHA2_BASE_ADDR + 0x40);

/*
 * msg : message pointer
 * msg_size : message size in bytes
 */
void message_padding(uint8_t *msg, int msg_size)
{
  int padding_zeros = BLOCK_SIZE - msg_size - 1 - 8;
  int index = msg_size;
  int bits = msg_size * 8;
  
  msg[index++] = 0x80;
  for (int i = 0; i < padding_zeros; i++)
    msg[index++] = 0x00;
  
  for (int i = 0; i < 4; i++)
    msg[index++] = 0x00;
  
  for (int i = 0; i < 4; i++)
    msg[index++] = bits >> ((3 - i) * 8);
}

void hwpe_run_sha2(int bytes, int src_add, int sink_add)
{
  plp_hwme_enable();
  int msg_size = ((bytes - 1) / 4) + 1;
  
  // soft clear
  pulp_write32(SHA2_SOFT_CLEAR, 0);
  rt_time_wait_us(50);
  
  // source base addresses
  sha2_hwpe_reg->src_base_addr = src_add;
  // sink base addresses
  sha2_hwpe_reg->sink_base_addr = sink_add;
  
  // trans size - both source and sink
  sha2_hwpe_reg->src_transf_size = msg_size;
  sha2_hwpe_reg->sink_transf_size = 8; // 256 bytes (8 x 32 bits)
  
  //*** SOURCE PARAMETERS ***//
  // source line length
  sha2_hwpe_reg->src_line_length = 0x1;
  // source line stride
  sha2_hwpe_reg->src_line_stride = 0x4;
  // source feat length
  sha2_hwpe_reg->src_feat_length = msg_size;
  // source feat stride
  sha2_hwpe_reg->src_feat_stride = 0x0;
  
  //*** SINK PARAMETERS ***//
  // sink line length
  sha2_hwpe_reg->sink_line_length = 0x1;
  // sink line stride
  sha2_hwpe_reg->sink_line_stride = 0x4;
  // sink feat length
  sha2_hwpe_reg->sink_feat_length = 16;
  // sink feat stride
  sha2_hwpe_reg->sink_feat_stride = 0x0;
  
  // ENGINE controller parameters
  pulp_write32(SHA2_TEXT_SIZE, bytes); // SHA2_TEXT_SIZE must be defined before reset
  pulp_write32(SHA2_ENGINE_CTRL, 0x1); // resetting sha core
  pulp_write32(SHA2_ENGINE_CTRL, 0x0); 
  
  // trigger
  pulp_write32(SHA2_TRIGGER, 0);
  
  soc_eu_fcEventMask_setEvent(142);
  __rt_periph_wait_event(142, 1);

  plp_hwme_disable();
  return;
}

#endif // __UVM_SHA_H__
