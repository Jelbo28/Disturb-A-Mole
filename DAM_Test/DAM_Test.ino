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
boolean moleHit[3] = { false,false,false };
boolean moleHittable[3] = { false,false,false };
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
	//// Solid color
	//{
	//		  B11111111,
	//		  B11111111,
	//		  B11111111,
	//		  B11111111,
	//		  B11111111,
	//		  B11111111,
	//		  B11111111,
	//		  B11111111
	//  }
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
		buttonState[matrixType[i]] = digitalRead(buttonInput[matrixType[i]]);
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
	if (buttonState[mole] == HIGH && moleHittable[mole])
	{
		digitalWrite(buttonLED[mole], HIGH);
		moleHit[mole] = true;
	}
	else
	{
		digitalWrite(buttonLED[mole], LOW);
	}
	if (moleHit[mole] == true)
	{
		//matrix[mole].clear();
		currentScore++;
		matrixHeight[mole] = 0;
		matrixState[mole] = 4;
		randomTime[mole] = random(5000, 8000);
	}
	//int moleChange = 0;
	//if (moleChange >= 2)
	//{
	//	moleChange = 0;
	//}
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
	case 4:
		moleDisturbed(matrix[mole], matrixHeight[mole], matrixType[mole], matrixTimer[mole], matrixTimerTwo[mole]);
		break;
	case 5:
		gameOver();
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
			mole.drawBitmap(0, random(0, 2), moleFrames[random(0, 2)], 8, 8, moleType + 1);
			mole.writeDisplay();
		}
	}
}


void moleLeave(Adafruit_BicolorMatrix & mole, int & moleHeight, int moleType, Chrono & timer, Chrono & timerTwo)
{
	moleHittable[moleType] = true;
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
				moleHittable[moleType] = false;
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

void moleEscape(Chrono & timer, Chrono & timerTwo, int moleType)
{

	if (escapedMoles >= 2)
	{
		matrixState[0] = 5;
		matrixState[1] = 5;
			matrixState[2] = 5;

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

void moleDisturbed(Adafruit_BicolorMatrix & mole, int & moleHeight, int moleType, Chrono & timer, Chrono & timerTwo)
{
	if (timer.hasPassed(randomTime[moleType]))
	{
		timer.restart();
		moleHeight = 9;
		//if (moleHit[0] == true && moleHit[1] == true || moleHit[2] == true && moleHit[1] == true || moleHit[0] == true && moleHit[2] == true)
		//{
		//	int moleTypeA = -1;
		//	int moleTypeB = -1;
		//	for (int i = 0; i < 3; i++)
		//	{
		//		if (moleHit[i] && moleTypeA == -1)
		//		{
		//			moleTypeA = matrixType[i];
		//			moleHit[i] = false;
		//		}
		//		if (moleHit[i] && moleTypeB == -1)
		//		{
		//			moleTypeB = matrixType[i];
		//			moleHit[i] = false;

		//		}
		//		if (moleTypeA != -1 && moleTypeB != -1)
		//		{
		//			int storage = 0;
		//			storage = matrixType[moleTypeB];
		//			matrixType[moleTypeB] = matrixType[moleTypeA];
		//			buttonInput[moleTypeA] = storage;
		//			storage = buttonInput[moleTypeA];
		//			buttonInput[moleTypeA] = buttonInput[moleTypeB];
		//			buttonInput[moleTypeB] = storage;
		//			int storage = 0;

		//			storage = moleTypeA;
		//			moleTypeA = -1;
		//			moleTypeB = -1;
		//		}

		//	}
	/*		if (moleTypeA != -1 && moleTypeB != -1)
			{
				matrixType[moleType] = 

			}*/
		//}
		moleHit[moleType] = false;
		matrixState[moleType] = 0;
	}
	else
	{
		mole.clear();
		//switch (moleType)
		//{
		//case 0:
		//	mole.drawBitmap(0, moleHeight, moleFrames[11], 8, 8, moleType + 1);
		//	break;
		//case 1:
		//	break;
		//case 2:
		//	break;
		//default:
		//	break;
		//}
		mole.drawBitmap(0, moleHeight, moleFrames[10], 8, 8, moleType + 1);
		mole.writeDisplay();
	}
}

void gameOver()
{
	matrix[0].clear();
	matrix[1].clear();

	matrix[2].clear();

	matrix[0].drawChar(0,0,currentScore,1,0,2);
	matrix[1].drawBitmap(0, 0, moleFrames[11], 8, 8, 0);
	matrix[2].drawBitmap(0, 0, moleFrames[11], 8, 8, 0);
	matrix[0].writeDisplay();
	matrix[1].writeDisplay();
	matrix[2].writeDisplay();


	//for (int i = 0; i < 3; i++)
	//{
	//	matrix[i].clear();
	//	matrix[i].drawBitmap
	//}
}


