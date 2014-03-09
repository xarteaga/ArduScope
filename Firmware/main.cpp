// Definitions
#define SERIAL_BAUDRATE 115200
#define ETHERNET_PORT 80

// Includes
#include <Arduino.h>
#include <stdint.h>
#include <Ethernet.h>
#include "sha1.h"
#include "Base64.h"
#include <socket.h>

#include "wiring_private.h"
#include "pins_arduino.h"

EthernetServer server = EthernetServer(ETHERNET_PORT);
EthernetClient client;

void setup() {
	// Initialize Serial port (RS232)
	Serial.begin(SERIAL_BAUDRATE);

	// Start Ethernet server
	Ethernet.begin((uint8_t[] ) { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED },
			(uint8_t[] ) { 192, 168, 10, 2 });
	server.begin();
	
}

inline uint16_t readPin()
{
	uint8_t low, high, pin;

#if defined(__AVR_ATmega32U4__)
	pin = analogPinToChannel(9);
	ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((pin >> 3) & 0x01) << MUX5);
#elif defined(ADCSRB) && defined(MUX5)
	// the MUX5 bit of ADCSRB selects whether we're reading from channels
	// 0 to 7 (MUX5 low) or 8 to 15 (MUX5 high).
	ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((pin >> 3) & 0x01) << MUX5);
#endif

	// set the analog reference (high two bits of ADMUX) and select the
	// channel (low 4 bits).  this also sets ADLAR (left-adjust result)
	// to 0 (the default).
#if defined(ADMUX)
	ADMUX = (DEFAULT << 6) | (pin & 0x07);
#endif

	// without a delay, we seem to read from the wrong channel
	//delay(1);

#if defined(ADCSRA) && defined(ADCL)
	// start the conversion
	sbi(ADCSRA, ADSC);

	// ADSC is cleared when the conversion finishes
	while (bit_is_set(ADCSRA, ADSC));

	// we have to read ADCL first; doing so locks both ADCL
	// and ADCH until ADCH is read.  reading ADCL second would
	// cause the results of each conversion to be discarded,
	// as ADCL and ADCH would be locked when it completed.
	low  = ADCL;
	high = ADCH;
#else
	// we dont have an ADC, return 0
	low  = 0;
	high = 0;
#endif

	// combine the two bytes
	return (high << 8) | low;
}

#define HEADLEN 17
#define KEYLEN 24
void handshake() {
	char c;
	uint8_t buffer[24];
	uint8_t i;

	while (client.connected() && client.available()) {
		// Read header
		c = client.read();
		for (i = 0; i < HEADLEN && c != ':'; i++) {
			buffer[i] = c;
			c = client.read();
		}

		// Check if header is the websocket key
		if (i == 17
				&& strncmp((char*) buffer, "Sec-WebSocket-Key", HEADLEN) == 0) {
			Sha1 sha;
			client.read();
			client.read(buffer, KEYLEN);
			client.flush();
			sha.update(buffer, KEYLEN);

			uint8_t const magic[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
			sha.update(magic, sizeof(magic) - 1);

			uint8_t result[21];
			sha.finish(result);

			uint8_t b64Result[30];
			uint16_t const len = base64_encode(b64Result, result, 20);

			// Handshake
			client.pushTx("HTTP/1.1 101 Web Socket Protocol Handshake\r\n");
			client.pushTx("Upgrade: websocket\r\n");
			client.pushTx("Connection: Upgrade\r\n");
			client.pushTx("Sec-WebSocket-Accept: ");
			client.pushTx(b64Result, len);
			client.pushTx("\r\n\r\n");
//			client.pushTx("\r\nSec-WebSocket-Protocol: chat\r\n\r\n");
			client.flushTx();
			return;
		}
		while (client.connected() && client.available() && client.read() != '\n')
			;
	}

}

char message[126];
boolean readMessage() {
	uint8_t mask[4];
	uint8_t f, c, size, i;

	if (!client.available())
		return false;

	f = client.read(); // First byte 129 Always
	if (f==0x88){
		client.stop();
		return false;
	}

	size = client.read() & 0x7f;

	if (size > 125) // This code does not support messages bigger than 125 Bytes
		return false;

	for (i = 0; i < 4; i++) {
		mask[i] = client.read();
	}

	for (i = 0; i < size; i++) {
		message[i] = client.read() ^ mask[i & 0x3];
	}
	message[i] = '\0';
	//client.flush();
	return true;
}

inline void sendMessage(const char * msg, uint8_t size) {
	client.pushTx(0x81);
	client.pushTx((char) size);
	client.pushTx((uint8_t*) msg, size);
	client.flushTx();
}

int main(void) {
	uint8_t i;
	init();

#if defined(USBCON)
	USB.attach();
#endif

	setup();

	for (;;) {
		client = server.available();
		if (!client)
			continue;

		Serial.println(F("---> Client connected <---"));
		handshake();

		if (!client.connected()) {
			Serial.println(F("---> Client Disconnected after handshake <---"));
			continue;
		}
		uint8_t msg [] = "XXXX XXXX";
		uint8_t head []= {0x81,sizeof(msg)};
		uint8_t _sock = client.getSocket();
		uint16_t raw;
		uint16_t stamp;
		/*msg[0] = 0x81;
		msg[1] = sizeof(msg);*/
		while (client.connected()) {
			raw = readPin();
			stamp = micros() & 0x1FFF;
#define STAMP_PTR 0
#define VAL_PTR 5
			msg[STAMP_PTR+3] = 0x30 + stamp % 10;
			stamp /= 10;
			msg[STAMP_PTR+2] = 0x30 + stamp % 10;
			stamp /= 10;
			msg[STAMP_PTR+1] = 0x30 + stamp % 10;
			stamp /= 10;
			msg[STAMP_PTR+0] = 0x30 + stamp % 10;
			stamp /= 10;
			msg[VAL_PTR+3] = 0x30 + raw % 10;
			raw /= 10;
			msg[VAL_PTR+2] = 0x30 + raw % 10;
			raw /= 10;
			msg[VAL_PTR+1] = 0x30 + raw % 10;
			raw /= 10;
			msg[VAL_PTR+0] = 0x30 + raw % 10;
			raw /= 10;
			send(_sock, (uint8_t*)head, sizeof(head));
			send(_sock, (uint8_t*)msg, sizeof(msg));

			/*if (readMessage()) {
				Serial.print(F("Message: '"));
				Serial.print(message);
				Serial.println("'");
				delay(1000);
				sendMessage(message, strlen(message));
			}*/
		}

		Serial.println(F("---> Client Disconnected <---"));

		if (serialEventRun)
			serialEventRun();
	}

	return 0;
}

