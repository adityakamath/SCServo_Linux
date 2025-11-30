/**
 * @file SyncWriteSpeed.cpp
 * @brief Test sequence for Feetech serial servos using SyncWriteSpe
 *
 * This sequence demonstrates the use of the SyncWriteSpe function to perform a multi-motor test sequence.
 * The test procedure:
 *   - Ramps all motors smoothly from 0 to -2400 (reverse max speed)
 *   - Sweeps input command from -2400 to +2400 in steps, measuring actual speed and detecting min/max input
 *   - Ramps all motors smoothly from +2400 back to 0 (stop)
 *   - Prints test summary: minimum/maximum input command, max measured speed, and effective input range
 *   - Ensures motors are stopped and torque is disabled on exit, error, or user termination
 *
 * This file is intended for hardware testing of Feetech SMS/STS servos using the SCServo library.
 */

#include <iostream>
#include <iomanip>
#include <csignal>
#include <cmath>
#include <unistd.h>
#include <vector>
#include "SCServo.h"
#include "INST.h"

SMS_STS sm_st;

/**
 * @brief Safely stops all motors and disables torque.
 */
void safeShutdown() {
    // Stop all motors and disable torque
    u8 ids[3] = {7, 8, 9};
    s16 Speed_array[3] = {0, 0, 0};
    u8 Acc_array[3] = {254, 254, 254};
    sm_st.SyncWriteSpe(ids, 3, Speed_array, Acc_array);
    usleep(500000);
    for(int i=0; i<3; i++) {
        sm_st.EnableTorque(ids[i], 0);
    }
    sm_st.end();
    std::cout << "Motors stopped and torque disabled." << std::endl;
}

u8 ID[3] = {7, 8, 9};
int speed_offsets[3] = {0, 0, 0}; // Calibrated offsets for each motor
int min_speeds[3] = {0, 0, 0}; // Minimum measurable speeds
int max_speeds[3] = {0, 0, 0}; // Maximum speeds

/**
 * @brief Handles SIGINT signal for safe shutdown.
 * @param signum Signal number
 */
void signalHandler(int signum) {
    if (signum == SIGINT) {
        safeShutdown();
        std::cout<<"Terminated by user (SIGINT)"<<std::endl;
        exit(0);
    }
}

// Measure actual speed by reading Present_Speed register
/**
 * @brief Measures actual speed by reading Present_Speed register.
 * @param motor_id Servo motor ID
 * @param samples Number of samples to average
 * @return Averaged measured speed
 */
int measureSpeed(u8 motor_id, int samples=5) {
    int total = 0;
    for(int i=0; i<samples; i++) {
        sm_st.FeedBack(motor_id);
        int speed = sm_st.ReadSpeed(-1); // Read from cached buffer
        total += speed;
        usleep(50000); // 50ms between samples
    }
    return total / samples;
}

// Calibrate offset for a motor by testing both directions
// Also determines minimum and maximum speeds
// Buffer ranges for future use
const int NEGATIVE_BUFFER_START = -2400;
const int NEGATIVE_BUFFER_END = -2600;
const int POSITIVE_BUFFER_START = 2400;
const int POSITIVE_BUFFER_END = 2600;

/**
 * @brief Smoothly ramps all motors to a target speed value.
 * @param ids Array of motor IDs
 * @param num_motors Number of motors
 * @param target Target speed value
 * @param step Step size for ramping
 * @param delay_us Delay in microseconds between steps
 */
void smoothRampToValue(u8* ids, int num_motors, int target, int step = 200, int delay_us = 100000) {
    int current = 0;
    s16 Speed_array[3] = {0, 0, 0};
    u8 Acc_array[3] = {254, 254, 254};
    if (target < 0) {
        for(; current >= target; current -= step) {
            for(int i=0; i<num_motors; i++) Speed_array[i] = current;
            sm_st.SyncWriteSpe(ids, num_motors, Speed_array, Acc_array);
            usleep(delay_us);
        }
    } else {
        for(; current <= target; current += step) {
            for(int i=0; i<num_motors; i++) Speed_array[i] = current;
            sm_st.SyncWriteSpe(ids, num_motors, Speed_array, Acc_array);
            usleep(delay_us);
        }
    }
    for(int i=0; i<num_motors; i++) Speed_array[i] = target;
    sm_st.SyncWriteSpe(ids, num_motors, Speed_array, Acc_array);
    usleep(200000);
}

void smoothStopMotors(u8* ids, int num_motors) {
    // Gradually bring motors to zero speed from +2400
    smoothRampToValue(ids, num_motors, 0, 200, 100000);
}

void testAllMotors(int test_speed=975) {
    // Sweep from -2400 to +2400 (safe limit, buffer ranges: -2600 to -2400 and 2400 to 2600)
    const int min_cmd = -2400;
    const int max_cmd = 2400;
    const int step = 100;
    const int sweep_delay_us = 62500; // 62.5ms

    std::vector<int> measured_speeds[3];

    
    std::cout << "== Ramping down 0 to " << min_cmd << " ==" << std::endl;
    // Ramp from 0 to -2400
    smoothRampToValue(ID, 3, -2400, 200, 250000);
        
    // Sweep from -2400 to +2400
    std::cout << "== Start sweep: " << min_cmd << " to " << max_cmd << " ==\n" << std::endl;
    for(int cmd = min_cmd; cmd <= max_cmd; cmd += step) {
        s16 Speed_array[3] = {cmd, cmd, cmd};
        u8 Acc_array[3] = {254, 254, 254};
        sm_st.SyncWriteSpe(ID, 3, Speed_array, Acc_array);
        usleep(500000);
        int speeds[3];
        for(int i=0; i<3; i++) {
            sm_st.FeedBack(ID[i]);
            speeds[i] = sm_st.ReadSpeed(ID[i]);
            measured_speeds[i].push_back(speeds[i]);
        }
        std::cout << "\rCMD: " << std::setw(5) << cmd
                  << " | M7: " << std::setw(5) << speeds[0]
                  << " | M8: " << std::setw(5) << speeds[1]
                  << " | M9: " << std::setw(5) << speeds[2] << std::flush;
    }
    std::cout << std::endl << "\n== Sweep complete ==" << std::endl;

    // Ramp down from +2400 to 0
    std::cout << "== Ramping down " << max_cmd << " to 0 ==\n" << std::endl;
    smoothRampToValue(ID, 3, 0, 200, 250000);

    // --- Print summary for all motors ---
    std::cout << "\n=== SUMMARY ===" << std::endl;
    std::cout << "Input command range: " << min_cmd << " to " << max_cmd << std::endl;
    for(int i=0; i<3; i++) {
        if (measured_speeds[i].empty()) {
            std::cout << "Motor ID " << (int)ID[i] << ": No measured speeds." << std::endl;
            continue;
        }
        int min_measured = measured_speeds[i][0];
        int max_measured_val = measured_speeds[i][0];
        for (size_t j = 1; j < measured_speeds[i].size(); ++j) {
            if (measured_speeds[i][j] < min_measured) min_measured = measured_speeds[i][j];
            if (measured_speeds[i][j] > max_measured_val) max_measured_val = measured_speeds[i][j];
        }
        int midpoint_offset = (max_measured_val + min_measured) / 2;
        std::cout << "\nMotor ID " << (int)ID[i] << ":" << std::endl;
        std::cout << "  Measured speed range: " << min_measured << " to " << max_measured_val << std::endl;
        std::cout << "  Midpoint offset: " << midpoint_offset << std::endl;
    }
    s16 Speed_array2[3] = {0, 0, 0};
    u8 Acc_array2[3] = {254, 254, 254};
    sm_st.SyncWriteSpe(ID, 3, Speed_array2, Acc_array2);
    sleep(1);
}

int main(int argc, char **argv)
{
    if(argc<2){
        std::cout<<"argc error! Usage: ./SyncWriteSpeed /dev/ttySERVO"<<std::endl;
        return 0;
	}
    
	std::cout<<"serial:"<<argv[1]<<std::endl;
    if(!sm_st.begin(1000000, argv[1])){ //115200 for sms, 1000000 for sts
        std::cout<<"Failed to init sms/sts motor!"<<std::endl;
        return 0;
    }

    signal(SIGINT, signalHandler);
    
    // Explicitly set registers for robust initialization
    std::cout << "Initializing motors..." << std::endl;
    for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
        int mode_ret = sm_st.writeByte(ID[i], SMS_STS_MODE, 1); // Set velocity mode
        std::cout << "Set Operating_Mode=1 (velocity) for motor " << (int)ID[i] << " (ret=" << mode_ret << ")" << std::endl;
        usleep(100000); // Wait 100ms for mode change
        int acc_ret = sm_st.writeByte(ID[i], SMS_STS_ACC, 254); // Set acceleration to max
        std::cout << "Set Acceleration=254 for motor " << (int)ID[i] << " (ret=" << acc_ret << ")" << std::endl;
        usleep(100000); // Wait 100ms for acceleration
        int torque_ret = sm_st.EnableTorque(ID[i], 1);
        std::cout << "Enable Torque for motor " << (int)ID[i] << " (ret=" << torque_ret << ")" << std::endl;
        usleep(100000); // Wait 100ms for torque enable
    }
    usleep(500000); // Wait 0.5s for all motors to process changes
    
    // Automatically test all motors together
    std::cout << "\n=== TEST ===" << std::endl;
    try {
        testAllMotors(975);
        std::cout << "\nStopping all motors..." << std::endl;
        safeShutdown();
        std::cout << "Exiting..." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        safeShutdown();
        return 1;
    } catch (...) {
        std::cerr << "Unknown error occurred." << std::endl;
        safeShutdown();
        return 2;
    }
}

