/*
  ESP32 + 5641AS
  SINGLE DIGIT TEST

  Only Digit 4 is connected.

  5641AS pinout:
    1  = E
    2  = D
    3  = DP
    4  = C
    5  = G
    6  = Digit 1
    7  = B
    8  = Digit 2
    9  = Digit 3
    10 = F
    11 = A
    12 = Digit 4

  Digit 4 (pin 12) -> GND

  ESP32:
    GPIO 23 -> resistor -> A (pin 11)
    GPIO 22 -> resistor -> B (pin 7)
    GPIO 21 -> resistor -> C (pin 4)
    GPIO 25 -> resistor -> D (pin 2)
    GPIO 26 -> resistor -> E (pin 1)
    GPIO 27 -> resistor -> F (pin 10)
    GPIO 32 -> resistor -> G (pin 5)

  Common cathode:
    GPIO HIGH = segment ON
    GPIO LOW  = segment OFF

  Serial commands:
    H   = show help
    0-9 = display that digit and pause countdown
    A   = all seven segments ON
    C   = resume countdown
*/

const int pinA = 23;
const int pinB = 22;
const int pinC = 21;
const int pinD = 25;
const int pinE = 26;
const int pinF = 27;
const int pinG = 32;

const int digit1Pin = 16;
const int digit2Pin = 17;
const int digit3Pin = 18;
const int digit4Pin = 19;
const unsigned long DISPLAY_REFRESH_US = 1000;  // 1 ms per digit

String serialText = "";
bool textMode = false;

int textOffset = -3;
unsigned long lastTextScroll = 0;

const unsigned long TEXT_SCROLL_MS = 1000;

int counter = 9999;

unsigned long lastCountTime = 0;
unsigned long countInterval = 1000;

bool countdownRunning = true;


// Turn all segments off
void allSegmentsOff()
{
  digitalWrite(pinA, LOW);
  digitalWrite(pinB, LOW);
  digitalWrite(pinC, LOW);
  digitalWrite(pinD, LOW);
  digitalWrite(pinE, LOW);
  digitalWrite(pinF, LOW);
  digitalWrite(pinG, LOW);
}


// Turn all seven segments on
void allSegmentsOn()
{
  digitalWrite(pinA, HIGH);
  digitalWrite(pinB, HIGH);
  digitalWrite(pinC, HIGH);
  digitalWrite(pinD, HIGH);
  digitalWrite(pinE, HIGH);
  digitalWrite(pinF, HIGH);
  digitalWrite(pinG, HIGH);
}


// Display a single digit
void displayDigit(int digit)
{
  allSegmentsOff();

  switch (digit) {

    case 0:
      digitalWrite(pinA, HIGH);
      digitalWrite(pinB, HIGH);
      digitalWrite(pinC, HIGH);
      digitalWrite(pinD, HIGH);
      digitalWrite(pinE, HIGH);
      digitalWrite(pinF, HIGH);
      break;

    case 1:
      digitalWrite(pinB, HIGH);
      digitalWrite(pinC, HIGH);
      break;

    case 2:
      digitalWrite(pinA, HIGH);
      digitalWrite(pinB, HIGH);
      digitalWrite(pinD, HIGH);
      digitalWrite(pinE, HIGH);
      digitalWrite(pinG, HIGH);
      break;

    case 3:
      digitalWrite(pinA, HIGH);
      digitalWrite(pinB, HIGH);
      digitalWrite(pinC, HIGH);
      digitalWrite(pinD, HIGH);
      digitalWrite(pinG, HIGH);
      break;

    case 4:
      digitalWrite(pinB, HIGH);
      digitalWrite(pinC, HIGH);
      digitalWrite(pinF, HIGH);
      digitalWrite(pinG, HIGH);
      break;

    case 5:
      digitalWrite(pinA, HIGH);
      digitalWrite(pinC, HIGH);
      digitalWrite(pinD, HIGH);
      digitalWrite(pinF, HIGH);
      digitalWrite(pinG, HIGH);
      break;

    case 6:
      digitalWrite(pinA, HIGH);
      digitalWrite(pinC, HIGH);
      digitalWrite(pinD, HIGH);
      digitalWrite(pinE, HIGH);
      digitalWrite(pinF, HIGH);
      digitalWrite(pinG, HIGH);
      break;

    case 7:
      digitalWrite(pinA, HIGH);
      digitalWrite(pinB, HIGH);
      digitalWrite(pinC, HIGH);
      break;

    case 8:
      allSegmentsOn();
      break;

    case 9:
      digitalWrite(pinA, HIGH);
      digitalWrite(pinB, HIGH);
      digitalWrite(pinC, HIGH);
      digitalWrite(pinD, HIGH);
      digitalWrite(pinF, HIGH);
      digitalWrite(pinG, HIGH);
      break;
  }
}


// Print Serial help
void printHelp()
{
  Serial.println();
  Serial.println("5641AS Test Commands");
  Serial.println("--------------------");
  Serial.println("H = show this help");
  Serial.println("0-9 = display digit and pause countdown");
  Serial.println("A = all seven segments ON");
  Serial.println("C = resume countdown");
  Serial.println();
}


// Process incoming Serial commands
void processSerial() {
  static String commandLine = "";

  while (Serial.available()) {
    char command = Serial.read();

    // Enter key = process the complete command
    if (command == '\n' || command == '\r') {

      if (commandLine.length() == 0) {
        continue;
      }

      // -------------------------------------------------
      // W <text> = display scrolling text
      // -------------------------------------------------
      if (commandLine.length() >= 2 &&
          (commandLine[0] == 'W' || commandLine[0] == 'w') &&
          commandLine[1] == ' ') {

        String newText = commandLine.substring(2);

        bool valid = true;

        // Check every character
        for (int i = 0; i < newText.length(); i++) {
          char c = newText[i];

          bool supported =
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
              c == ' ' ||
              c == '-' ||
              c == '_' ||
              c == '=';

          if (!supported) {
            Serial.print("ERROR: Unsupported character: ");
            Serial.println(c);
            valid = false;
            break;
          }
        }

        // Only change the display if the entire command is valid
        if (valid && newText.length() > 0) {
          serialText = newText;
          textMode = true;
          textOffset = 0;
          lastTextScroll = millis();
          
Serial.print("DEBUG: textMode = ");
Serial.println(textMode);

Serial.print("DEBUG: serialText = [");
Serial.print(serialText);
Serial.println("]");

Serial.print("DEBUG: text length = ");
Serial.println(serialText.length());

Serial.print("DEBUG: textOffset = ");
Serial.println(textOffset);


          Serial.print("Displaying: ");
          Serial.println(serialText);
        }

      } else {

        // -------------------------------------------------
        // Existing single-character commands
        // -------------------------------------------------

        if (commandLine.length() == 1) {
          char singleCommand = commandLine[0];

          switch (singleCommand) {

            case 'H':
            case 'h':
              printHelp();
              break;

            case 'A':
            case 'a':
              textMode = false;
              allSegmentsOn();
              break;

            case 'C':
            case 'c':
              textMode = false;
              break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
              textMode = false;
              displayDigit(singleCommand - '0');
              break;

            default:
              Serial.print("Unknown command: ");
              Serial.println(singleCommand);
              break;
          }
        } else {
          Serial.println("Unknown command.");
        }
      }

      // Clear command buffer
      commandLine = "";

    } else {
      // Add normal characters to command buffer
      commandLine += command;
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Segment pins
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT);
  pinMode(pinD, OUTPUT);
  pinMode(pinE, OUTPUT);
  pinMode(pinF, OUTPUT);
  pinMode(pinG, OUTPUT);

  // Digit common pins
  pinMode(digit1Pin, OUTPUT);
  pinMode(digit2Pin, OUTPUT);
  pinMode(digit3Pin, OUTPUT);
  pinMode(digit4Pin, OUTPUT);

  // All digits OFF initially.
  // Common-cathode display: LOW = ON, HIGH = OFF.
  digitalWrite(digit1Pin, LOW);
  digitalWrite(digit2Pin, LOW);
  digitalWrite(digit3Pin, LOW);
  digitalWrite(digit4Pin, LOW);

  allSegmentsOff();

  Serial.println();
  
  

  Serial.println("5641AS single-digit countdown test");

  printHelp();

  Serial.println("Countdown starting...");

  Serial.print("Count: ");
  Serial.println(counter);

  lastCountTime = millis();
}

void cycleDigits() {
  static unsigned long lastRefresh = 0;
  static int currentDigit = 0;

  // Refresh one digit every 1 ms
  if (micros() - lastRefresh < 1000) {
    return;
  }

  lastRefresh = micros();

  // -------------------------------------------------
  // 1. Turn ALL digits OFF
  // -------------------------------------------------
  digitalWrite(digit1Pin, HIGH);
  digitalWrite(digit2Pin, HIGH);
  digitalWrite(digit3Pin, HIGH);
  digitalWrite(digit4Pin, HIGH);

  // Blank previous digit briefly
  delayMicroseconds(20);

  // -------------------------------------------------
  // 2. Determine what this digit should display
  // -------------------------------------------------

  if (textMode) {

    int textIndex = textOffset + currentDigit;

    if (textIndex >= 0 && textIndex < serialText.length()) {
      displayCharacter(serialText[textIndex]);
    } else {
      allSegmentsOff();
    }

  } else {

    int digitValue;

    switch (currentDigit) {
      case 0:
        digitValue = (counter / 1000) % 10;
        break;

      case 1:
        digitValue = (counter / 100) % 10;
        break;

      case 2:
        digitValue = (counter / 10) % 10;
        break;

      case 3:
        digitValue = counter % 10;
        break;

      default:
        digitValue = 0;
        break;
    }

    displayDigit(digitValue);
  }

  // -------------------------------------------------
  // 3. Turn ON selected digit
  // -------------------------------------------------

  switch (currentDigit) {
    case 0:
      digitalWrite(digit1Pin, LOW);
      break;

    case 1:
      digitalWrite(digit2Pin, LOW);
      break;

    case 2:
      digitalWrite(digit3Pin, LOW);
      break;

    case 3:
      digitalWrite(digit4Pin, LOW);
      break;
  }

  // -------------------------------------------------
  // 4. Advance to next digit
  // -------------------------------------------------

  currentDigit++;

  if (currentDigit >= 4) {
    currentDigit = 0;
  }
}

void updateTextScroll() {
  if (!textMode || serialText.length() <= 4) {
    return;
  }

  if (millis() - lastTextScroll < TEXT_SCROLL_MS) {
    return;
  }

  lastTextScroll = millis();

  textOffset++;

  int maxOffset = serialText.length() - 4;

  if (textOffset > maxOffset) {
    textOffset = 0;
  }

  Serial.print("TEXT offset = ");
  Serial.println(textOffset);
}


void displayCharacter(char c) {
  allSegmentsOff();

  switch (c) {
    case '0': displayDigit(0); break;
    case '1': displayDigit(1); break;
    case '2': displayDigit(2); break;
    case '3': displayDigit(3); break;
    case '4': displayDigit(4); break;
    case '5': displayDigit(5); break;
    case '6': displayDigit(6); break;
    case '7': displayDigit(7); break;
    case '8': displayDigit(8); break;
    case '9': displayDigit(9); break;

    case 'A':
    case 'a':
      digitalWrite(pinA, HIGH);
      digitalWrite(pinB, HIGH);
      digitalWrite(pinC, HIGH);
      digitalWrite(pinE, HIGH);
      digitalWrite(pinF, HIGH);
      digitalWrite(pinG, HIGH);
      break;

    case 'B':
    case 'b':
      digitalWrite(pinC, HIGH);
      digitalWrite(pinD, HIGH);
      digitalWrite(pinE, HIGH);
      digitalWrite(pinF, HIGH);
      digitalWrite(pinG, HIGH);
      break;

    case 'C':
    case 'c':
      digitalWrite(pinA, HIGH);
      digitalWrite(pinD, HIGH);
      digitalWrite(pinE, HIGH);
      digitalWrite(pinF, HIGH);
      break;

    case 'D':
    case 'd':
      digitalWrite(pinB, HIGH);
      digitalWrite(pinC, HIGH);
      digitalWrite(pinD, HIGH);
      digitalWrite(pinE, HIGH);
      digitalWrite(pinG, HIGH);
      break;

    case 'E':
    case 'e':
      digitalWrite(pinA, HIGH);
      digitalWrite(pinD, HIGH);
      digitalWrite(pinE, HIGH);
      digitalWrite(pinF, HIGH);
      digitalWrite(pinG, HIGH);
      break;

    case 'F':
    case 'f':
      digitalWrite(pinA, HIGH);
      digitalWrite(pinE, HIGH);
      digitalWrite(pinF, HIGH);
      digitalWrite(pinG, HIGH);
      break;

    case 'G':
    case 'g':
      digitalWrite(pinA, HIGH);
      digitalWrite(pinB, HIGH);
      digitalWrite(pinC, HIGH);
      digitalWrite(pinD, HIGH);
      digitalWrite(pinF, HIGH);
      digitalWrite(pinG, HIGH);
      break;

    case 'H':
    case 'h':
      digitalWrite(pinB, HIGH);
      digitalWrite(pinC, HIGH);
      digitalWrite(pinE, HIGH);
      digitalWrite(pinF, HIGH);
      digitalWrite(pinG, HIGH);
      break;

    case 'I':
    case 'i':
      digitalWrite(pinB, HIGH);
      digitalWrite(pinC, HIGH);
      break;

    case 'J':
    case 'j':
      digitalWrite(pinB, HIGH);
      digitalWrite(pinC, HIGH);
      digitalWrite(pinD, HIGH);
      digitalWrite(pinE, HIGH);
      break;

case 'K':
case 'k':
  digitalWrite(pinB, HIGH);
  digitalWrite(pinC, HIGH);
  digitalWrite(pinE, HIGH);
  digitalWrite(pinF, HIGH);
  digitalWrite(pinG, HIGH);
  break;

    case 'L':
    case 'l':
      digitalWrite(pinD, HIGH);
      digitalWrite(pinE, HIGH);
      digitalWrite(pinF, HIGH);
      break;

case 'M':
case 'm':
  digitalWrite(pinA, HIGH);
  digitalWrite(pinC, HIGH);
  digitalWrite(pinE, HIGH);
  digitalWrite(pinF, HIGH);
  break;

    case 'N':
    case 'n':
      digitalWrite(pinC, HIGH);
      digitalWrite(pinE, HIGH);
      digitalWrite(pinG, HIGH);
      break;

    case 'O':
    case 'o':
      digitalWrite(pinC, HIGH);
      digitalWrite(pinD, HIGH);
      digitalWrite(pinE, HIGH);
      digitalWrite(pinG, HIGH);
      break;

    case 'P':
    case 'p':
      digitalWrite(pinA, HIGH);
      digitalWrite(pinB, HIGH);
      digitalWrite(pinE, HIGH);
      digitalWrite(pinF, HIGH);
      digitalWrite(pinG, HIGH);
      break;

      case 'Q':
case 'q':
  digitalWrite(pinA, HIGH);
  digitalWrite(pinB, HIGH);
  digitalWrite(pinC, HIGH);
  digitalWrite(pinD, HIGH);
  digitalWrite(pinE, HIGH);
  digitalWrite(pinF, HIGH);
  break;

    case 'R':
    case 'r':
      digitalWrite(pinE, HIGH);
      digitalWrite(pinG, HIGH);
      break;

    case 'S':
    case 's':
      digitalWrite(pinA, HIGH);
      digitalWrite(pinC, HIGH);
      digitalWrite(pinD, HIGH);
      digitalWrite(pinF, HIGH);
      digitalWrite(pinG, HIGH);
      break;

    case 'T':
    case 't':
      digitalWrite(pinD, HIGH);
      digitalWrite(pinE, HIGH);
      digitalWrite(pinF, HIGH);
      digitalWrite(pinG, HIGH);
      break;

    case 'U':
    case 'u':
      digitalWrite(pinB, HIGH);
      digitalWrite(pinC, HIGH);
      digitalWrite(pinD, HIGH);
      digitalWrite(pinE, HIGH);
      digitalWrite(pinF, HIGH);
      break;

      case 'V':
case 'v':
  digitalWrite(pinB, HIGH);
  digitalWrite(pinC, HIGH);
  digitalWrite(pinD, HIGH);
  digitalWrite(pinE, HIGH);
  break;

  case 'W':
case 'w':
  digitalWrite(pinB, HIGH);
  digitalWrite(pinC, HIGH);
  digitalWrite(pinD, HIGH);
  digitalWrite(pinE, HIGH);
  break;

  case 'X':
case 'x':
  digitalWrite(pinB, HIGH);
  digitalWrite(pinC, HIGH);
  digitalWrite(pinE, HIGH);
  digitalWrite(pinF, HIGH);
  digitalWrite(pinG, HIGH);
  break;

    case 'Y':
    case 'y':
      digitalWrite(pinB, HIGH);
      digitalWrite(pinC, HIGH);
      digitalWrite(pinD, HIGH);
      digitalWrite(pinF, HIGH);
      digitalWrite(pinG, HIGH);
      break;

case 'Z':
case 'z':
  digitalWrite(pinA, HIGH);
  digitalWrite(pinB, HIGH);
  digitalWrite(pinD, HIGH);
  digitalWrite(pinE, HIGH);
  digitalWrite(pinG, HIGH);
  break;

    case '-':
      digitalWrite(pinG, HIGH);
      break;

   case '_':
      digitalWrite(pinD, HIGH);
      break;

      case '=':
      digitalWrite(pinD, HIGH);
      digitalWrite(pinG, HIGH);
      break; 

  
    case ' ':
      // Blank
      break;

    default:
      // Unsupported character = blank
      break;
  }
}


void loop()
{
  // Check for Serial test commands
  cycleDigits();
  updateTextScroll();
  processSerial();
  // Countdown
  if (countdownRunning) {
    unsigned long now = millis();

    if (now - lastCountTime >= countInterval) {

      lastCountTime += countInterval;

      if (counter > 0) {
        counter--;
      }
      else {
        counter = 9999;
      }

    //  displayDigit(counter % 10);
      
      Serial.print("Count: ");
      Serial.println(counter);
      Serial.print("countInterval: ");
      Serial.println(countInterval);
    }
  }
}
