#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>

#define HSI_FREQ     16000000
#define BAUD         9600

#define DRY_VALUE    908
#define WET_VALUE    472
#define WINDOW_SIZE  5
#define MEDIAN_WINDOW 5

// --- Globals for Soil Moisture ---
static uint32_t moisture_readings[WINDOW_SIZE];
static uint32_t moisture_total = 0;
static uint8_t moisture_idx = 0;

// --- Globals for DHT11 ---
uint8_t temp_buffer[MEDIAN_WINDOW] = {0};
uint8_t temp_index = 0;
uint8_t temperature_median = 0;

uint8_t humidity_buffer[MEDIAN_WINDOW] = {0};
uint8_t humidity_index = 0;
uint8_t humidity_median = 0;

uint8_t DHT11_Data[5];

// --- Globals for Light Sensor ---
#define LIGHT_WINDOW_SIZE 10

// ----------- Rain Sensor Definitions -----------
#define RAIN_RAW_DRY     672
#define RAIN_RAW_WET     326
#define RAIN_FILTER_SIZE 8

uint16_t rainBuf[RAIN_FILTER_SIZE];
uint8_t  rainBufIdx = 0;






// ----------- Delay using TIM2 (µs) -----------
void TIM2_Init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 16 - 1;         // 1 µs tick
    TIM2->ARR = 0xFFFF;
    TIM2->CR1 |= TIM_CR1_CEN;
}

void delay_us(uint32_t us) {
    if (us == 0 || us > 0xFFFF) return;
    TIM2->CNT = 0;
    while (TIM2->CNT < us);
}

void delay_ms(uint32_t ms) {
    while (ms--) delay_us(1000);
}



// ----------- ADC Initialization for PA0, PA4, and PA5 -----------
void adc_init(void) {
    RCC->AHB1ENR  |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR  |= RCC_APB2ENR_ADC1EN;

    // PA0 for soil moisture
    GPIOA->MODER |= (3 << (0 * 2));       // PA0 analog
    GPIOA->PUPDR &= ~(3 << (0 * 2));

    // PA4 for light sensor
    GPIOA->MODER |= (3 << (4 * 2));       // PA4 analog
    GPIOA->PUPDR &= ~(3 << (4 * 2));

    // PA5 for rain sensor
    GPIOA->MODER |= (3 << (5 * 2));       // PA5 analog
    GPIOA->PUPDR &= ~(3 << (5 * 2));

    ADC->CCR = ADC_CCR_ADCPRE_0;          // /4
    ADC1->CR1 = (1 << 24);                // 10-bit resolution
    ADC1->SMPR2 |= (3 << (0 * 3));        // 56 cycles for channel 0
    ADC1->SMPR2 |= (3 << (4 * 3));        // 56 cycles for channel 4
    ADC1->SMPR2 |= (3 << (5 * 3));        // 56 cycles for channel 5
    ADC1->SQR1    &= ~ADC_SQR1_L;         // Regular channel sequence length
    ADC1->SQR3     = 0;                    // First conversion in regular sequence
    ADC1->CR2   = ADC_CR2_ADON;           // Enable ADC

    // Small delay after ADC enable
    delay_us(10);
}




// Read ADC value from specified channel
uint16_t adc_read(uint8_t channel) {
    ADC1->SQR3 = channel;                 // Set the channel to read
    ADC1->CR2 |= ADC_CR2_SWSTART;         // Start conversion
    while (!(ADC1->SR & ADC_SR_EOC));     // Wait for conversion to complete
    uint16_t result = (uint16_t)(ADC1->DR & 0x0FFF); // Read data
    ADC1->SR &= ~ADC_SR_EOC;              // Clear EOC flag
    return result;
}

// ----------- DHT11 Initialization -----------
void DHT11_GPIO_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->PUPDR &= ~(3 << (1 * 2));
}

void DHT11_SetPinOutput(void) {
    GPIOA->MODER &= ~(3 << (1 * 2));
    GPIOA->MODER |=  (1 << (1 * 2));
}

void DHT11_SetPinInput(void) {
    GPIOA->MODER &= ~(3 << (1 * 2));
}

uint8_t DHT11_Read(void) {
    uint8_t i, j;
    uint32_t timeout;

    DHT11_SetPinOutput();
    GPIOA->ODR &= ~(1 << 1);
    delay_us(18000);
    GPIOA->ODR |=  (1 << 1);
    delay_us(30);
    DHT11_SetPinInput();

    // Increased timeout values for better reliability
    timeout = 50000;
    while (GPIOA->IDR & (1 << 1)) if (--timeout == 0) return 1;

    timeout = 50000;
    while (!(GPIOA->IDR & (1 << 1))) if (--timeout == 0) return 2;

    timeout = 50000;
    while (GPIOA->IDR & (1 << 1)) if (--timeout == 0) return 3;

    for (j = 0; j < 5; j++) {
        DHT11_Data[j] = 0;
        for (i = 0; i < 8; i++) {
            timeout = 50000;
            while (!(GPIOA->IDR & (1 << 1))) if (--timeout == 0) return 4;

            delay_us(30);
            if (GPIOA->IDR & (1 << 1)) {
                DHT11_Data[j] |= (1 << (7 - i));
            }

            timeout = 50000;
            while (GPIOA->IDR & (1 << 1)) if (--timeout == 0) return 5;
        }
    }

    uint8_t sum = DHT11_Data[0] + DHT11_Data[1] + DHT11_Data[2] + DHT11_Data[3];
    return (sum == DHT11_Data[4]) ? 0 : 6;
}

// ----------- UART2 Initialization -----------

volatile char received_char;   // To store received data
volatile int data_received = 0; // Flag to indicate data has been received

void USART2_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // PA2 -> TX, PA3 -> RX
    GPIOA->MODER &= ~((3 << (2 * 2)) | (3 << (3 * 2)));
    GPIOA->MODER |=  ((2 << (2 * 2)) | (2 << (3 * 2)));  // Alternate function for PA2 and PA3
    GPIOA->AFR[0] &= ~((0xF << (2 * 4)) | (0xF << (3 * 4)));
    GPIOA->AFR[0] |=  ((7 << (2 * 4)) | (7 << (3 * 4))); // AF7 for USART2

    USART2->BRR = (HSI_FREQ + BAUD/2) / BAUD; // Set baud rate

    USART2->CR1 = USART_CR1_TE | USART_CR1_RE;         // Enable TX and RX
    USART2->CR1 |= USART_CR1_RXNEIE;                   // Enable RX interrupt
    USART2->CR1 |= USART_CR1_UE;                        // Enable USART

    NVIC_EnableIRQ(USART2_IRQn); // Enable USART2 interrupt in NVIC
}

// ----------- Transmit Functions -----------
void USART2_WriteChar(char c) {
    while (!(USART2->SR & USART_SR_TXE)); // Wait until TX buffer is empty
    USART2->DR = c;                       // Send character
    while (!(USART2->SR & USART_SR_TC));  // Wait for transmission complete
}

void USART2_WriteString(const char* s) {
    while (*s) USART2_WriteChar(*s++);
}

// Add this function to flush UART
void USART2_Flush(void) {
    while (!(USART2->SR & USART_SR_TC));
    delay_us(100); // Small delay to ensure transmission
}

// ----------- Interrupt Handler -----------
void USART2_IRQHandler(void) {
    if (USART2->SR & USART_SR_RXNE) {      // Check if RXNE flag is set
        received_char = (char)(USART2->DR); // Read data
        data_received = 1;                 // Set flag
    }
}





// ----------- Moving Average Filter for Soil Moisture -----------
uint16_t Filter_SoilMoisture(uint16_t new_reading) {
    moisture_total -= moisture_readings[moisture_idx];
    moisture_readings[moisture_idx] = new_reading;
    moisture_total += new_reading;
    moisture_idx = (moisture_idx + 1) % WINDOW_SIZE;
    return moisture_total / WINDOW_SIZE;
}

// ----------- Calibration for Soil Moisture -----------
uint8_t Calibrate_Moisture(uint16_t avg_raw) {
    if (avg_raw >= DRY_VALUE) return 0;
    if (avg_raw <= WET_VALUE) return 100;
    return (uint8_t)((uint32_t)(DRY_VALUE - avg_raw) * 100 / (DRY_VALUE - WET_VALUE));
}



// ----------- Generic Median Filter -----------
uint8_t update_median_filter(uint8_t* buffer, uint8_t* index, uint8_t new_val) {
    buffer[*index] = new_val;
    *index = (*index + 1) % MEDIAN_WINDOW;
    uint8_t sorted[MEDIAN_WINDOW];
    for (int i = 0; i < MEDIAN_WINDOW; i++) sorted[i] = buffer[i];
    // Sort the array
    for (int i = 0; i < MEDIAN_WINDOW - 1; i++) {
        for (int j = i + 1; j < MEDIAN_WINDOW; j++) {
            if (sorted[i] > sorted[j]) {
                uint8_t tmp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = tmp;
            }
        }
    }
    // Return the median value
    return sorted[MEDIAN_WINDOW / 2];
}



// ----------- Light Sensor Read and Calibration -----------
int Read_Light_Sensor(void) {
    // Static variables for moving average filter
    static uint32_t light_readings[LIGHT_WINDOW_SIZE] = {0};
    static uint32_t light_total = 0;
    static uint8_t light_readIndex = 0;
    static uint8_t initialized = 0;

    // Light sensor calibration values
    static const uint32_t rawMin = 0;
    static const uint32_t rawMax = 700;
    static const int32_t luxMin = 0;
    static const int32_t luxMax = 6500;

    // Initialize buffer on first call
    if (!initialized) {
        for (int i = 0; i < LIGHT_WINDOW_SIZE; i++) {
            light_readings[i] = 0;
        }
        initialized = 1;
    }

    // Read raw ADC value from channel 4 (PA4)
    uint32_t light_raw = adc_read(4);

    // Remove the oldest value from total
    light_total -= light_readings[light_readIndex];

    // Insert new reading
    light_readings[light_readIndex] = light_raw;

    // Add new value to total
    light_total += light_readings[light_readIndex];

    // Update index (circular buffer style)
    light_readIndex = (light_readIndex + 1) % LIGHT_WINDOW_SIZE;

    // Compute average (integer division)
    int avgLightRaw = light_total / LIGHT_WINDOW_SIZE;

    // Map raw ADC to Lux using integer math
    int32_t lux;
    int32_t rangeIn = rawMax - rawMin;
    int32_t rangeOut = luxMax - luxMin;

    // Prevent division by zero if rawMin == rawMax
    if (rangeIn == 0) {
        lux = luxMin;
    } else {
        lux = ((int64_t)(avgLightRaw - rawMin) * rangeOut) / rangeIn + luxMin;
    }

    // Clamp lux to defined range
    if (lux < luxMin) lux = luxMin;
    if (lux > luxMax) lux = luxMax;

    return (int)lux;
}




// ----------- Rain Sensor Functions -----------
uint16_t Rain_Filter(uint16_t sample) {
    rainBuf[rainBufIdx++] = sample;
    if (rainBufIdx >= RAIN_FILTER_SIZE) rainBufIdx = 0;

    uint32_t sum = 0;
    for (int i = 0; i < RAIN_FILTER_SIZE; i++)
        sum += rainBuf[i];

    return (uint16_t)(sum / RAIN_FILTER_SIZE);
}

uint8_t Rain_Percent(uint16_t filtered_adc) {
    if (filtered_adc > RAIN_RAW_DRY) filtered_adc = RAIN_RAW_DRY;
    if (filtered_adc < RAIN_RAW_WET) filtered_adc = RAIN_RAW_WET;

    uint16_t range = RAIN_RAW_DRY - RAIN_RAW_WET;
    uint16_t value = RAIN_RAW_DRY - filtered_adc;

    return (uint8_t)((value * 100) / range);
}

void WaterPump_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;            // Enable GPIOA
    GPIOA->MODER &= ~(3 << (6 * 2));          // Clear mode
    GPIOA->MODER |=  (1 << (6 * 2));          // Set PA6 as output
}



// ----------- Main Function -----------
int main(void) {
	char buf[64]; // Increased buffer size
    uint16_t raw, avgRaw;
    uint8_t moisture;

    uint16_t adc_raw = 0;
    uint16_t filtered_adc = 0;
    uint8_t rain_percent = 0;
    uint8_t sensor_error_count = 0;
    // Initialize all peripherals
    TIM2_Init();
    adc_init();
    USART2_Init();
    DHT11_GPIO_Init();
    WaterPump_Init();
    // Initialize buffer for rain sensor
    for (int i = 0; i < RAIN_FILTER_SIZE; i++) {
        rainBuf[i] = 0;
    }

    // Initial delay to let sensors stabilize
    delay_ms(3000);

    while (1) {
        // ---- Soil Moisture Read and Filter ----
        raw = adc_read(0); // Read from channel 0 (PA0)
        avgRaw = Filter_SoilMoisture(raw);
        moisture = Calibrate_Moisture(avgRaw);

        // ---- DHT11 Read with error handling ----
        uint8_t status = DHT11_Read();
        if (status == 0) {
            temperature_median = update_median_filter(temp_buffer, &temp_index, DHT11_Data[2]);
            humidity_median = update_median_filter(humidity_buffer, &humidity_index, DHT11_Data[0]);
            sensor_error_count = 0;
        } else {
        	sensor_error_count++;
			if (sensor_error_count < 3) {
				// Retry a few times before giving up
				delay_ms(500);
				continue;
			}
			// Use last known good values if sensor fails repeatedly
			snprintf(buf, sizeof(buf), "DHT11_ERR:%d\r\n", status);
			USART2_WriteString(buf);
			USART2_Flush();

        }

        // ---- Light Sensor ----
        int lux = Read_Light_Sensor();

        // ---- Rain Sensor ----
        adc_raw = adc_read(5);
        filtered_adc = Rain_Filter(adc_raw);
        rain_percent = Rain_Percent(filtered_adc);

        // ---- Send all data over UART ----
        snprintf(buf, sizeof(buf),
                "%d,%d,%d,%d,%d\r\n",
                moisture, temperature_median, humidity_median, lux, rain_percent);
        USART2_WriteString(buf);
        USART2_Flush();


        // --- UART WaterPump control ---
        if (data_received) {
			data_received = 0;

			// Echo back the received character
			USART2_WriteChar(received_char);

			if (received_char == '1') {
				GPIOA->ODR |= (1 << 6);   // Turn pump ON
				USART2_WriteString(" - Pump ON\r\n");
			} else if (received_char == '0') {
				GPIOA->ODR &= ~(1 << 6);  // Turn pump OFF
				USART2_WriteString(" - Pump OFF\r\n");
			} else if (received_char == '?') {
				// Status query
				uint8_t pump_status = (GPIOA->IDR & (1 << 6)) ? 1 : 0;
				snprintf(buf, sizeof(buf), "PUMP_STATUS:%d\r\n", pump_status);
				USART2_WriteString(buf);
			} else if (received_char != '\r' && received_char != '\n') {
				USART2_WriteString(" - Invalid CMD (use 0/1/?)\r\n");
			}
			USART2_Flush();
		}







        delay_ms(2000);
    }
}
