# ESP32 4-Digit 7-Segment Countdown & Scrolling Text Display

An Arduino/ESP32 project using a **5641AS 4-digit, 7-segment LED display** as a practical introduction to multiplexing, serial commands, character rendering, and embedded display timing.

The project began as a simple `9999` countdown timer and has evolved to support displaying and scrolling text received through the Arduino Serial Monitor.

## Features

* 4-digit 7-segment LED display
* Multiplexed display operation
* Starts at `9999` and counts down once per second
* Automatically wraps from `0000` back to `9999`
* Serial command interface
* Single-digit display testing
* All-segments-on test
* Scrolling text mode
* Supports letters `A-Z` and `a-z`
* Supports digits `0-9`
* Supports spaces
* Supports `-`, `_`, and `=`
* Lowercase characters use the same 7-segment glyphs where appropriate
* Approximate glyphs are used for letters that cannot be accurately represented on seven segments
* Countdown continues running while scrolling text is displayed
* Multiplexing remains independent of the countdown timer
* Display blanking interval reduces visible ghosting during multiplexing

## Hardware

### Microcontroller

* ELEGOO ESP32 DevKit V1
* ESP32-WROOM-32

### Display

* 5641AS 4-digit 7-segment display
* Common-cathode configuration
* One resistor per segment

The display's digit commons are driven LOW to enable a digit.

### Display Pin Mapping

| 5641AS Pin | Function | ESP32 GPIO |
| ---------: | -------- | ---------: |
|          1 | E        |         26 |
|          2 | D        |         25 |
|          3 | DP       |         33 |
|          4 | C        |         21 |
|          5 | G        |         32 |
|          6 | Digit 1  |         16 |
|          7 | B        |         22 |
|          8 | Digit 2  |         17 |
|          9 | Digit 3  |         18 |
|         10 | F        |         27 |
|         11 | A        |         23 |
|         12 | Digit 4  |         19 |

The decimal point is currently unused.

### Segment Layout

The display segments are conventionally arranged:

```text
       A
      ---
   F |   | B
      -G-
   E |   | C
      ---
       D
```

## Multiplexing

The four digits share the seven segment connections. Only one digit is enabled at a time.

The program rapidly cycles through:

```text
Digit 1
Digit 2
Digit 3
Digit 4
Digit 1
...
```

Each digit is refreshed approximately every 1 ms, producing a complete four-digit refresh cycle of approximately 4 ms, or about 250 Hz.

The human eye perceives the rapidly refreshed digits as a continuously illuminated four-digit display.

### Ghosting Prevention

Before changing the segment outputs, all four digit commons are turned off.

The sequence is:

```text
1. Disable all digits
2. Wait briefly
3. Set segment outputs
4. Enable the selected digit
5. Move to the next digit
```

A short blanking interval is used to prevent the previous digit from briefly displaying the new segment pattern.

## Countdown Mode

The default display is a four-digit countdown beginning at:

```text
9999
```

The counter decrements once per second:

```text
9999
9998
9997
...
0002
0001
0000
9999
```

The countdown timing is independent of display multiplexing.

The multiplexing code runs continuously and determines what the display should show. The countdown code only updates the value of `counter`; it does not directly manipulate the segment GPIOs.

This separation is important because directly changing the segment outputs from the countdown timer would interfere with multiplexing.

## Serial Commands

Open the Arduino Serial Monitor at:

```text
115200 baud
```

### Help

```text
H
```

or:

```text
h
```

Displays the available commands.

### All Segments

```text
A
```

Turns on all seven segments.

This is useful for verifying the segment wiring.

### Resume Countdown

```text
C
```

Resumes normal countdown display operation.

### Display a Digit

Send a single digit:

```text
0
```

through:

```text
9
```

This is useful for testing individual numeric glyphs.

### Display Text

Use:

```text
W <text>
```

For example:

```text
W HELLO
```

or:

```text
W 123456789
```

Text longer than four characters scrolls across the display.

For example:

```text
W 123456789
```

produces approximately:

```text
1234
2345
3456
4567
5678
6789
1234
...
```

The scroll speed is controlled by:

```cpp
const unsigned long TEXT_SCROLL_MS = 300;
```

## Supported Characters

The display supports:

### Numbers

```text
0 1 2 3 4 5 6 7 8 9
```

### Letters

```text
A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
```

Both uppercase and lowercase input are accepted.

Because a seven-segment display has only seven independently controlled segments, some letters cannot be represented accurately. Those characters use the closest practical approximation.

For example, `X`, `M`, `K`, `V`, and `W` are necessarily approximations.

### Other Characters

Currently supported:

```text
space
-
_
=
```

These use the following segment patterns:

```text
-    middle segment (G)
_    bottom segment (D)
=    middle + bottom segments (G + D)
```

## Text Rendering

Character rendering is handled by:

```cpp
displayCharacter(char c)
```

Numeric characters use the existing:

```cpp
displayDigit(int digit)
```

function.

This keeps numeric and alphabetic rendering separate while allowing both to be used by the multiplexing routine.

## Program Structure

The main display logic is divided into several responsibilities.

### `cycleDigits()`

Handles the high-speed multiplexing of the four display digits.

It determines whether the display is currently showing:

* the countdown value, or
* scrolling text.

It is the primary owner of the display segment and digit GPIOs.

### `displayDigit()`

Displays one numeric digit using the appropriate seven-segment pattern.

### `displayCharacter()`

Displays a numeric or alphabetic character by setting the appropriate segment outputs.

### `updateTextScroll()`

Updates the current position within the text at the configured scroll interval.

### `processSerial()`

Receives and interprets commands from the Serial Monitor.

The `W` command collects an entire line so that multi-character text can be entered.

## Why Multiplexing Is Separate From the Countdown

A key design point in this project is that **display refresh and countdown timing are two completely different timing systems**.

The display needs to be refreshed hundreds of times per second:

```text
~250 Hz
```

The counter only changes once per second:

```text
1 Hz
```

The program therefore does not "refresh the countdown four times per second" or otherwise tie the two operations together.

Instead:

```text
                    ┌── counter changes once/sec
                    │
loop() ─────────────┤
                    │
                    ├── text position changes every 300 ms
                    │
                    └── display multiplexing runs continuously
```

This allows the display to remain stable while other program functions operate at much slower rates.

## Ghosting and Display Timing

The display originally exhibited brief ghosting when switching between digits.

This occurs because the segment GPIOs can change while the previous digit is still enabled.

The display routine therefore uses a blanking sequence:

```text
ALL DIGITS OFF
       ↓
short delay
       ↓
SET SEGMENTS
       ↓
ENABLE CURRENT DIGIT
```

The current blanking interval is approximately:

```cpp
delayMicroseconds(20);
```

The multiplex refresh interval is approximately:

```cpp
1000 microseconds
```

These values can be adjusted if different display hardware or brightness characteristics require it.

## Development Notes

This project is intentionally simple and uses direct GPIO control rather than a dedicated LED driver IC.

It is primarily intended as a learning and experimentation project covering:

* GPIO control
* seven-segment displays
* multiplexing
* timing
* serial input
* character rendering
* state machines
* separation of application timing from display timing

It also demonstrates an important embedded-systems design principle:

> One subsystem should have clear ownership of a shared hardware resource.

In this project, `cycleDigits()` owns the display outputs. The countdown and text systems provide it with the information to display rather than independently manipulating the display hardware.

## Possible Future Improvements

Potential future additions include:

* More punctuation characters
* Decimal point support
* Configurable scroll speed
* Text entry buffer limits
* Better glyphs for difficult letters
* Text scrolling with blank space entering and leaving the display
* Pause/resume text mode
* Serial commands for changing the countdown value
* Configurable countdown interval
* Button controls
* Non-blocking diagnostic/test modes
* Display brightness control through multiplex duty cycle
* Dedicated LED driver hardware

## Software

Developed using:

* Arduino IDE 1.8.19
* ESP32 Arduino Core 3.3.11

The sketch is:

```text
Countdown.ino
```

## License

See the repository license for the terms under which this project may be used, modified, and distributed.
