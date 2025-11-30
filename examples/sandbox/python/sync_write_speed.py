import signal
import sys
import time
from scservo_python import SMS_STS

ID = [7, 8, 9]
SERIAL_PORT = "/dev/ttySERVO"
BAUD_RATE = 1000000
sm_st = SMS_STS()

def safe_shutdown():
    speeds = [0] * len(ID)
    accs = [254] * len(ID)
    sm_st.SyncWriteSpe(ID, speeds, accs)
    time.sleep(0.5)
    for motor_id in ID:
        sm_st.EnableTorque(motor_id, 0)
    print("Motors stopped and torque disabled.")

def signal_handler(sig, frame):
    safe_shutdown()
    print("Terminated by user (SIGINT)")
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

def smooth_ramp_to_value(ids, target, step=200, delay_s=0.1):
    current = 0
    accs = [254] * len(ids)
    if target < 0:
        while current >= target:
            speeds = [current] * len(ids)
            sm_st.SyncWriteSpe(ids, speeds, accs)
            time.sleep(delay_s)
            current -= step
    else:
        while current <= target:
            speeds = [current] * len(ids)
            sm_st.SyncWriteSpe(ids, speeds, accs)
            time.sleep(delay_s)
            current += step
    speeds = [target] * len(ids)
    sm_st.SyncWriteSpe(ids, speeds, accs)
    time.sleep(0.2)

def sync_write_speed_test():
    print("Initializing motors...")
    if not sm_st.begin(BAUD_RATE, SERIAL_PORT):
        print("Failed to initialize serial bus!")
        return
    for motor_id in ID:
        mode_ret = sm_st.writeByte(motor_id, 33, 1) # Set velocity mode
        print(f"Set Operating_Mode=1 (velocity) for motor {motor_id} (ret={mode_ret})")
        time.sleep(0.1)
        acc_ret = sm_st.writeByte(motor_id, 41, 254) # Set acceleration
        print(f"Set Acceleration=254 for motor {motor_id} (ret={acc_ret})")
        time.sleep(0.1)
        torque_ret = sm_st.EnableTorque(motor_id, 1)
        print(f"Enable Torque for motor {motor_id} (ret={torque_ret})")
        time.sleep(0.1)
    time.sleep(0.5)

    min_cmd = -2400
    max_cmd = 2400
    step = 100
    sweep_delay_s = 0.625

    measured_speeds = [[] for _ in ID]

    print("\n=== TEST ===")
    print(f"== Ramping down 0 to {min_cmd} ==")
    smooth_ramp_to_value(ID, min_cmd, 200, 0.25)

    print(f"== Start sweep: {min_cmd} to {max_cmd} ==\n")
    for cmd in range(min_cmd, max_cmd + 1, step):
        speeds = [cmd] * len(ID)
        accs = [254] * len(ID)
        sm_st.SyncWriteSpe(ID, speeds, accs)
        time.sleep(0.5)
        actual_speeds = []
        for i, motor_id in enumerate(ID):
            sm_st.FeedBack(motor_id)
            speed = sm_st.ReadSpeed(motor_id)
            measured_speeds[i].append(speed)
            actual_speeds.append(speed)
        print(f"\rCMD: {cmd:5d} | M7: {actual_speeds[0]:5d} | M8: {actual_speeds[1]:5d} | M9: {actual_speeds[2]:5d}", end='', flush=True)
    print("\n\n== Sweep complete ==")

    print(f"== Ramping down {max_cmd} to 0 ==\n")
    smooth_ramp_to_value(ID, 0, 200, 0.25)

    print("\n=== SUMMARY ===")
    print(f"Input command range: {min_cmd} to {max_cmd}")
    for i, motor_id in enumerate(ID):
        if not measured_speeds[i]:
            print(f"Motor ID {motor_id}: No measured speeds.")
            continue
        min_measured = min(measured_speeds[i])
        max_measured_val = max(measured_speeds[i])
        midpoint_offset = (max_measured_val + min_measured) // 2
        print(f"\nMotor ID {motor_id}:")
        print(f"  Measured speed range: {min_measured} to {max_measured_val}")
        print(f"  Midpoint offset: {midpoint_offset}")
    sm_st.SyncWriteSpe(ID, [0]*len(ID), [254]*len(ID))
    time.sleep(1)

if __name__ == "__main__":
    try:
        sync_write_speed_test()
        print("\nStopping all motors...")
        safe_shutdown()
        print("Exiting...")
    except Exception as e:
        print(f"ERROR: {e}")
        safe_shutdown()
