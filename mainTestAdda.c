/**
 * @file mainTestAdda.c
 * @author Boris Bocquet <borisboc@free.fr>
 * @date May, 2018
 * @brief Testing AD-DA-WS-RPI lib
 *
 * @details Usefull to perform Analog to Digital(ADC, using ADS1256 circuit) and Digital to Analog (DAC, using DAC8552).
 * @todo More code especially for DAC8552.
 */

// This Source Code Form is subject to the terms of the
// GNU Lesser General Public License V 3.0.
// If a copy of the GNU GPL was not distributed
// with this file, You can obtain one at https://www.gnu.org/licenses/lgpl-3.0.en.html

#include "AD-DA-WS-RPI/AD-DA-WS-RPI.h"

/**
 * @brief Simple code to get the voltage on each 8 inputs (single ended) using the ADC1256 and write the input 1 value to output 1 using DAC8552.
 * You can use the jumper of the waveshare board to connect to 5V (power supply and vref), and connect ADO to ADJ (the potentiometer) and DAC to LEDA.
 * 
 * @return int 
 */
int main(void)
{
	// Force stdout line buffered for immediate feedback
	setvbuf(stdout, NULL, _IOLBF, 0);
	printf("Starting Test ADDA\r\n");

	int initSpi = spi_init();
	if (initSpi != 1)
	{
		printf("SPI init failed with code %d\r\n", initSpi);
		return -55;
	}
	printf("SPI initialized\r\n");

	int NbChannels = 8;
	int MainLoop = 0;
	int RetCode = 0;
	uint8_t *Channels = malloc(NbChannels * sizeof(uint8_t));
	if (!Channels)
	{
		printf("Malloc failed for Channels\r\n");
		return -56;
	}
	uint8_t Ch;
	for (Ch = 0; Ch < NbChannels; Ch++)
	{
		Channels[Ch] = Ch;
	}
	printf("Channels array ready\r\n");

	while (1)
	{
		printf("[Loop %d] Calling ADC_DAC_Init...\r\n", MainLoop);
		int Id = 0;
		int InitCode = ADC_DAC_Init(&Id, ADS1256_GAIN_1, ADS1256_100SPS);
		if (InitCode != 0)
		{
			printf("ADC_DAC_Init failed, code=%d\r\n", InitCode);
			RetCode = -1;
			break;
		}
		printf("ADC_DAC_Init ok, chip ID=%d\r\n", Id);

		// Zero DAC outputs AFTER init (CS lines configured)
		DAC8552_Write(channel_A, Voltage_Convert(5.0, 0));
		DAC8552_Write(channel_B, Voltage_Convert(5.0, 0));

		int Loop;
		for (Loop = 0; Loop < 10; Loop++)
		{
			int32_t *AdcValues = NULL;
			int rv = ADS1256_ReadAdcValues(&Channels, NbChannels, SINGLE_ENDED_INPUTS_8, &AdcValues);
			if (rv != 0)
			{
				printf("ADS1256_ReadAdcValues failed (%d)\r\n", rv);
				RetCode = -3;
				break;
			}
			double *volt = ADS1256_AdcArrayToMicroVolts(AdcValues, NbChannels, 1.0 / 1000000.0);
			printf("Sample %d: 0:%f 1:%f 2:%f 3:%f 4:%f 5:%f 6:%f 7:%f V\r\n",
				   Loop, volt[0], volt[1], volt[2], volt[3], volt[4], volt[5], volt[6], volt[7]);
			DAC8552_Write(channel_A, Voltage_Convert(5.0, volt[0]));
			free(AdcValues);
			free(volt);
			bsp_DelayUS(1000);
		}

		printf("Closing ADC/DAC...\r\n");
		int CloseCode = ADC_DAC_Close();
		if (CloseCode != 0)
		{
			printf("ADC_DAC_Close failed (%d)\r\n", CloseCode);
			RetCode = -2;
			break;
		}

		MainLoop++;
		if (MainLoop >= 1) // only one loop for initial debug
			break;
	}

	printf("Test ADDA finished with returned code %d\r\n", RetCode);
	free(Channels);
	return RetCode;
}
