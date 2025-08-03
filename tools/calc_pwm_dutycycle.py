"""
Script to calculate duty cycle and other properties for PWM ADC driving
an AL8807 LED driver CTRL input.

Setup:
- ATtiny GPIO generating high-frequency PWM into RC low-pass filter
- RC filter connected to CTRL input of AL8807
- Calculations account for GPIO typical internal resistance
- Calculations assume typical values for AL8807 properties (Vref, internal resistor, etc.)

Usage:
1) Fill 'Vc' list with CTRL pin voltages where the duty cycle shall be calculated for
2) Check and adjust parameters as needed
3) Run script


(C) 2025, Felix Althaus

"""


# Input
Vc = [1.235, 0.490, 2.435]


# Parameters (typical):
Ri = 50         # GPIO internal resistance [Ohm]
Rs = 1000       # RC filter resistor [Ohm]
Vdd = 3.3       # unloaded GPIO high-level voltage [V]
                # (low-level is assumed as 0V)
RL = 10000      # Pull-down resistor at AL8807 CTRL input
Rctrl = 50000   # AL8807 internal resistance CTRL input to int. reference
Vref = 2.5      # AL8807 internal reference voltage


print("PWM ADC driving AL8807 LED Driver")
print()
print("Output unloaded:")
print()
print(" Vc [V]\t\tVo [V]\t\td [-]")
print(" "+"---"*12)

for x in Vc:

    d = round(x*(Ri+Rs)/(x*Ri + Vdd*Rs)*255)
    Vo = (x*Ri + Vdd*Rs) / (Ri+Rs)

    print(f" {x:1.3f}V\t\t{Vo:1.3f}V\t\t{d: 4d}")

print()
print()

print("Output connected to AL8807 CTRL pin:")
print("...")
#FXIME: to be implemented
