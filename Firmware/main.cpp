// Definitions
#define SERIAL_BAUDRATE 115200
#define ETHERNET_PORT 80
#define ANALOG_PIN 0
#define DATA_LEN 9
//#define LISTEN_MESSAGES /* Uncomment this line for allow receive messages */

// Includes
#include <Arduino.h>
#include <stdint.h>
#include <Ethernet.h>
#include "sha1.h"
#include "Base64.h"
#include <socket.h>
#include <USBAPI.h>

#include <wiring_private.h>
#include <pins_arduino.h>

EthernetServer server = EthernetServer(ETHERNET_PORT);
EthernetClient client;
char messageBuffer[126];

void setup() {
	uint8_t ip []= {192, 168, 10, 2};
	uint8_t mac [] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

	// Initialize Serial port (RS232)
	Serial.begin(SERIAL_BAUDRATE);
	Serial.println("Starting WebSocket server...");

	// Start Ethernet server
	Ethernet.begin(mac, ip);
	server.begin();

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
		Serial.print(c);
		for (i = 0; i < HEADLEN && c != ':'; i++) {
			buffer[i] = c;
			c = client.read();
			Serial.print(c);
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

boolean readMessage() {
	uint8_t mask[4];
	uint8_t f, c, size, i;

	if (!client.available())
		return false;

	f = client.read(); // First byte 129 Always
	if (f == 0x88) {
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
		messageBuffer[i] = client.read() ^ mask[i & 0x3];
	}
	messageBuffer[i] = '\0';
	//client.flush();
	return true;
}

inline void sendMessage(const char * msg, uint8_t size) {
	client.pushTx(0x81);
	client.pushTx((char) size);
	client.pushTx((uint8_t*) msg, size);
	client.flushTx();
}

void stream() {
	uint8_t s = client.status(); // TCP Client status

	uint8_t msg[DATA_LEN + 1] = "STAM RAWV"; // Message with the time stamp (12 bit, from 0 to 8191) and the raw value (V)
	uint8_t head[] = { 0x81, DATA_LEN }; // Message Head with the opening byte and message length
	uint8_t _sock = client.getSocket();		// Socket (Faster without stack)

	uint8_t low, high;
	uint16_t raw, stamp;

	// Set ADC MUX to the desired pin
	ADMUX = (DEFAULT << 6) | (ANALOG_PIN & 0x07);

	// While client is connected stream reads
	while (!(s == SnSR::LISTEN || s == SnSR::CLOSED || s == SnSR::FIN_WAIT)) {
		// Start conversion; While the Micro is converting, it gets the time stamp
		sbi(ADCSRA, ADSC);

		// Get time stamp
		stamp = micros() & 0x1FFF;

		// Build message time stamp
		msg[3] = 0x30 + stamp % 10;
		stamp /= 10;
		msg[2] = 0x30 + stamp % 10;
		stamp /= 10;
		msg[1] = 0x30 + stamp % 10;
		stamp /= 10;
		msg[0] = 0x30 + stamp;

		// Send head
		send(_sock, head, 2);

		// Wait until the conversion finishes
		while (bit_is_set(ADCSRA, ADSC))
			Serial.println(F("Warning! RT Error!"));

		low = ADCL;
		high = ADCH;

		// combine the two bytes
		raw = (high << 8) | low;

		// Build message value
		msg[8] = 0x30 + raw % 10;
		raw /= 10;
		msg[7] = 0x30 + raw % 10;
		raw /= 10;
		msg[6] = 0x30 + raw % 10;
		raw /= 10;
		msg[5] = 0x30 + raw;

		// Send message
		send(_sock, (uint8_t*) msg, DATA_LEN);

#ifdef LISTEN_MESSAGES
		if (readMessage()) {
			Serial.print(F("Message: '"));
			Serial.print(messageBuffer);
			Serial.println("'");
			sendMessage(messageBuffer, strlen(messageBuffer));
		}
#endif // LISTEN_MESSAGES

		// Update status
		s = client.status();
	}
}

int main(void) {
	// Initialize Arduino API
	init();

#if defined(USBCON)
	USB.attach();
#endif

	// Call setup
	setup();

	// Forever
	while (true) {
		delay(1000);
		Serial.println("ECHO!");

		client = server.available();
		if (!client.connected())
			continue;

		// If somebody has connected, handshake
		Serial.println(F("---> Client connected <---"));
		handshake();

		// If the client remains connected do not continue
		if (!client.connected()) {
			Serial.println(F("---> Client Disconnected after handshake <---"));
			continue;
		}

		// While the client is connected listen messages
		while (client.connected()) {
			// If message, print it
			if (readMessage()) {
				Serial.print(F("Message Received: '"));
				Serial.print(messageBuffer);
				Serial.println("'");
				//sendMessage(messageBuffer, strlen(messageBuffer)); // Uncomment for enable echo

				// If "START" detected, start stream
				if (strcmp(messageBuffer, "START") == 0) {
					stream();
				}
			}
		}

		Serial.println(F("---> Client Disconnected <---"));

		if (serialEventRun)
			serialEventRun();
	}

	return 0;
}

