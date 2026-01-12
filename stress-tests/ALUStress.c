#include <stdio.h>
#include <rt/rt_api.h>
#include <stdint.h>

unsigned int __rt_iodev_uart_baudrate = 115200;


/***************************************
**	Test: ALUStress
**	Desc: Operações intensivas com inteiros 
**
***************************************/


#define NUM_ITERATIONS (5000000)  // Número total de operações (ajuste conforme necessário)
// Expected results:
//  - 100:  0x5437C07C
//  - 10000:  0x36BA6A31
//  - 1000000:  0x4410C374

#define LED 23

int main() {
	
	printf("ALUStress: Intensivas operações com inteiros\n\r");
	
	volatile uint32_t a = 0x12345678;
	volatile uint32_t b = 0x9ABCDEF0;
	volatile uint32_t result = 0;
	
	rt_pad_set_function(LED, 1);
	rt_gpio_init(0, LED);
	rt_gpio_set_dir(0, 1<<LED, 1);
	rt_gpio_set_value(0, 1<<LED, 1);
	rt_time_wait_us(1000000*10);
	rt_gpio_set_value(0, 1<<LED, 0);
	printf("Iniciando estresse da ALU...\n\r");

	// Configure PERF counters
	rt_perf_t perf;
	rt_perf_init(&perf);
	rt_perf_conf(&perf, (1<<RT_PERF_ACTIVE_CYCLES) | (1<<RT_PERF_INSTR));
	rt_perf_reset(&perf);
	rt_perf_start(&perf);
	//uint32_t start_active_cycles = rt_perf_read(RT_PERF_ACTIVE_CYCLES);
	//uint32_t start_instr = rt_perf_read(RT_PERF_INSTR);
	
    for (uint32_t i = 0; i < NUM_ITERATIONS; ++i) {
        // Sequência de operações aritméticas e lógicas
        result = a + b;
        result ^= (result << 3);
        result = (result * 13) + (a ^ b);
        result = result - ((a >> 2) | (b << 1));
        result = (result & 0xFFFF0000) | (result ^ 0xAAAAAAAA);
        result = ~result + (a & b);
        result = result ^ ((a << 1) + (b >> 3));
        
        // Modifica a/b para evitar otimização
        a += i ^ result;
        b ^= i + result;
    }

	//uint32_t end_active_cycles = rt_perf_read(RT_PERF_ACTIVE_CYCLES);
	//uint32_t end_instr = rt_perf_read(RT_PERF_INSTR);
	rt_perf_stop(&perf);
	rt_gpio_set_value(0, 1<<LED, 1);
	
	//printf("Total active cycles: %d\n\r", (end_active_cycles - start_active_cycles));
	//printf("Total instructions : %d\n\r", (end_instr - start_instr));
	
    // Impede que o compilador remova os cálculos
    printf("Resultado final: 0x%08X\n\r", result);
    return 0;
}
