#include <Arduino.h>
#include <IRremote.hpp>
#include <LedControl.h>
#include <LiquidCrystal.h>

#define IR_RECV_PIN 2
LedControl lc = LedControl(6, 5, 4, 1);
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

int secret;
int maxTurns = 6;

// IR remote 0-9
int numberCommands[10] = {0x16, 0xC,  0x18, 0x5E, 0x8,
                          0x1C, 0x5A, 0x42, 0x52, 0x4A};

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(9600);

  lcd.begin(16, 2);
  IrReceiver.begin(IR_RECV_PIN, ENABLE_LED_FEEDBACK);

  lc.shutdown(0, false);
  lc.setIntensity(0, 0);
  lc.clearDisplay(0);
}

int getGuess() {
  bool gotGuess = false;
  int guess = 0;

  while (!gotGuess) {
    if (IrReceiver.decode()) {
      int receivedCommand = IrReceiver.decodedIRData.command;
      Serial.print("Received command: ");
      Serial.println(receivedCommand, HEX);
      if (receivedCommand == 0x40 &&
          guess > 0) { // 0x40 corresponds to "Play/Pause"
        gotGuess = true;
      } else if (receivedCommand == 0x44) { // 0x44 corresponds to "Backspace"
        guess /= 10;
      } else {
        for (int i = 0; i < 10; i++) {
          if (receivedCommand == numberCommands[i]) {
            guess = guess * 10 + i;
            break;
          }
        }
      }
      lcd.setCursor(0, 1);
      lcd.print(guess);
      delay(150);
      IrReceiver.resume();
    }
  }
  Serial.print("Returning Guess: ");
  Serial.println(guess);
  return guess;
}

void lcColumns(int columnAmount) {
  if (columnAmount == 0) {
    lc.clearDisplay(0);
    return;
  }
  for (int j = 0; j < columnAmount; j++) {
    for (int i = 0; i < 8; i++) {
      lc.setLed(0, j, i, true);
    }
  }
}

void loop() {
  bool won = false;
  randomSeed(micros());
  secret = random(1, 100);

  lcd.print("Guess a number!");
  for (int i = 0; i < maxTurns; i++) {
    lc.clearDisplay(0);
    int remainingTurns = maxTurns - i;
    Serial.print("Remaining Turns: ");
    Serial.println(remainingTurns);
    lcColumns(remainingTurns);

    int guess = getGuess();
    if (guess > secret) {
      lcd.clear();
      lcd.print("Go Lower!");
    } else if (guess < secret) {
      lcd.clear();
      lcd.print("Go Higher!");
    } else {
      won = true;
      break;
    }
  }
  if (won) {
    lcColumns(0);
    lcd.clear();
    lcd.print("You win!");
    String winLineTwo = "Number was " + String(secret);
    lcd.setCursor(0, 1);
    lcd.print(winLineTwo);
  } else {
    lcColumns(0);
    lcd.clear();
    lcd.print("You lose!");
    String loseLineTwo = "Number was " + String(secret);
    lcd.setCursor(0, 1);
    lcd.print(loseLineTwo);
  }
  bool continuePressed = false;
  while (!continuePressed) {
    if (IrReceiver.decode()) {
      int receivedCommand = IrReceiver.decodedIRData.command;
      Serial.print("Received command: ");
      Serial.println(receivedCommand, HEX);
      continuePressed = true;
    }
  }
  lcd.clear();
}
