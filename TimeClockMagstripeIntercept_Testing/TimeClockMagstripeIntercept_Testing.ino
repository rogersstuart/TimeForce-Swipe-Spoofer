#include <Wire.h>
#include <nfc.h>

//Magstripe Interceptor and Emulator

//timeforce magstripe format
// ss | data | es | lrc
//assembled as follows
//4 bits of the char lsb first + odd parity bit
//the entire sequence is inverted before transmission
//
//to convert a ascii value just subtract 48

//pin 2 - DATA
//pin 3 - CLK
//pin 4 - MD

//latch on rising edge

//magstripe conversion constants
const uint8_t start_sentinel = 0b11010;
const uint8_t end_sentinel   = 0b11111;

const uint8_t key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

NFC_Module nfc;

void setup()
{
	Serial.begin(9600);
	
	initalizePins();

	nfc.begin();
	
	nfc.SAMConfiguration();	
}

void loop()
{	
	uint8_t nfc_buffer[32], block_buffer[16], card_present;
	
	card_present = nfc.InListPassiveTarget(nfc_buffer);
	if(card_present)
	{
		for(uint8_t block_num = 4; block_num < 8; block_num++)
			if(nfc.MifareAuthentication(0, block_num, nfc_buffer+1, nfc_buffer[0], key))
			{
				if(nfc.MifareReadBlock(block_num, block_buffer))
				{
					nfc.puthex(block_buffer, 16);
					Serial.println();
				}
			}
	}
}

void initalizePins()
{
	for(uint8_t pin_counter = 5; pin_counter < 8; pin_counter++)
	{
		pinMode(pin_counter, OUTPUT);
		digitalWrite(pin_counter, HIGH);
	}
}

//accepts any positive number <= 9999
uint8_t emulateMagstripe(uint16_t to_convert)
{
	uint8_t char_rep_index = 0;
	uint8_t char_rep[4] = {0, 0, 0, 0}; //set the limit to 4 digits
	uint8_t lrc = 0;
	
	while(1)
	{
		char_rep[char_rep_index] = to_convert % 10;
		to_convert = to_convert/10;
		
		if(to_convert > 0)
			char_rep_index++;
		else
			break;
	}
	
	uint8_t lrc_counter_0 = 2, lrc_counter_1 = 2, lrc_counter_2 = 1, lrc_counter_3 = 2;
	for(uint8_t index_counter = 0; index_counter <= char_rep_index; index_counter++)
	{
		//rotate
		uint8_t char_rep_holding = char_rep[index_counter];
		char_rep[index_counter]  = (char_rep_holding & 0b1000) >> 3;
		char_rep[index_counter] |= (char_rep_holding & 0b0100) >> 1;
		char_rep[index_counter] |= (char_rep_holding & 0b0010) << 1;
		char_rep[index_counter] |= (char_rep_holding & 0b0001) << 3;
		
		//shift to the left by one to make room for the parity
		char_rep[index_counter] = char_rep[index_counter] << 1;
		
		//generate odd parity bit
		int parity_counter = 0;
		for(int bit_pos_counter = 1; bit_pos_counter < 5; bit_pos_counter++)
		if((char_rep[index_counter] >> bit_pos_counter) & 1)
		parity_counter++;
		
		if((parity_counter % 2) == 0)
		char_rep[index_counter] |= 1;
		
		//increment the lrc counters
		if((char_rep[index_counter] & 0b10000) > 0)
			lrc_counter_0++;
		if((char_rep[index_counter] & 0b1000) > 0)
			lrc_counter_1++;
		if((char_rep[index_counter] & 0b100) > 0)
			lrc_counter_2++;
		if((char_rep[index_counter] & 0b10) > 0)
			lrc_counter_3++;
	}
	
	//compute the lrc and lrc parity
	lrc |= (lrc_counter_0 % 2) > 0 ? 0b10000 : 0;
	lrc |= (lrc_counter_1 % 2) > 0 ? 0b1000 : 0;
	lrc |= (lrc_counter_2 % 2) > 0 ? 0b100 : 0;
	lrc |= (lrc_counter_3 % 2) > 0 ? 0b10 : 0;
	
	uint8_t lrc_parity_counter = 0;
	for(int bit_position_counter = 1; bit_position_counter < 5; bit_position_counter++)
	lrc_parity_counter += ((lrc >> bit_position_counter) & 1) ? 1 : 0;
	
	lrc |= (lrc_parity_counter % 2) == 0 ? 1 : 0;
	
	//invert and transmit each component
	digitalWrite(7, LOW);
	
	for(int clock_counter = 0; clock_counter < 11; clock_counter++)
		pulseClock();
	
	shiftOutByte(~start_sentinel);
	while(1)
	{
		shiftOutByte(~char_rep[char_rep_index]);
		if(char_rep_index == 0)
			break;
		else
			char_rep_index--;
	}
	shiftOutByte(~end_sentinel);
	shiftOutByte(~lrc);
	
	digitalWrite(5, HIGH);
	
	for(int clock_counter = 0; clock_counter < 11; clock_counter++)
		pulseClock();
	
	digitalWrite(7, HIGH);
	
	return 0;
}

void shiftOutByte(uint8_t to_shift)
{
	for(int clock_counter = 4; clock_counter >= 0; clock_counter--)
	{
		digitalWrite(5, (to_shift >> clock_counter) & 1);
		delayMicroseconds(60);
		
		pulseClock();
	}
}

void pulseClock()
{
	digitalWrite(6, LOW);
	delayMicroseconds(20);
	digitalWrite(6, HIGH);
	delayMicroseconds(40);
}
