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
Vc = [0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 2.0, 2.5]


# Parameters (typical):
Ri = 50         # GPIO internal output resistance [Ohm]
Rs = 1000       # RC filter resistor [Ohm]
Vdd = 3.3       # unloaded GPIO high-level voltage [V]
                # (low-level is assumed as 0V)
RL = 10e3       # Pull-down resistor at AL8807 CTRL input
Rctrl = 50e3    # AL8807 internal resistance CTRL input to int. reference
Vref = 2.5      # AL8807 internal reference voltage
N = 2           # Number of AL8807 connected to PWM ADC output


print("PWM ADC driving AL8807 LED Driver")
print()
print("Output unloaded:")
print()
print(" Vc [V]\t\tVo [V]\t\t  d [-]")
print(" "+"-"*40)

for x in Vc:
    d = round(x*(Ri+Rs)/(x*Ri + Vdd*Rs)*255)
    Vo = (x*Ri + Vdd*Rs) / (Ri+Rs)

    print(f" {x:1.3f}V\t\t{Vo:1.3f}V\t\t{d:3d} / {round(d/255*100, 1):.1f}%")

print()
print()

print("Output connected to AL8807 CTRL pin:")
print()
print(" d [-]\t\td [%]\t\tVctrl [V]")
print(" "+"-"*40)

for ocr1a in [40, 100, 158]:
    d = ocr1a/256
    v_ctrl =  (Rs*Vref+Rctrl/2*Vdd*d)*RL/(RL*Rs+Rctrl/N*Rs+Rctrl/N*RL)
    print(f" {ocr1a:3d}\t\t{d*100:.1f}%\t\t{v_ctrl:1.3f}V")

print()
print()

print(" Vctrl [V]\td [%]\t\td [-]")
print(" "+"-"*36)
for x in Vc:
    d = (x/RL*(RL*Rs+Rctrl/N*Rs+Rctrl/N*RL) - Rs*Vref)/(Rctrl/2*Vdd)
    ocr1a = round(d*256)
    print(f" {x:1.3f}V\t\t{d*100:.1f}%\t\t{ocr1a:4d}")
