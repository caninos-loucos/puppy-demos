#ifndef __CRYPTO_AES_H__
#define __CRYPTO_AES_H__

#define NONCE_L       0xabcdef01    // must match the nonce in the python stimuli generator
#define NONCE_H       0xabcdef01    // must match the nonce in the python stimuli generator

#define HWPE_AES_BASE_ADDR    0x1a10c000

// HWPE registers
#define AES_TRIGGER             HWPE_AES_BASE_ADDR + 0x00
#define AES_SOFT_CLEAR          HWPE_AES_BASE_ADDR + 0x14
#define AES_ENGINE_CTRL         HWPE_AES_BASE_ADDR + 0x20
#define AES_NONCE_BASE_ADDR     HWPE_AES_BASE_ADDR + 0x24   // 4 registers
/* 
 * AES_KEY_SEL: selects index for write (key is divided in 32-bit words)
 *              key is passed via AES_KEY_ADDR register
 * bits 3 - 0 : index/selector
 * bit  4     : write enable
 * bits 7 - 6 : key size (00b -> 256 ; 01b -> 192 ; 10b -> 128)
 */
#define AES_KEY_SEL             HWPE_AES_BASE_ADDR + 0x34
#define AES_KEY_ADDR            HWPE_AES_BASE_ADDR + 0x38

struct __packed aes_hwpe_reg_t 
{
    uint32_t src_base_addr[4];
    uint32_t sink_base_addr[4];
    uint32_t transf_size;
    uint32_t src_line_length;
    uint32_t src_line_stride;
    uint32_t src_feat_length;
    uint32_t src_feat_stride;
    uint32_t sink_line_length;
    uint32_t sink_line_stride;
    uint32_t sink_feat_length;
    uint32_t sink_feat_stride;
};

volatile struct aes_hwpe_reg_t *aes_hwpe_reg = (volatile struct aes_hwpe_reg_t *)(HWPE_AES_BASE_ADDR + 0x40);

/*
 * msg_size : in groups of 128-bits
 * src_add  : base address of source data
 * sink_add : base address of sink data
 * key_size : 00b - 256 bit key
 *            01b - 192 bit key
 *            10b - 128 bit key
 * mode     : 0 - CTR mode
 *            1 - CBC mode
 */
void hwpe_run(int msg_size, int src_add, int sink_add, int key_size, int mode)
{
  int msg_size_comp = msg_size + 1;  // msg_size compensated to fix failures with certain sizes (e.g. 67, 100)
//   int msg_size_comp = msg_size;
  plp_hwme_enable();
  
  // soft clear
  pulp_write32(AES_SOFT_CLEAR, 0);
  rt_time_wait_us(50);
  
  // enable clock gating on engine
  pulp_write32(AES_ENGINE_CTRL, 0x00000001 | (mode << 2));

  // nonce
  pulp_write32(AES_NONCE_BASE_ADDR, 0);
  pulp_write32(AES_NONCE_BASE_ADDR + 0x4, 0);
  pulp_write32(AES_NONCE_BASE_ADDR + 0x8, NONCE_L);
  pulp_write32(AES_NONCE_BASE_ADDR + 0xC, NONCE_H);
  
  // reset counter to load nonce
  pulp_write32(AES_ENGINE_CTRL, 0x00000003 | (mode << 2));
  pulp_write32(AES_ENGINE_CTRL, 0x00000001 | (mode << 2));
  
  // key : 0xabcdef01_abcdef01_abcdef01_abcdef01_abcdef01_abcdef01_abcdef01_abcdef01
  for (int i = 0; i < 8; i++)
  {
    pulp_write32(AES_KEY_SEL, i | (1 << 4) | (key_size << 6));
    pulp_write32(AES_KEY_ADDR, 0xabcdef01);
  }
  pulp_write32(AES_KEY_SEL, (key_size << 6));
  
  for (int i = 0; i < 4; i++)
  {
    // source base addresses
    aes_hwpe_reg->src_base_addr[i] = src_add + i * 0x4;
    // sink base addresses
    aes_hwpe_reg->sink_base_addr[i] = sink_add + i * 0x4;
  }
  
  // trans size - both source and sink
  aes_hwpe_reg->transf_size = msg_size_comp;
  //*** SOURCE PARAMETERS ***//
  // source line length
  aes_hwpe_reg->src_line_length = 0x1;
  // source line stride
  aes_hwpe_reg->src_line_stride = 0x10;
  // source feat length
  aes_hwpe_reg->src_feat_length = msg_size_comp;
  // source feat stride
  aes_hwpe_reg->src_feat_stride = 0x0;
  
  //*** SINK PARAMETERS ***//
  // sink line length
  aes_hwpe_reg->sink_line_length = 0x1;
  // sink line stride
  aes_hwpe_reg->sink_line_stride = 0x10;
  // sink feat length
  aes_hwpe_reg->sink_feat_length = msg_size_comp;
  // sink feat stride
  aes_hwpe_reg->sink_feat_stride = 0x0;
  
  // trigger
  pulp_write32(AES_TRIGGER, 0);
  
  soc_eu_fcEventMask_setEvent(ARCHI_SOC_EVENT_FCHWPE0);
  __rt_periph_wait_event(ARCHI_SOC_EVENT_FCHWPE0, 1);

  plp_hwme_disable();
  
  return;
}

#endif // __CRYPTO_AES_H__
