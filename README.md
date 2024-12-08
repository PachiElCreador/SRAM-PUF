# SRAM PUF Implementation on STM32 Nucleo F103RB

## Objective
The purpose of this project is to implement and validate an SRAM-based Physically Unclonable Function (PUF) on the STM32 Nucleo F103RB board. The PUF leverages the intrinsic characteristics of SRAM at power-up to generate a unique and reproducible "fingerprint" for the device. This fingerprint is extracted, processed, and transmitted via UART for further analysis.

---

## Introduction
An SRAM PUF exploits the variations in SRAM cells caused by manufacturing differences. When the microcontroller powers up, these cells initialize randomly to either `0` or `1`, creating a unique pattern. This pattern can:
- Serve as a unique identifier for the microcontroller.
- Enable cryptographic key generation and secure device authentication.

This document details the implementation steps, code structure, and validation of the PUF system.

---

## Development

### Hardware Configuration
- **Board:** STM32 Nucleo F103RB.
- **Communication:** UART is configured to transmit the SRAM PUF data to a terminal via USB.
- **SRAM Access:** A `.noinit` section in the memory ensures that the SRAM content is preserved during reset and not initialized by the startup code.

### Software Setup
- **Toolchain:** STM32CubeIDE.
- **Programming Language:** C.
- **Key Memory Configuration:** A custom linker script defines a `.noinit` section where variables are excluded from initialization.

### Code Explanation

#### Memory Configuration (STM32F103RBTX_FLASH.ld)
```c
.noinit (NOLOAD) :
{
    . = ALIGN(4);
    *(.noinit)   /* Captures all the variables in this section */
    . = ALIGN(4);
} >RAM
```
- **Purpose:** Ensures that variables placed in `.noinit` retain the uninitialized state of SRAM upon reset or power-up.
- **Relevance:** This is critical for capturing the randomness of the SRAM needed for the PUF.

#### SRAM Data Capture (main.c)
```c
__attribute__((section(".noinit"))) uint8_t puf_data[16];
```
- **Purpose:** Declares an array of 16 bytes (`128 bits`) stored in the `.noinit` section. This array holds the residual data from the SRAM at power-up.
- **Justification:** The size (`128 bits`) is chosen for compatibility with cryptographic use cases like AES keys.

#### Reading and Transmitting SRAM Data
```c
void read_sram_and_generate_key(void)
{
    char buffer[64];
    for (int i = 0; i < 16; i++) {
        sprintf(buffer, "PUF[%02d]: %02X\r\n", i, puf_data[i]);
        HAL_UART_Transmit(&huart2, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);
    }
}
```
- **Purpose:**
  - Reads the data stored in `puf_data` (residual SRAM values).
  - Transmits each byte in hexadecimal format over UART.
- **Logic:**
  - A loop iterates over the `puf_data` array, formatting each byte as a readable string.
  - `HAL_UART_Transmit` sends the formatted string to the UART terminal for external visualization.

#### UART Configuration
```c
huart2.Init.BaudRate = 9600;
huart2.Init.WordLength = UART_WORDLENGTH_8B;
huart2.Init.StopBits = UART_STOPBITS_1;
huart2.Init.Parity = UART_PARITY_NONE;
huart2.Init.Mode = UART_MODE_TX_RX;
```
- **Purpose:** Sets up UART communication to send the PUF data.
- **Details:**
  - **Baud rate:** 9600 for compatibility with most serial terminals.
  - **Data format:** 8 data bits, no parity, 1 stop bit.

#### Error Handling
```c
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        // Infinite loop to signal an error state
    }
}
```
- **Purpose:** Ensures safe handling of unexpected errors during execution.
- **Rationale:** Disables interrupts and halts the system to prevent further issues.

---

## Results
<img src="https://github.com/user-attachments/assets/45c103aa-5c38-4a21-b02e-a0381f09bb30" alt="image" width="300" />


### Data Captured
1. **First result:** `48 D2 1E 68 F4 B1 04 F0 0F 01 2F 04 D0 F4 D3`
2. **Second result:** `A1 C6 20 A2 CD 4E 51 B4 8D D1 37 B3 AA 2A 81`
3. **Third result:** `E0 C6 20 A2 CD 4E 11 36 8D 51 37 93 AE 2A 81`
4. **Fourth result:** `E1 CE 20 82 4E 4E 11 34 AD 51 27 93 AA 2A 92`

### Observations
- **Within a single reset (without disconnection):** The generated patterns remained constant, indicating consistent behavior of the PUF under identical conditions.
- **Between consecutive resets (with disconnection):** Variations in patterns were observed, as expected. These changes were measured using the Bit Error Rate (BER):

![image](https://github.com/user-attachments/assets/f16a1a19-f6e8-4689-a6fc-0af4b726ffe2)


### Analysis
- **First vs Second Reset:** High BER due to the randomness of SRAM's initial state after power-down.
- **Subsequent Resets:** Stabilized patterns with BER values within acceptable limits (<10%).

---

## Validation

1. **Reproducibility:**
   - Consistent patterns observed on the same device under identical conditions.
   - Acceptable BER observed between power cycles.
2. **Uniqueness:**
   - Pending further testing with multiple boards.
3. **Stability:**
   - Patterns remained stable under normal and slightly varying environmental conditions.

---

## Conclusion
The implemented SRAM PUF on the STM32 Nucleo F103RB successfully demonstrates the unique fingerprint generation and reproducibility necessary for secure applications. Variations in the SRAM data align with expectations, with reproducible results under consistent conditions and acceptable BER under power cycles.

---



