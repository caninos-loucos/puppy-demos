#include <stdio.h>
#include <rt/rt_api.h>
#include <stdint.h>
//#include <math.h>


unsigned int __rt_iodev_uart_baudrate = 115200;

/***************************************
**	Test: FPUStress
**	Desc: Operações intensivas com ponto 
**		  flutuante.
***************************************/


#define NUM_ITERATIONS (100)  // Ajuste para mais ou menos carga
#define NUM_MEGA_ITERATIONS (10000)  // Ajuste para mais ou menos carga

#define LED 13


int main() {
	
	printf("ALUStress: Intensivas operações com ponto flutuante\n\r");

	volatile float a = 1.2345f;
	volatile float b = 6.7890f;
	volatile float c = 3.1415f;
	volatile float result = 0.0f;
	
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
	uint32_t start_active_cycles = rt_perf_read(RT_PERF_ACTIVE_CYCLES);
	uint32_t start_instr = rt_perf_read(RT_PERF_INSTR);

	for (uint32_t j = 0; j < NUM_MEGA_ITERATIONS; ++j) {    
		a = 1.2345f;
		b = 6.7890f;
		c = 3.1415f;
		result = 0.0f;

        	for (uint32_t i = 0; i < NUM_ITERATIONS; ++i) {
			// Operações intensivas de ponto flutuante
			result = a + b + c;
			result = result - (c);
			result = result * a + (b*c);
			result = result / (b + 0.0001f);
			result = result + b;
			result = result + a;
			result = result / (c + 1.0f) + c/a;

			// Muda os operandos dinamicamente
			a += 0.0001f * result;
			b -= 0.0001f * result;
			c += 0.00005f * a;
		}
	}
	
	uint32_t end_active_cycles = rt_perf_read(RT_PERF_ACTIVE_CYCLES);
	uint32_t end_instr = rt_perf_read(RT_PERF_INSTR);
	rt_perf_stop(&perf);
	rt_gpio_set_value(0, 1<<LED, 1);
	
	printf("Total active cycles: %d\n\r", (end_active_cycles - start_active_cycles));
	printf("Total instructions : %d\n\r", (end_instr - start_instr));
	
    // Impede que o compilador remova os cálculos
    printf("Resultado final: %f\n\r", result);
    return 0;
}
