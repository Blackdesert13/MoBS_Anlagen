#include <Servo.h>
#include <Wire.h>
struct Servo_t
{
	Servo servo;
	bool manuell = false;
	int winkelStellung1 = 135; //größerer Winkel
	int winkelStellung0 = 45; // kleinerer Winkel
	int letzteZeit = 0;
	int warteZeit = 0;
	int stellung;
	int zielstellung = 45;
};
Servo_t servos[16];

const byte ArdNr = 1;//ArduinoNummer
//Rückmeldung
unsigned long TimeNextRMAbfrage = 0;//
long TimeIntervallRMAbfrage = 3000;
//Rückmeldung Slave
int SlaveAktivListe = 0;
long RMintervalSlaveAbfrage = 2000;//Interval zum abfragen der Rückmelder vom Slave
unsigned long RMZeitSlaveAbfrage = 0;//nächste RM-Aktualisierung
//USB
int ByteZaehlerUSBEingang = 0;
int USBDatenEingangByteZahler;
byte USBBefehlEingang[5];
//Ausgänge
byte outPinStart[6];


// the setup function runs once when you press reset or power the board
void setup() {
	Wire.begin();
	Serial.begin(9600);

	for (int i = 2; i < 18; i++) { pinMode(i, OUTPUT); digitalWrite(i, HIGH); }
	for (int i = 22; i < 54; i++) { pinMode(i, OUTPUT); digitalWrite(i, HIGH); }
	outPinStart[0] = 2;//festlegen des Output-Pins für Adresse0 LOWByte und Bit0
	outPinStart[1] = 10;//festlegen des Output-Pins für Adresse0 HIGHByte und Bit0
	outPinStart[2] = 22;//festlegen des Output-Pins für Adresse1 LOWByte und Bit0
	outPinStart[3] = 30;//festlegen des Output-Pins für Adresse1 HIGHByte und Bit0
	outPinStart[4] = 38;//festlegen des Output-Pins für Adresse2 LOWByte und Bit0
	outPinStart[5] = 46;//festlegen des Output-Pins für Adresse2 HIGHByte und Bit0
	Definition();
}

// the loop function runs over and over again until power down or reset
void loop() {
	USBDatenEmpfang();
	ServoBewegung();
	//Rueckmeldung();
	// RMvonSlaves();
	// RMvonSlavesNeu();
	// SlaveAktivAbfrage();
}

void ServoBewegung() {
	for (int i = 0; i < 16; i++) {
		if (servos[i].stellung != servos[i].zielstellung) {
			//Serial.write(i);
			if (servos[i].warteZeit != 0) {
				int zeit = millis();
				if (zeit - servos[i].letzteZeit >= servos[i].warteZeit) {
					servos[i].letzteZeit = zeit;
					if (servos[i].zielstellung > servos[i].stellung) {
						servos[i].stellung++;
					}
					else {
						servos[i].stellung--;
					}
				}
			}
			else {
				servos[i].stellung = servos[i].zielstellung;
			}
			servos[i].servo.write(servos[i].stellung);
		}
	}
}

void USBDatenEmpfang()
{
	while (Serial.available() > 0)
	{//wenn vom PC Daten gesendet wurden
		if (USBDatenEingangByteZahler < 5)
		{
			USBBefehlEingang[USBDatenEingangByteZahler] = Serial.read();
			Serial.write(USBBefehlEingang[USBDatenEingangByteZahler]);
		}
		USBDatenEingangByteZahler++;

		if (USBDatenEingangByteZahler > 4)
		{
			//for(int i=0 ; i<8 ; i++ ){digitalWrite( i+30, bitRead(BefehlInput[3],i) );}
			if (controlBefehl(USBBefehlEingang))//wenn die Prüfsumme stimmt
			{
				//Serial.write(USBBefehlEingang, 5);
				if (USBBefehlEingang[0] == 1)
				{
					BefehlsAusfuerung(USBBefehlEingang);
					//USBBefehlEingang[0] = 2;
					//USBBefehlEingang[4]++;
					// Serial.write(12);
				}
				if (USBBefehlEingang[0] > 1)
				{//Befehl an EINEN Slave-Arduino
					//Serial.write(2);
					Wire.beginTransmission(USBBefehlEingang[0]);
					Wire.write(USBBefehlEingang, 5);
					Wire.endTransmission();
				}
				if (USBBefehlEingang[0] == 0)
				{//Befehl an alle Arduinos
				 // es mussen noch die Kontroll-Bytes korrigiert werden
					USBBefehlEingang[0] = 1;
					BefehlsAusfuerung(USBBefehlEingang);
					for (int i = 0; i<16; i++)
					{
						if (bitRead(SlaveAktivListe, i))
						{
							USBBefehlEingang[0] = i + 2;
							Wire.beginTransmission(USBBefehlEingang[0]);
							Wire.write(USBBefehlEingang, 5);
							Wire.endTransmission();
						}
					}
				}
				USBDatenEingangByteZahler = 0;
			}
			else {
				USBBefehlEingang[0] = USBBefehlEingang[1];
				USBBefehlEingang[1] = USBBefehlEingang[2];
				USBBefehlEingang[2] = USBBefehlEingang[3];
				USBBefehlEingang[3] = USBBefehlEingang[4];
				USBDatenEingangByteZahler = 4;
			}
		}
	}
}

boolean controlBefehl(byte Befehl[5]) {
	//kontrolliert die Prüfsumme
	byte cb = Befehl[0] + Befehl[1] + Befehl[2] + Befehl[3];
	return cb == Befehl[4];
}

void BefehlsAusfuerung(byte Befehl[5])
{
	// int a = 0;
	switch (Befehl[1])
	{
	case 1:
	{//Slave abfrage einfügen
		//SlaveAktivAbfrage();
		//Serial.write(98);
		break;
	}
	case 2:
	{
		//SlaveAktivListe = 0;
		break;
	}
	case 9:
	{
		/* Wire.requestFrom(3, 10);    // request 6 bytes from slave device #2

		while(Wire.available())    // slave may send less than requested
		{
		byte b = Wire.read(); // receive a byte as character
		Serial.write(b);         // print the character
		}*/

		break;
	}
	/*case 3:
	{
	intervalImpuls = BefehlInput[2] * 100;
	break;
	}

	case 20://permanentOutput
	{
	//BefehlZurueckSenden();
	//a = 30 + (8*BefehlInput[2]);
	//for(int i=0 ; i<8 ; i++ ){digitalWrite( i+a, bitRead(BefehlInput[3],i) );}
	ByteToRelais(BefehlInput[3],outPinStart[BefehlInput[2]]);
	break;
	}
	case 30:
	{
	a = 6 + (8*BefehlInput[2]);
	for(int i=0 ; i<8 ; i++ ){digitalWrite( i+a, bitRead(BefehlInput[3],i) );}
	timeEnd[BefehlInput[2]] = intervalImpuls + millis();
	a = 3 + BefehlInput[2];
	digitalWrite(a,LOW);
	break;
	}*/
	case 40:
	{//PermanentOutput 16Bit auf Adresse0
		ByteToServo(Befehl[2], outPinStart[0]);
		ByteToServo(Befehl[3], outPinStart[1]);
		break;
	}case 41:
	{//PermanentOutput 16Bit auf Adresse1
		ByteToRelais(Befehl[2], outPinStart[2]);
		ByteToRelais(Befehl[3], outPinStart[3]);
		break;
	}
	case 42:
	{//PermanentOutput 16Bit auf Adresse2
		ByteToRelais(Befehl[2], outPinStart[4]);
		ByteToRelais(Befehl[3], outPinStart[5]);
		break;
	}
	case 70:
		switch (Befehl[2] >> 6)	{
		case 0:
			if (Befehl[3] & 0b00000001) {
				servos[Befehl[2] & 0b00111111].manuell = true;
			}
			else {
				servos[Befehl[2] & 0b00111111].manuell = false;
			}
			break;
		case 1:
			servos[Befehl[2] & 0b00111111].warteZeit = Befehl[3];
			break;
		case 2:
			break;
		case 3:
			break;
		default:
			break;
		}
		break;
	case 71:
		if ((Befehl[2] & 0b10000000) != 0) {
			servos[Befehl[2] & 0b00111111].winkelStellung1 = Befehl[3];
		}
		else {
			servos[Befehl[2] & 0b00111111].winkelStellung0 = Befehl[3];
		}
		break;
	case 72:

		switch (Befehl[2] >> 6) {
		case 0:
			//Servo Winkel manuell einstellen
			ServoManuellSteuern(Befehl[2] & 0b00111111, Befehl[3]);
			break;
		case 1:
			break;
		case 2:
			byte servoPin = (byte)(Befehl[2] & 0b00111111);
			int winkel = servos[Befehl[2] & 0b00111111].stellung;
			
			winkel += (char)Befehl[3];
			Serial.write(winkel);
			ServoManuellSteuern(servoPin, winkel);
			break;

		}
		break;
	default:
		break;
	}
}

void ByteToRelais(byte datenByte, int startPin) {
	datenByte = ~datenByte;//datenByte muß invertiert werden weil die Relaisplatinen low-aktiv sind!
	for (int i = 0; i < 8; i++) {
		digitalWrite(i + startPin, bitRead(datenByte, i));
	}
}

void ByteToServo(byte datenByte, int startPin) {
	startPin -= 2;
	int j = 0;
	for (int i = 0; i < 8; i++) {
		j = startPin + i;
		if (servos[j].manuell)
			continue; //Beendet aktuellen Schleifendurchlauf, Sprung in nächste Iteration
		if (bitRead(datenByte, i)) {
			servos[j].zielstellung = servos[j].winkelStellung1;
		}
		else {
			servos[j].zielstellung = servos[j].winkelStellung0;
		}
	}
}

// pin = Platinen-Nr.*16 + Bit-Nr.(1.Byte 0-7, 2.Byte 8-15)
void ServoManuellSteuern(byte pin, int winkel) {
	if (pin < 16) {
		if (servos[pin].manuell) {
			servos[pin].zielstellung = constrain(winkel, servos[pin].winkelStellung0, servos[pin].winkelStellung1);
		}
	}
}

void Definition() {
	servos[3].winkelStellung0 = 85;
	servos[3].winkelStellung1 = 110;
	servos[4].winkelStellung0 = 120;
	servos[4].winkelStellung1 = 85;
	servos[5].winkelStellung0 = 65;
	servos[5].winkelStellung1 = 100;
	servos[10].winkelStellung0 = 108;
	servos[10].winkelStellung1 = 78;
	servos[6].winkelStellung1 = 105;
	servos[6].winkelStellung0 = 70;
	servos[8].winkelStellung0 = 80;
	servos[8].winkelStellung1 = 108;
	servos[9].winkelStellung1 = 96;
	servos[9].winkelStellung0 = 60;
	servos[7].winkelStellung1 = 105;
	servos[7].winkelStellung0 = 70;
	servos[2].winkelStellung1 = 90;
	servos[2].winkelStellung0 = 60;
	servos[0].winkelStellung1 = 75;
	servos[0].winkelStellung0 = 110;
	servos[1].winkelStellung1 = 110;
	servos[1].winkelStellung0 = 70;
	servos[15].winkelStellung1 = 140;
	servos[15].winkelStellung0 = 30;
	servos[14].winkelStellung1 = 160;
	servos[14].winkelStellung0 = 40;

	servos[14].warteZeit = 30;
	servos[15].warteZeit = 30;

	servos[13].manuell = true;
	servos[13].winkelStellung1 = 130;
	servos[13].winkelStellung0 = 60;
	servos[13].warteZeit = 40;

	servos[12].manuell = true;
	servos[12].winkelStellung1 = 130;
	servos[12].winkelStellung0 = 60;
	servos[12].warteZeit = 40;

	for (int i = 0; i < 16; i++) {
		//servos[i].servo.attach(i + 2, 1000, 2000);
		servos[i].servo.attach(i + 2);
		//ByteToServo(0, 2);
		//ByteToServo(0, 10);
		//delay(500);
	}
	ByteToServo(0, 2);
	ByteToServo(0, 10);

	//delay(2000);

	//servos[12].servo.attach(12 + 2);
	//servos[13].servo.attach(13 + 2);
	//servos[14].servo.attach(14 + 2);
	//servos[15].servo.attach(15 + 2);
}
