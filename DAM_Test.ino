#include <Chrono.h>
#include <LightChrono.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

Adafruit_BicolorMatrix matrix[3] = { Adafruit_BicolorMatrix(),Adafruit_BicolorMatrix(),Adafruit_BicolorMatrix() };

int matrixHeight[3] = { 9,9,9 };
int matrixType[3] = { 0,1,2 };                 // Colors: Red, Yellow, Green
int matrixState[3] = { 0, 0, 0 };
int buttonState[3] = { 0,0,0 };               // status of each button in listed order: 11,12,13
int buttonInput[3] = { 2,7,8 };
int moleHit[3] = { false,false,false };
int buttonLED[3] = { 3,5,6 };
int randomTime[3] = { 0,0,0 };
int matrixBrightness = 16; // The red mole can vary in brightness 
int currentScore = 0;
int escapedMoles = 0;


#pragma region Mole Frames

static const byte moleFrames[11][8] PROGMEM  // 8-bit frames for all the mole expressions
{
	// Look left
	{
	  B00111100,
	  B01000010,
	  B10101001,
	  B10101001,
	  B10010001,
	  B10111001,
	  B10000001,
	  B10000001
	},
	// Look right
	{
	  B00111100,
	  B01000010,
	  B10010101,
	  B10010101,
	  B10001001,
	  B10011101,
	  B10000001,
	  B10000001
	},
	// Look center
	{
		// Fly version

		//B00000000,
		//B00110110,
		//B01001001,
		//B00101010,
		//B00011100,
		//B00111110,
		//B00011100,
		//B00000000
	  B00111100,
	  B01000010,
	  B10100101,
	  B10100101,
	  B10011001,
	  B10011001,
	  B10111101,
	  B10000001
	},
	// Angry face
	{
	  B00111100,
	  B01000010,
	  B10100101,
	  B10011001,
	  B10100101,
	  B10100101,
	  B10011001,
	  B10000001
	},
	// Taunt face
	{
		B00111100,
		B01000010,
		B10100101,
		B10100101,
		B10011001,
		B10111101,
		B10011001,
		B10011001
	},
	// Cheeky face
	{
		B00111100,
		B01000010,
		B10100101,
		B10100101,
		B10011001,
		B11011011,
		B11000011,
		B10111101
	},
	// Cooled-down face
	{
	  B00111100,
	  B01000010,
	  B10000001,
	  B11100111,
	  B11000011,
	  B10011001,
	  B10111101,
	  B10000001
	},
	// Anxious face one
	{
	  B10111101,
	  B01000010,
	  B10100101,
	  B10100101,
	  B10000001,
	  B10011001,
	  B10100101,
	  B10111101
	},
	// Anxious face two
	{
	  B00111100,
	  B01000010,
	  B10100101,
	  B10100101,
	  B11000011,
	  B10011001,
	  B10100101,
	  B10000001
	},
	// Frown face
	{
	  B00111100,
	  B01000010,
	  B10100101,
	  B10100101,
	  B10011001,
	  B10000001,
	  B10011001,
	  B10100101
	},
	// Solid color
	{
			  B11111111,
			  B11111111,
			  B11111111,
			  B11111111,
			  B11111111,
			  B11111111,
			  B11111111,
			  B11111111
	  }
};
#pragma endregion

Chrono matrixTimer[3];
Chrono matrixTimerTwo[3];

void setup() {
	Serial.begin(9600); // serial for debugging
	for (int i = 0; i < 3; i++)
	{
		matrix[i].begin(0x70 + i);  // pass in the address
		matrix[i].setRotation(1);
		// initialize the LED pin as an output:
		pinMode(buttonLED[i], OUTPUT);
		// initialize the pushbutton pin as an input:
		pinMode(buttonInput[i], INPUT);
	}
}

void loop() {
	Serial.println(escapedMoles);
	for (int i = 0; i < 3; i++)
	{
		buttonState[i] = digitalRead(buttonInput[i]);
		moleController(i);
		//if (buttonState[i] == HIGH)
		//{
		//	digitalWrite(buttonLED[i], HIGH);
		//}
		//else
		//{
		//	digitalWrite(buttonLED[i], LOW);
		//}
	}
}

void moleController(int mole)
{
	switch (matrixState[mole])
	{
	case 0:
		moleRise(matrix[mole], matrixHeight[mole], matrixType[mole], matrixTimer[mole]);
		break;
	case 1:
		moleLook(matrix[mole], matrixType[mole], matrixState[mole], matrixTimer[mole], matrixTimerTwo[mole]);
		break;
	case 2:
		moleLeave(matrix[mole], matrixHeight[mole], matrixType[mole], matrixTimer[mole], matrixTimerTwo[mole]);
		break;
	case 3:
		moleEscape(matrixTimer[mole], matrixTimerTwo[mole], matrixType[mole]);
		break;
	default:
		break;
	}
}

void moleRise(Adafruit_BicolorMatrix & mole, int & moleHeight, int moleType, Chrono & timer)
{
	if (timer.hasPassed(100)) {
		timer.restart(); // Restart the chronometer.
		if (moleHeight >= 0)
		{
			mole.clear();
			mole.drawBitmap(0, moleHeight--, moleFrames[2], 8, 8, moleType + 1);
		}
		else
		{
			randomTime[moleType] = random(8000, 12000);
			matrixState[moleType] = 1;
		}
		mole.writeDisplay();
	}
}

void moleLook(Adafruit_BicolorMatrix & mole, int moleType, int moleState, Chrono & timer, Chrono & timerTwo)
{
	//Serial.println(randomTime[moleType]);
	if (timerTwo.hasPassed(randomTime[moleType])) {
		timerTwo.restart();
		mole.clear();
		mole.drawBitmap(0, 0, moleFrames[2], 8, 8, moleType + 1);
		mole.writeDisplay();
		randomTime[moleType] = random(2000, 4000);
		matrixState[moleType] = 2;
	}
	else
	{
		if (timer.hasPassed(300)) {
			timer.restart(); // Restart the chronometer.
			mole.clear();
			mole.drawBitmap(0, random(0,2), moleFrames[random(0, 2)], 8, 8, moleType + 1);
			mole.writeDisplay();
		}
	}
}


void moleLeave(Adafruit_BicolorMatrix & mole, int & moleHeight, int moleType, Chrono & timer, Chrono & timerTwo)
{
	if (buttonState[moleType] == HIGH)
	{
		digitalWrite(buttonLED[moleType], HIGH);
		moleHit[moleType] = true;
	}
	else
	{
		digitalWrite(buttonLED[moleType], LOW);
	}
	if (moleHit[moleType] == true)
	{
		mole.clear();
		moleHeight = 9;
		matrixState[moleType] = 0;
		moleHit[moleType] == false;
	}
	else
	{
		if (moleHeight < 0)
		{
			moleHeight++;
		}
		if (timerTwo.hasPassed(randomTime[moleType]))
		{
			//timerTwo.restart();
			if (timer.hasPassed(1000 - (moleHeight * 110))) {
				timer.restart(); // Restart the chronometer.
				if (moleHeight < 9)
				{
					mole.clear();
					mole.drawBitmap(0, moleHeight++, moleFrames[3 + moleType], 8, 8, moleType + 1);
				}
				else
				{
					randomTime[moleType] = random(10000, 15000);
					matrixState[moleType] = 3;
				}
				mole.writeDisplay();
			}
		}
		else
		{
			if (timer.hasPassed(500))
			{
				timer.restart();
				mole.clear();
				mole.drawBitmap(0, 0, moleFrames[3 + moleType], 8, 8, moleType + 1);
				mole.writeDisplay();
			}
		}
	}
	
}

void moleEscape(Chrono & timer, Chrono & timerTwo, int moleType)
{

	if (escapedMoles >= 3)
	{
		gameOver();
	}
	else if (timer.hasPassed(randomTime[moleType]))
	{
		timer.restart();
		timerTwo.restart();
		currentScore--;
		escapedMoles++;
		matrixState[moleType] = 0;
	}
}

void gameOver()
{
	for (int i = 0; i < 3; i++)
	{
		matrix[i].clear();
		//matrix[i].drawBitmap
	}
}


