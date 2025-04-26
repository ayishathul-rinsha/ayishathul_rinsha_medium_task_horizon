#define motorA 5
#define motorB 6

int encoderPin1 = 2;
int encoderPin2 = 3;
volatile int lastEncoded = 0;
volatile long encoderValue = 0;

const long PPR = 3820; // Pulses Per Revolution (360 degrees)

bool waitingForInput = true;
float targetAngle = 0;

void setup() {
  Serial.begin(9600);
  pinMode(encoderPin1, INPUT);
  pinMode(encoderPin2, INPUT);
  digitalWrite(encoderPin1, HIGH);
  digitalWrite(encoderPin2, HIGH);

  attachInterrupt(digitalPinToInterrupt(encoderPin1), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPin2), updateEncoder, CHANGE);

  pinMode(motorA, OUTPUT);
  pinMode(motorB, OUTPUT);

  Serial.println("Enter the desired angle (between -180 to 180 degrees):");
}

void loop() {
  if (waitingForInput && Serial.available()) {
    targetAngle = Serial.parseFloat();
    if (targetAngle > 180) targetAngle = 180;
    if (targetAngle < -180) targetAngle = -180;

    Serial.print("Rotating to ");
    Serial.print(targetAngle);
    Serial.println(" degrees...");

    rotateToAngle(targetAngle);
    Serial.println("Rotation complete.");
    
    waitingForInput = true;
    Serial.println("Enter another angle:");
  }
}

void rotateToAngle(float desiredAngle) {
  float currentAngle = (float)encoderValue * 360.0 / (float)PPR;

  // Normalize currentAngle to -180° to 180°
  currentAngle = normalizeAngle(currentAngle);

  float angleToRotate = desiredAngle - currentAngle;

  // Handle shortest rotation (optional)
  if (angleToRotate > 180) angleToRotate -= 360;
  if (angleToRotate < -180) angleToRotate += 360;

  long pulsesToRotate = (angleToRotate * PPR) / 360;
  long targetEncoderValue = encoderValue + pulsesToRotate;

  if (angleToRotate > 0) {
    analogWrite(motorA, 127);
    digitalWrite(motorB, LOW);
    while (encoderValue < targetEncoderValue) {
      delay(1);
    }
  } else if (angleToRotate < 0) {
    analogWrite(motorA, 0);
    digitalWrite(motorB, HIGH);
    while (encoderValue > targetEncoderValue) {
      delay(1);
    }
  }

  // Stop the motor
  digitalWrite(motorA, LOW);
  digitalWrite(motorB, LOW);
}

float normalizeAngle(float angle) {
  while (angle > 180) angle -= 360;
  while (angle < -180) angle += 360;
  return angle;
}

void updateEncoder() {
  int MSB = digitalRead(encoderPin1);
  int LSB = digitalRead(encoderPin2);

  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue--;

  lastEncoded = encoded;
}
