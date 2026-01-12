
#include <stdio.h>
#include <rt/rt_api.h>
#include <stdint.h>
#include <stdlib.h>

unsigned int __rt_iodev_uart_baudrate = 115200;

/***************************************
**	Test: RAMStress
**	Desc: Acesso aleatório e intensivo 
**        de leitura/escrita na RAM
**
***************************************/


#define RAM_BASE_ADDRESS  ((volatile uint32_t*) 0x1C010000)  // Altere conforme o mapeamento da RAM
#define RAM_SIZE_BYTES	   (128*1024) // 128 KB
#define NUM_ACCESSES      (100000000)           // Número total de acessos
#define STRIDE            (4)                 // Acesso palavra por palavra (32 bits)

#define LED 13

int main() {
	
	
	printf("RAMStress: acesso aleatório e intensivo à RAM\n\r");
	printf("\tRAM base address: 0x%x\n\r", RAM_BASE_ADDRESS);
	printf("\tRAM size bytes: 0x%x\n\r", RAM_SIZE_BYTES);
	printf("\tNumero de acessos: 0x%x\n\r", NUM_ACCESSES);
	
	// RAM related stuff
	volatile uint32_t* ram = RAM_BASE_ADDRESS;
	size_t ram_words = RAM_SIZE_BYTES / STRIDE;
	
	rt_pad_set_function(LED, 1);
	rt_gpio_init(0, LED);
	rt_gpio_set_dir(0, 1<<LED, 1);
	rt_gpio_set_value(0, 1<<LED, 1);
	rt_time_wait_us(1000000*10);
	rt_gpio_set_value(0, 1<<LED, 0);
	
	printf("Iniciando acessos aleatórios à RAM...\n\r");
	
	// Configure PERF counters
	rt_perf_t perf;
	rt_perf_init(&perf);
	rt_perf_conf(&perf, (1<<RT_PERF_ACTIVE_CYCLES) | (1<<RT_PERF_INSTR));
	rt_perf_reset(&perf);
	rt_perf_start(&perf);
	uint32_t start_active_cycles = rt_perf_read(RT_PERF_ACTIVE_CYCLES);
	uint32_t start_instr = rt_perf_read(RT_PERF_INSTR);
	
    for (size_t i = 0; i < NUM_ACCESSES; i++) {
        // Gera um índice aleatório dentro da RAM
        //size_t index = rand() % ram_words;
	size_t index = pulp_read32(0x1A000000 + (i%0x2000) ) % ram_words;

        // Alterna entre escrita e leitura a cada iteração
        if (i % 2 == 0) {
            // Escrita: escreve um valor pseudoaleatório
            uint32_t value = (uint32_t)(0x10792160 ^ i);
            ram[index] = value;
        } else {
            // Leitura: lê o valor e faz algo com ele (ex: XOR para evitar otimização)
            volatile uint32_t value = ram[index];
            asm volatile("" ::: "memory"); // Previne otimizações agressivas
        }
    }

	uint32_t end_active_cycles = rt_perf_read(RT_PERF_ACTIVE_CYCLES);
	uint32_t end_instr = rt_perf_read(RT_PERF_INSTR);
	rt_perf_stop(&perf);
	rt_gpio_set_value(0, 1<<LED, 1);
	
    	printf("Acessos finalizados.\n\r");
	printf("Total active cycles: %d\n\r", (end_active_cycles - start_active_cycles));
	printf("Total instructions : %d\n\r", (end_instr - start_instr));
    return 0;
}
