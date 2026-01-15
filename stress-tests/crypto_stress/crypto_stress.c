
#include "rt/rt_api.h"
#include "stdio.h"
#include "archi/hwme/hwme_v1.h"
#include "hal/hwme/hwme_v1.h"


/********************* USER CONFIGURABLE DEFINES *********************/

// TEST_* macros must be defined before expected_out.h include
#define TEST_AES        1
#define TEST_SHA256     0
#define TEST_SHA3       0

// VERBOSE : show comparison per data
#define VERBOSE         0

// TINY_DATA : change input and expected output data sources.
// If 1, input for tests are fixed 1 msg/block size and expected 
// output is read from expected_out.h file.
// Otherwise, input and expected output data are copied from external 
// flash.
#define TINY_DATA       0

// BX_DATA_BASE_ADDR : Base address for memory blocks in the 
// interleaved memory RAM on the SoC
// For "non-Tiny" data tests, these memory blocks will be used
// alternating between inputs, calculated outputs and expected outputs.
#define B0_DATA_BASE_ADDR       0x1C011000
#define B1_DATA_BASE_ADDR       0x1C030000

// FL_XX_XX_ADDR : Address for data in the external flash
#define FL_AES_FIRST_DATA_ADDR  0x010000
#define FL_AES_IN2OUT_OFFSET    0x018000
#define FL_AES_MODE_OFFSET      6 * FL_AES_IN2OUT_OFFSET
#define FL_AES_KEY_OFFSET       2 * FL_AES_IN2OUT_OFFSET

#define FL_SHA2_FIRST_DATA_ADDR     0x200000
#define FL_SHA2_IN2OUT_OFFSET       0x018000

#define FL_SHA3_FIRST_DATA_ADDR     0x230000
#define FL_SHA3_IN2OUT_OFFSET       0x018000

#if TINY_DATA == 0
#define AES_DATA_SIZE   100     // number of 128-bit inputs
#define SHA2_DATA_SIZE  1024
#define SHA3_DATA_SIZE  1024     // number of 512-bit inputs
#define SHA3_MSG_SIZE   64      // size of the input in bytes
#endif 

// CARRIAGE RETURN : change/add special character for newline mode
// 0: \n for newline
// 1: \n\r for newline
#define CARRIAGE_RETURN     0

#define TEXT_REP        10000

/*********************************************************************/



#include "expected_out.h"
#include "color_code.h"
#include "utility.h"

unsigned int __rt_iodev_uart_baudrate = 115200;

unsigned int start_time, end_time;

unsigned int iter = 0;

#if TINY_DATA == 0

#include "spi_flash.h"
rt_spim_t *spim;

void spi_setup()
{
    rt_spim_conf_t conf;
    rt_spim_conf_init(&conf);
    conf.max_baudrate = 20000000;
    conf.id = 0; 
    conf.cs = 1;

    spim = rt_spim_open(NULL, &conf, NULL);
}

#endif 

void count_iter()
{
    iter++;
    if (iter >= TEXT_REP)
    {
        printf("Ran %d times", iter);
        newline(1);
        iter = 0;
    }
}

#if TEST_AES

#include "aes.h"

#define   KEY_128   0x2
#define   KEY_192   0x1
#define   KEY_256   0x0

#define   CTR_MODE  0
#define   CBC_MODE  1

char *key_size_str[3] = {
    "256 bit",
    "192 bit",
    "128 bit",
};

char *aes_mode_str[2] = {
    "CTR Mode",
    "CBC Mode",
};


typedef struct {
  int msg_size;
  int key_size;
  int mode;
} aes_data_t;

static void enqueue_aes(void* arg)
{
    aes_data_t *data = (aes_data_t *)arg;
    
    // calculate ciphertext
    hwpe_run(data->msg_size, B0_DATA_BASE_ADDR, B1_DATA_BASE_ADDR, data->key_size, data->mode);
    count_iter();
}
  
int stress_aes(aes_data_t *data)
{
    uint32_t *b0 = (uint32_t *) B0_DATA_BASE_ADDR;
    uint32_t *b1 = (uint32_t *) B1_DATA_BASE_ADDR;
    int error = 0;
    int msg_size;   // in 128-bit blocks
    char* title;
#if TINY_DATA == 0
    uint32_t flash_addr;
#endif
    
    sprintf(title, "AES %s - %s key", aes_mode_str[data->mode], key_size_str[data->key_size]);
   
    print_title(title);
    
    // Loading input data
#if TINY_DATA
    msg_size = 1;
    for (int i = 0; i < msg_size * 4; i++)
        b0[i] = i;
#else
    printf("Loading data... ");
    msg_size = AES_DATA_SIZE;
    flash_addr = FL_AES_FIRST_DATA_ADDR + (data->mode * FL_AES_MODE_OFFSET) + (data->key_size * FL_AES_KEY_OFFSET);
    
    start_time = rt_time_get_us();
    readData(spim, flash_addr, (uint8_t *) B0_DATA_BASE_ADDR, msg_size * 16);
    
    printf("%u us", rt_time_get_us() - start_time);
    newline(1);
#endif
    
    printf("Processing %d data (128-bit inputs)", msg_size);
    newline(1);
    
    enqueue_aes(data);
    
    return 0;
}

#endif // TEST_AES

#if TEST_SHA256

#include "sha2.h"

static void enqueue_sha2()
{
    // calculate ciphertext
    hwpe_run_sha2(BLOCK_SIZE * SHA2_DATA_SIZE, 
                  B0_DATA_BASE_ADDR,
                  B1_DATA_BASE_ADDR
                 );    // sha256 output is 256 bits or 32 bytes
    count_iter();
}

int stress_sha256()
{
    int msg_size = 0;
    int data_size;
    uint8_t pre_msg[BLOCK_SIZE];
    uint8_t hash_reverse[BLOCK_SIZE];
    uint32_t *b0 = (uint32_t *) B0_DATA_BASE_ADDR;
    uint32_t *b1 = (uint32_t *) B1_DATA_BASE_ADDR;
    int error = 0;
#if TINY_DATA == 0
    uint32_t flash_addr;
#endif
    char* title;
    
    sprintf(title, "SHA2 256-bit");
    
    print_title(title);
    
    // Loading input data
#if TINY_DATA
    msg_size = sprintf(pre_msg, "Hello");
    data_size = 1;
    message_padding(pre_msg, msg_size);
    reverse_byte(pre_msg, (uint8_t *)b0, data_size * 16);
#else 
    printf("Loading data... ");
    data_size = SHA2_DATA_SIZE;
    flash_addr = FL_SHA2_FIRST_DATA_ADDR;
    
    start_time = rt_time_get_us();
    readData(spim, flash_addr, (uint8_t *) B0_DATA_BASE_ADDR, data_size * 64);
    
    printf("%u us", rt_time_get_us() - start_time);
    newline(1);
#endif 
    
    printf("Processing %d data (512-bit inputs)", data_size);
    newline(1);
    
    enqueue_sha2();
//     reverse_byte(hash, hash_reverse, 8);
    
    return 0;
}

#endif // TEST_SHA256

#if TEST_SHA3

#include "sha3.h"

static void enqueue_sha3()
{
    // calculate hash
    hwpe_run_sha3(SHA3_MSG_SIZE * SHA3_DATA_SIZE, 
                    B0_DATA_BASE_ADDR,
                    B1_DATA_BASE_ADDR
                    );    // sha3 output is 512 bits or 64 bytes
    count_iter();
}

int stress_sha3()
{
    int msg_size;
    int data_size;
    uint8_t pre_msg[SHA3_BLOCK_SIZE];
    uint8_t hash_reverse[SHA3_BLOCK_SIZE];
    uint32_t *b0 = (uint32_t *) B0_DATA_BASE_ADDR;
    uint32_t *b1 = (uint32_t *) B1_DATA_BASE_ADDR;
    int error = 0;
#if TINY_DATA == 0
    uint32_t flash_addr;
#endif
    char* title;
    
    sprintf(title, "SHA3 512-bit");
    
    print_title(title);
    
    // Loading input data
#if TINY_DATA
    msg_size = sprintf(pre_msg, "Hello");
    data_size = 1;
    reverse_byte(pre_msg, (uint8_t *)b0, ((msg_size - 1)/4 + 1)); // size in words is ceil of msg_size / 4
#else
    printf("Loading data... ");
    msg_size = SHA3_MSG_SIZE;
    data_size = SHA3_DATA_SIZE;
    flash_addr = FL_SHA3_FIRST_DATA_ADDR;
    
    start_time = rt_time_get_us();
    readData(spim, flash_addr, (uint8_t *) B0_DATA_BASE_ADDR, data_size * 64);
    
    printf("%u us", rt_time_get_us() - start_time);
    newline(1);
#endif 
    
    printf("Processing %d data (512-bit inputs)", data_size);
    newline(1);
    
    // calculate hash
    enqueue_sha3();
//     reverse_byte(hash, hash_reverse, 16);
    
    return 0;
}

#endif // TEST_SHA3

void setup()
{
    printf("Starting test");
    newline(2);
    
#if TINY_DATA == 0
    spi_setup();
#endif
    
#if TEST_AES
    aes_data_t aes_data = {AES_DATA_SIZE, KEY_256, CBC_MODE};
    stress_aes(&aes_data);
    while (1) enqueue_aes(&aes_data);
#endif // TEST_AES
    
#if TEST_SHA256
    stress_sha256();
    while (1) enqueue_sha2();
#endif // TEST_SHA256
    
#if TEST_SHA3
    stress_sha3();
    while (1) enqueue_sha3();
#endif // TEST_SHA3

    printf("End of tests");
    newline(1);
  
//     return 0;
}

void loop()
{
  
}
