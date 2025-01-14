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

## First results
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

## Final Validation

A total of 20 power-on resets and 20 soft resets were performed on the board. The data was collected using the **"Read PUF.py"** script. This process was repeated on three different boards.

### Results and Validation

Data was obtained from the **"data_sram_puf.csv"** file generated, where every two columns correspond to different PUF values. The first column in each pair represents the data after the power-on reset, and the second column represents the data after the soft reset. This structure is repeated for each of the three boards:

- Columns 1 and 2: data from the first board.
- Columns 3 and 4: data from the second board.
- Columns 5 and 6: data from the third board.

After processing the data using the **"Validation metrics calculation.py"** script, the following metrics were calculated.

![image](https://github.com/user-attachments/assets/5a511993-9dea-4ef9-b718-d87cff847a74)


## PUF Metrics (Evaluation criteria) explanation

### 1. **Uniformity**
**Comparison:** Within the same board.

#### **What does it measure?**
Uniformity measures whether the bits generated by the PUF are balanced between `0`s and `1`s. Ideally, there should be an equal number of `0`s and `1`s in the pattern.

#### **Calculation:**
```math
Uniformity = \left(\frac{\text{Number of 1s}}{\text{Total number of bits}}\right) \times 100
```
- **Ideal Value:** **50%** (indicating a perfect balance between `0`s and `1`s).

#### **Example:**
If a PUF generates a 128-bit pattern and 64 bits are `1`, then:
```math
Uniformity = \frac{64}{128} \times 100 = 50\%
```

---

### 2. **Unicity (Inter Hamming Distance)**
**Comparison:** Between different boards.

#### **What does it measure?**
Unicity measures how different the patterns generated by different boards are under identical conditions. It evaluates the **uniqueness** of each board.

#### **Calculation:**
1. Take patterns generated by two different boards under identical conditions (e.g., power-on reset).
2. Calculate the **Hamming Distance** between the patterns.
3. Repeat for all board combinations.
4. Average the Hamming distances.

**Formula:**
```math
Inter HD = \frac{\text{Sum of Hamming distances between boards}}{\text{Total number of board combinations}}
```
- **Ideal Value:** **50%** (indicating that, on average, half the bits differ between boards).

#### **Example:**
- Pattern from Board 1: `10101010`
- Pattern from Board 2: `11001100`
- Hamming Distance: 4 bits differ.

This difference should ideally remain consistent across board comparisons.

---

### 3. **Diffusion (Avalanche Effect)**
**Comparison:** Within the same board.

#### **What does it measure?**
Diffusion measures how much the output pattern changes when there is a small change in the input conditions. For example, a slight variation in temperature or input should cause significant changes in the output.

#### **Calculation:**
1. Generate two patterns for the same board under slightly different conditions.
2. Calculate the **Hamming Distance** between the two patterns.
3. Average the distances over multiple tests.

- **Ideal Value:** **50%** (indicating that half the bits change due to small input variations).

#### **Example:**
- Pattern 1: `10101010`
- Pattern 2: `01010101`
- Hamming Distance: 8 bits change (all bits).

This indicates strong diffusion.

---

### 4. **Robustness (Intra Hamming Distance)**
**Comparison:** Within the same board.

#### **What does it measure?**
Robustness measures the stability of the PUF when generating patterns under identical conditions. It ensures that the same board produces consistent patterns over time.

#### **Calculation:**
1. Generate multiple patterns for the same board under identical conditions (e.g., multiple power-on resets).
2. Calculate the **Hamming Distance** between each pair of patterns.
3. Average the distances.

**Formula:**
```math
Intra HD = \frac{\text{Sum of Hamming distances between patterns on the same board}}{\text{Total number of pattern combinations}}
```
- **Ideal Value:** **0%** (indicating no differences between patterns generated under the same conditions).

#### **Example:**
- Pattern 1: `10101010`
- Pattern 2: `10101010`
- Hamming Distance: 0 (identical patterns).

This indicates excellent robustness.

---

### **Table of Comparisons and Ideal Values**

| **Metric**          | **Comparison**             | **What it Measures**                           | **Ideal Value** |
|----------------------|---------------------------|------------------------------------------------|-----------------|
| **Uniformity**       | Within the same board      | Balance of `0`s and `1`s in a pattern          | 50%             |
| **Unicity (Inter HD)** | Between different boards   | Differences between patterns from different boards | 50%             |
| **Diffusion**        | Within the same board      | Changes in output due to small input variations | 50%             |
| **Robustness (Intra HD)** | Within the same board      | Stability of patterns under identical conditions | 0%              |


## Results Validation Interpretation

1. **Uniformity:**
   - The uniformity result is **49.13%**, which is very close to the ideal value of 50%. This indicates that the distribution of '1's and '0's in the PUF responses is well-balanced, a desirable property for randomness in PUFs.

2. **Unicity:**
   - The inter-device Hamming distance (Unicity) has a mean value of **48.84%**, which is very close to the expected theoretical value of 50%. This means that the PUF responses from different boards are distinct enough to ensure device-specific uniqueness, confirming the effectiveness of the PUF in distinguishing between devices.

3. **Diffusion:**
   - The diffusion mean is **9.90%**, which is lower than the expected value of ~50%. This suggests that the PUF responses do not exhibit a strong avalanche effect between power-on and soft reset states. This might indicate room for improvement in the design to ensure higher sensitivity to input changes.

4. **Robustness:**
   - The intra-device Hamming distance (Robustness) has a mean value of **0.00%**, which is ideal. This demonstrates that the PUF responses are highly stable and consistent within the same device across multiple resets. Stability is crucial for reproducibility in PUF applications.

---

## Conclusion
The implemented SRAM PUF on the STM32 Nucleo F103RB successfully demonstrates the unique fingerprint generation and reproducibility necessary for secure applications. Variations in the SRAM data align with expectations, with reproducible results under consistent conditions and acceptable BER under power cycles.

---



