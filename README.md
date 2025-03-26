# LM2596 MPPT charger
This project uses a standard LM2596 based DC-DC step-down module to achieve MPPT of a solar panel.
It was built for a **30 Watt** solar panel with 21V open circuit / ~ 16V MPP voltage, which seems to be the maximum power rating for the poor LM2596 chip (which needs to be cooled with a heatsink)

In contrast to other solar MPPT projects using custom boards with ideally balanced circuits to achieve a very high efficiency, this re-uses an **off-the-shelf voltage stepdown module**, an Arduino and some other components to achieve **good-enough efficiency for small installations**. I am able to reach **80% efficiency** under full sunlight.

The single common ground level allows for easy voltage measurements.


## Main idea

To understand the concept behind this MPPT charger, we need to understand the base schematic of those LM2596 modules:

![base schematic](images/initial_circuit.png)

Simplified a lot, it works like this: The input voltage is stabilized by using a capacitor and fed into the LM2596 chip. It basically acts like a switch controlled with a very high frequency (in the 100kHz range), resulting in a PWM signal at the output oscillating between 0V and the input voltage. Using the inductor, diode and capacitor at the output as a low-pass filter, the output is smoothed to a stable lower voltage. The magic comes from the feedback loop: Through a resistor divider, the output voltage is fed back into the LM2596. It tries to keep the feedback pin at exactly 1.23 V. If the voltage at the feedback pin is too low, which means the output voltage itself is too low, it closes its internal switch and connects input to output. After a very short time, the output voltage has risen, so the feedback voltage gets too high. This makes the LM2596 disconnect its switch, so the voltage falls again. By doing this at very high frequency, the output voltage ever so slightly oscillates around the desired voltage. By adjusting values in the resistor divider, the desired output voltage can be set - that's whats the potentiometer on the board is for.

When doing MPPT, we need to drive the solar panel at a specific voltage (which is about 80% of its open circuit voltage). And that's what we can use the LM2596's feedback loop for: Instead of taking it from the output, we need to derive the feedback signal from the input voltage. There's only one problem: When the feedback pin is too low, the chip believes the output is too low and therefore connects input to output to let current flow. In our modified case, things are the wrong way around: In this case, letting current flow means putting more load on the supply, i.e. the solar panel, which makes its voltage decrease even more.
To solve this problem, we use an NPN transistor to invert the feedback signal.
If now the **input voltage is too low**, the input to the inverting transistor is low, which means the output connected to the feedback pin is high(er than 1.23 V, that's all that really matters). The LM296 reacts to the high feedback by disconnecting the output from the input, removing the solar panel's load, **allowing its voltage to rise** - exactly what we want.
The same works in the opposite case. An **input voltage too high** leads to a high inverter input, therefore a low feedback pin makes the LM2596 put load on the panel, **dropping the voltage**.
Through high frequency oscillations, this allows driving the panel at a specific voltage.
Again, we can set this voltage by varying the resistor divider.

Our new schematic looks like this:

![modded circuit](images/modded_circuit.png)

## the Arduino part

Driving the panel at a specific voltage works without active control by an Arduino.
Its task is to monitor the battery charge level and find the exact maximum power point by implementing a tracking algorithm. To allow it to control the resistor divider, we need to add a digital potentiometer IC whose resistance can be controlled in software. As the digital potentiometer, a **MCP42010** is used, which has 10 kOhm, 2 channels (only one used here) and 256 steps.

The current arduino firmware does not implement MPP tracking, instead it keeps the panel voltage at 16V and increases it to lower the power as the battery gets fully charged. This approach worked fine for me for the last few months.

There's no deep discharge protection for the battery when feeding the Arduino and USB ports!

## modding your LM2596 Board

As seen in the schematic, we need to locate the feedback loop on the board (highlighted in yellow)

![circuit with feedback loop highlighted](images/feedback_loop.png)

Un-solder the potentiometer between feedback and out+ and the SMD resistor (labelled R2 on this PCB) between feedback and ground. Add a wire to the feedback pin to connect it to the new circuit.

![board with potentiometer and resistor removed](images/removed_resistors.jpg)

The new schematic looks like this. The part at the top represents the original step-down module, the middle part the added resistor divider and transistor, and the entire bottom part is for controlling the digipot IC with an arduino.
The 3.3V supply needed for the transistor can be taken from the Arduino.

![finished circuit](images/finished_circuit.png)

Transistor and resistor values are product of what was lying around on my desk.
Why is the digital potentiometer on the low side between ground and feedback? That's because, although it behaves like a "real" potentiometer, its connections must never exceed the 0 - VCC (5V) range. By putting the digipot on the low side, we ensure that we stay within this range and don't damage it.

My board looks like this:

![board 1](images/board1.jpg)

![board 2](images/board2.jpg)

I mounted the LM2596 module vertically onto a perfboard. There are some leftover wires and resistors from my prototyping stage which I don't bother removing. An old heatsink is clamped to the back, where the metal clamp cools the chip itself - simple, effective and silent.

To measure power, and solar and battery voltage, there are two **INA219** power sensor breakout boards connected via I²C. 

For my project, I made a (somewhat) nice case with analog meters, and buttons to switch between the internal battery (old RC battery pack with 6s NiMH) and an external one. A 5V regulator from a mechanically broken cigarette lighter adapter provides power to the Arduino and some USB ports on the front. <del>The display is still work in progress.</del>

![case front](images/front.jpg)

![case back](images/back.jpg)

It's pretty messy in there...

Inspired by this article by zabex (in German): [http://www.zabex.de/site/mpptracker.html](http://www.zabex.de/site/mpptracker.html)

## Thoughts after using this device for a year
 - I didn't add the (e-ink) display. Displaying things like the last day's power history may be a nice gimmick, but it didn't add any value to me in the long run.
 - I never leave it unattended. The battery and solar panel are disconnected at night and when I'm away during the day. This is because the device itself doesn't have that much capacity (see next point), and I mainly use it as a glorified USB charger. Not that this is a bad thing.
 - The battery I use as "internal" is an old 6s NiMH RC car battery with not much capacity left. But it works fine for this purpose. To get a desired voltage at the solar panel (16V MPP), there needs to be a sink where the converter can pump as much power as needed. That is what the battery is for. I don't use it to power things at night. Directly connected to the battery is a 5V regulator that powers the Arduino (drawing a negligible amount) and some USB ports where I can charge my phone and stuff. Since the battery is almost always fully charged, the regulator raises the panel voltage (16 ... 20V open circuit) to match the power dynamically drawn by the USB devices. But it is also able to quickly provide more current if needed or if there are some clouds due to the internal battery. The main problem is: USB devices don't know and don't care that this is a solar-powered system and will draw whatever power they need. This goes against the solar MPP idea of taking whatever power is available from the solar panel.
 - The second use is to charge a 2s Li-Ion battery that I use in my netbook. The difference to USB phone charging is that it can charge at whatever current is available and actually take advantage of all that MPP stuff.
 - There is no MPP tracking algorithm implemented yet, it just sets it to 16V and increases that voltage when less power is needed and the battery voltage is high enough. The whole thing isn't that efficient (maybe 80% at best), so those few percent that could be harvested aren't worth it. Also, the way the digital potentiometer is wired, the panel voltage can be adjusted in maybe 0.5V steps. Not precise enough to find the MPP. But like I said, I don't care. That is not the purpose of this unit.
 - The three analog meters (battery voltage, input current, and panel voltage) are extremely valuable because I can see the state of the system at a glance. Also, they don't produce light, don't flicker (looking at you, e-ink), more like an analog wall clock. For example, when I charge my phone from a USB port, I have to manually make sure the internal battery does not run down. On the "input current" meter, which basically shows "input power" since the input voltage is always around 16V, you can mark compensation points that show how much current you can draw without draining the battery. Basically, if I see that the input power is above value x, which is marked with a dot on the scale, I know that it can keep my phone charged in slow mode. And if the power is above y, I can plug it into the fast charge port. (Tip: If the D+ and D- data wires are connected on a USB port, it signals that the device can draw more than 500mA/2.5W. Not every device respects this, though.)

This is definitely not a setup-and-forget kind of thing. I have to get a feel for what the three meters are telling me about the state (do I need to disconnect the load because there's not enough power? Is it reducing its power consumption (leaving the MPP) and I could actually plug in another USB device?)

But it's a bit forgiving, because energy is simply wasted if too little power is drawn, and the battery can compensate if too much is drawn from the USB ports. So no problem concentrating on other things while it is on.

If anyone actually dares to replicate this, *please* contact me! There's so much more to say.
