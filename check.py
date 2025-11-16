import serial
import time
import sys

# --- CONFIGURATION ---
# --- MUST BE UPDATED ---
PORT_DUT    = "COM6"  # COM Port for your Main Arduino (Device Under Test)
PORT_TESTER = "COM7"  # COM Port for your new Tester Arduino
BAUD        = 9600
# ---------------------

def run_test_suite():
    """
    Main function to open BOTH ports and run all test cases.
    """
    ser_dut = None
    ser_tester = None
    test_results = {}
    
    try:
        # --- 1. Open Serial Connections ---
        print(f"Attempting to open DUT port {PORT_DUT}...")
        ser_dut = serial.Serial(PORT_DUT, BAUD, timeout=1)
        print(f"*** SUCCESS: DUT Port {PORT_DUT} opened. ***")
        
        print(f"Attempting to open TESTER port {PORT_TESTER}...")
        ser_tester = serial.Serial(PORT_TESTER, BAUD, timeout=1)
        print(f"*** SUCCESS: TESTER Port {PORT_TESTER} opened. ***")
        
        # Wait for both Arduinos to reset
        print("Waiting for devices to boot (5s)...")
        time.sleep(5) 
        
        # Clear any boot-up messages
        dut_junk = ser_dut.read_all().decode('utf-8')
        tester_junk = ser_tester.read_all().decode('utf-8')
        print(f"Tester boot: {tester_junk.strip()}")
        print("Serial buffers cleared. Starting tests...")
        
        # --- 2. Run Test Cases ---
        try:
            tc1_result = tc1_led_ldr_check(ser_dut, ser_tester)
            test_results["TC1"] = tc1_result
        except Exception as e:
            test_results["TC1"] = (False, f"Test crashed: {e}")
            
        try:
            tc2_result = tc2_set_high_limit(ser_dut, ser_tester)
            test_results["TC2"] = tc2_result
        except Exception as e:
            test_results["TC2"] = (False, f"Test crashed: {e}")

        try:
            tc3_result = tc3_set_low_limit(ser_dut, ser_tester)
            test_results["TC3"] = tc3_result
        except Exception as e:
            test_results["TC3"] = (False, f"Test crashed: {e}")

    except serial.SerialException as e:
        print(f"\n*** FATAL ERROR: Could not open port. ***")
        print(f"Error: {e}")
        return False

    finally:
        # --- 3. Close Connections & Print Summary ---
        if ser_dut and ser_dut.is_open:
            ser_dut.close()
            print(f"\nPort {PORT_DUT} (DUT) closed.")
        if ser_tester and ser_tester.is_open:
            ser_tester.close()
            print(f"Port {PORT_TESTER} (Tester) closed.")
            
        print_summary(test_results)

# --- Test Case Definitions ---

def tc1_led_ldr_check(ser_dut, ser_tester):
    """
    TC1: Press LED button, sets brightness to 0.
         Press again, sets brightness to 99.
         Verifies the LDR reading increases.
    """
    print("\n" + "-"*40)
    print("--- Running TC1: Control LED Brightness ---")
    
    # --- Set brightness to 0 ---
    print("Pressing LED Button (for 0%)")
    press_button(ser_tester, ser_dut, 'B', "LED Control: Pressed")
    print("Sending value 0...")
    send_value_and_confirm(ser_dut, 0, "-> Updated Brightness")
    
    status_line_0 = query_status(ser_dut)
    ldr_at_0 = parse_status(status_line_0, "LDR")
    print(f"LDR value at 0% brightness: {ldr_at_0}")

    # --- Set brightness to 99 ---
    print("\nPressing LED Button (for 99%)")
    press_button(ser_tester, ser_dut, 'B', "LED Control: Pressed")
    print("Sending value 99...")
    send_value_and_confirm(ser_dut, 99, "-> Updated Brightness")

    status_line_99 = query_status(ser_dut)
    ldr_at_99 = parse_status(status_line_99, "LDR")
    print(f"LDR value at 99% brightness: {ldr_at_99}")

    # --- Verify ---
    assert ldr_at_99 > ldr_at_0, f"LDR at 99% ({ldr_at_99}) was NOT greater than LDR at 0% ({ldr_at_0})"
    
    return (True, f"PASSED: LDR {ldr_at_0} -> {ldr_at_99}")

def tc2_set_high_limit(ser_dut, ser_tester):
    """
    TC2: Press HL button.
         Sends value 50 and verifies.
    """
    print("\n" + "-"*40)
    print("--- Running TC2: Set High Limit ---")
    
    print("Pressing High Limit Button...")
    press_button(ser_tester, ser_dut, 'H', "High Limit: Pressed")
    print("Sending value 50...")
    send_value_and_confirm(ser_dut, 50, "-> Updated High Limit")

    status_line = query_status(ser_dut)
    hl_value = parse_status(status_line, "HL")
    
    print(f"Current High Limit (HL) from device: {hl_value}")
    assert hl_value == 50, f"High Limit was {hl_value}, expected 50"
    
    return (True, "PASSED: High Limit set to 50")

def tc3_set_low_limit(ser_dut, ser_tester):
    """
    TC3: Press LL button.
         Sends value 20 and verifies.
    """
    print("\n" + "-"*40)
    print("--- Running TC3: Set Low Limit ---")
    
    print("Pressing Low Limit Button...")
    press_button(ser_tester, ser_dut, 'L', "Low Limit: Pressed")
    print("Sending value 20...")
    send_value_and_confirm(ser_dut, 20, "-> Updated Low Limit")

    status_line = query_status(ser_dut)
    ll_value = parse_status(status_line, "LL")
    
    print(f"Current Low Limit (LL) from device: {ll_value}")
    assert ll_value == 20, f"Low Limit was {ll_value}, expected 20"

    return (True, "PASSED: Low Limit set to 20")

# --- Helper Functions (Do Not Modify) ---

def press_button(ser_tester, ser_dut, button_char, dut_echo):
    """Tells Tester to press, and waits for DUT to confirm."""
    print(f"  [TX->TESTER] -> {button_char}")
    ser_tester.write(button_char.encode('utf-8'))
    # Wait for the DUT to send its confirmation message
    read_until_expected(ser_dut, dut_echo, 5.0, "DUT")
    print("  Button press confirmed by DUT.")
    # Read the echo from the tester to clear its buffer
    tester_echo = ser_tester.read_until(b'\n').decode('utf-8').strip()
    print(f"  [RX<-TESTER] <- {tester_echo}")

def send_value_and_confirm(ser_dut, value, value_echo):
    """Sends a value + newline to DUT and confirms receipt."""
    value_str = f"{value}\n"
    print(f"  [TX->DUT] -> {value_str.strip()}")
    ser_dut.write(value_str.encode('utf-8'))
    
    read_until_expected(ser_dut, f"Received: {value}", 5.0, "DUT")
    read_until_expected(ser_dut, value_echo, 5.0, "DUT")
    print("  Value set successfully.")

def query_status(ser_dut):
    """Sends '?' to DUT and reads the 'STATUS:...' line."""
    print("  [TX->DUT] -> ?")
    ser_dut.write(b'?')
    status_line = read_until_expected(ser_dut, "STATUS:", 5.0, "DUT")
    return status_line

def read_until_expected(ser, expected_string, timeout, name="Serial"):
    """Reads lines from serial until a line contains the expected string."""
    start_time = time.time()
    
    while (time.time() - start_time) < timeout:
        line = ser.readline().decode('utf-8').strip()
        if line:
            print(f"  [RX<-{name}] <- {line}")
            if expected_string in line:
                return line # Success, return the line
    
    raise Exception(f"TIMEOUT: Device '{name}' response '{expected_string}' not found.")

def parse_status(status_line, key):
    """Parses a 'STATUS:...' line for a specific key (e.g., 'LDR')."""
    try:
        key_to_find = f"{key}="
        parts = status_line.split(' ')
        for part in parts:
            if part.startswith(key_to_find):
                value = part.split('=')[1]
                return int(value)
    except Exception as e:
        raise Exception(f"Could not parse key '{key}' from line '{status_line}'. Error: {e}")

def print_summary(results):
    """Prints a final summary of all test cases."""
    print("\n" + "="*50)
    print("         AUTOMATED TEST SUMMARY")
    print("="*50)
    
    passed = 0
    for test_name, (success, message) in results.items():
        if success:
            print(f" [PASS] {test_name}: {message}")
            passed += 1
        else:
            print(f" [FAIL] {test_name}: {message}")
            
    print("-"*50)
    print(f"Total: {len(results)} tests | Passed: {passed} | Failed: {len(results) - passed}")
    print("="*50)
    
# --- Run the Script ---
if __name__ == "__main__":
    run_test_suite()