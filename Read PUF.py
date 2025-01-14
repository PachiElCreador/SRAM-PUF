import serial
import csv
import time

def read_and_save_serial_data(port, baudrate, filename, board_id=1, reset_type='p'):
    """
    Continuously reads data from the serial port and saves each PUF sequence to a specific column in a CSV file.
    The column is determined by the board_id and reset_type.
    """
    # Validate reset_type
    if reset_type not in ['p', 's']:
        raise ValueError("reset_type must be 'p' or 's'")

    # Calculate column and header based on board_id and reset_type
    column = (board_id * 2) - 1 if reset_type == 'p' else board_id * 2
    header = f"b{board_id} {'power-on' if reset_type == 'p' else 'soft-reset'}"
    print(f"Calculated column: {column} (board_id={board_id}, reset_type={reset_type})")

    current_puf = []  # Temporary storage for the current sequence

    while True:
        try:
            # Attempt to open the serial port
            with serial.Serial(port, baudrate, timeout=1) as ser, open(filename, mode='a', newline='') as file:
                print("Listening for data... Press Ctrl+C to stop.")
                writer = csv.writer(file)

                while True:
                    try:
                        line = ser.readline().decode("utf-8", errors="ignore").strip()
                        if line.startswith("PUF"):
                            # Extract the hex value
                            value = line.split(": ")[1]
                            current_puf.append(value)

                            # Check if we have a full sequence
                            if len(current_puf) == 16:
                                puf_sequence = "".join(current_puf)  # Combine into a single string
                                print(f"Captured PUF Sequence: {puf_sequence}")

                                # Write the sequence to the specific column in the CSV file
                                row = ["" for _ in range(column + 1)]
                                row[column] = puf_sequence
                                writer.writerow(row)

                                # Reset the current sequence for the next set
                                current_puf = []
                    except serial.SerialException as se:
                        print(f"Serial error: {se}")
                        break  # Exit the inner loop to reconnect
                    except KeyboardInterrupt:
                        print("\nStopping data capture...")
                        return  # Exit the entire function
                    except Exception as e:
                        print(f"Error: {e}")

        except serial.SerialException as e:
            print(f"Could not connect to {port}. Retrying in 5 seconds...")
            time.sleep(5)  # Wait before retrying to reconnect
        except KeyboardInterrupt:
            print("\nExiting program...")
            break

if __name__ == "__main__":
    # Serial port configuration
    SERIAL_PORT = "COM15"  # Adjust to the serial port
    BAUD_RATE = 9600  # baud rate set during stm configuration
    FILENAME = "data_sram_puf.csv"

    # Board configuration
    BOARD_ID = 3  # Configure the board ID (1, 2, 3, ...)
    RESET_TYPE = 's'  # Configure the reset type ('p' for power-on or 's' for soft reset)

    try:
        read_and_save_serial_data(SERIAL_PORT, BAUD_RATE, FILENAME, BOARD_ID, RESET_TYPE)
    except Exception as e:
        print(f"Error: {e}")
