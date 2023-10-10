import math
import random
import time

import matplotlib.pyplot as plt
from simple_pid import PID


class Tank:
    def __init__(self, starting_pressure):
        self.pressure = starting_pressure

    def update(self, input_val, output_val):
        self.pressure += input_val - output_val
        return self.pressure


if __name__ == "__main__":
    run_time = 10000  # ms
    tank = Tank(0)

    pid = PID(1, 0.1, 0.05, setpoint=0)

    # while True:
    #     control = pid(tank.pressure)
    #     pid.setpoint=100
    #     # Feed the PID output to the system and get its current value
    #     v = tank.update(control, 0)
    #     print(tank.pressure)
    #     time.sleep(.1)

    pressures = []
    targets = []

    for i in range(run_time):
        pid.setpoint = int(350 * math.sin(math.pi * i / (run_time - 1)))

        control_output = pid(tank.pressure)
        v = tank.update(control_output, 0)

        # Logging & graphing
        pressures.append(tank.pressure)
        targets.append(pid.setpoint)

        if i % 1000 == 0:
            print("Setpoint: " + str(pid.setpoint))
            print("Pressure: " + str(tank.pressure))
            print("Control Signal: " + str(control_output))

    plt.plot(pressures, label='Tank Pressure')
    plt.plot(targets, '--', label='Target Pressure')
    plt.xlabel("Time (ms)")
    plt.ylabel("Pressure (psi)")
    plt.title("Tank Pressure over Time using PID Control")
    plt.legend()
    plt.show()
