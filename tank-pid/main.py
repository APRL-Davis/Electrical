import math
import random

import matplotlib.pyplot as plt


class PID:
    def __init__(self, Kp, Ki, Kd, target, anti_windup):
        self.Kp = Kp
        self.Ki = Ki
        self.Kd = Kd

        self.target = target  # Target PSI to reach
        self.prev_error = 0  # Previous error used by derivative
        self.integral = 0

        self.max_output = 1
        self.min_output = 0

        self.enable_anti_windup = anti_windup

    def update(self, current_value):
        error = self.target - current_value

        self.integral += error
        derivative = error - self.prev_error

        # Apply weights and calculate output
        output = self.Kp * error + self.Ki * self.integral + self.Kd * derivative

        if self.enable_anti_windup:
            if output > self.max_output:
                output = self.max_output
                # If error and output are the same sign, stop integrating
                if error > 0:
                    self.integral -= error
            elif output < self.min_output:
                output = self.min_output
                # If error and output are the same sign, stop integrating
                if error < 0:
                    self.integral -= error

        self.prev_error = error
        return output


class Tank:
    def __init__(self, starting_pressure):
        self.pressure = starting_pressure

    def update(self, input_val, output_val):
        self.pressure += input_val - output_val
        return self.pressure


if __name__ == '__main__':
    tank = Tank(0)  # Set tank starting pressure

    # Proportional, Integral, and Derivative weight amounts.
    # Should the controller use anti-windup?

    # 25 psi range requirement

    actuator_state = 0  # Initial actuator state
    time_steps = 10000  # Milliseconds
    actuator_rate = 50  # Actuation rate (actuation/second)
    output_rate = 1  # Console log rate. (logs/second)
    initial_target = 0
    anti_windup = True
    actuator_open_rate = .5
    output_rate_high = .3
    output_rate_low = .2

    # PID weights:
    proportional = 1
    integral = .1
    derivative = .01

    pid = PID(Kp=proportional, Ki=integral, Kd=derivative, target=initial_target, anti_windup=anti_windup)

    # plot stuff
    pressures = []
    targets = []

    #  Simulation loop
    for i in range(time_steps):
        if i % (1000 / output_rate) == 0:
            print("Pressure: " + str(tank.pressure))

        #  Simulate live throttling by changing target pressure while simulation is running
        pid.target = 350 * math.sin(math.pi * i / (time_steps - 1))

        #  Turn actuator on or off depending on control signal state

        control_signal = pid.update(tank.pressure)

        # Update actuator 10 times a second
        if i % (1000 / actuator_rate) == 0:
            if control_signal > 0:
                actuator_state = control_signal;
                # actuator_state = actuator_open_rate
            elif control_signal <= 0:
                actuator_state = control_signal;
                # actuator_state = 0

        pressure_drop = random.uniform(output_rate_low, output_rate_high)  # Simulate air leaving tank
        pressure = tank.update(actuator_state, pressure_drop)

        #  plot stuff
        pressures.append(pressure)
        targets.append(pid.target)

        # time.sleep(1 / 1000)  # wait 1 ms

    #  more plot stuff
    plt.plot(pressures, label='Tank Pressure')
    plt.plot(targets, '--', label='Target Pressure')
    plt.xlabel("Time (ms)")
    plt.ylabel("Pressure (psi)")
    plt.title("Tank Pressure over Time using PID Control")
    plt.legend()
    plt.show()
