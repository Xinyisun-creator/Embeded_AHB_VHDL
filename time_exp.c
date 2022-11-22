/*
 *  ======== util.c ========
 */
#include <xdc/runtime/Timestamp.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "gtz.h"

#define threshold 1000

UInt32 time1, time2;

char digit; // the pressed key from users.

int freq1, freq2; // the 2 frequencies

int mag1, mag2; // the 2 magnitudes

int time_i = 0; // fake index to form a dead loop

int gtz_out_flt[8], gtz_out_fix[8]; // out put from goertzel transformation by main

int flt_en = 0, fix_en = 0; // The enables of stop time recording of float coefficient and fixed coefficient.

int        flt_sta        ,        flt_sto       ,          flt_diff          ;
//    float start time   //    float stop time  //    float time difference   //

int        fix_sta       ,       fix_sto         ,           fix_diff         ;
//    fixed start time   //    fixed stop time  //    fixed time difference   //

int sample; // the input sample wave



short coef[8] = {0x6D02, 0x68AD, 0x63FC, 0x5EE7, 0x4A70, 0x4090, 0x3290, 0x23CE}; // goertzel coefficients

// Frequency A set: 697Hz, 770Hz, 852Hz, 941Hz.
// Frequency B set: 1209Hz, 1336Hz, 1477Hz, 1633Hz.
int I_maxflt_a = 0, I_maxflt_b = 4; // The index of the max GTZ output using float coefficient.
int I_maxfix_a = 0, I_maxfix_b = 4;//  The index of the max GTZ output using fixed(Q15) coefficient.

void task1_dtmfDetect(void)
{


	mag1 = 32768; mag2 = 32768;  // the magnitudes of input wave.



	while (1)
	{
//        ____________________________________________________________________________________________________________
//       |                                                                                                            |
//       |                          The input part. User need to enter the pressed key.                               |
//       |____________________________________________________________________________________________________________|

		System_printf("\n Please press a key from the range of (0-9), (A-D) (a-d) or (*,#) \n");
		System_flush();
		scanf(" %c",&digit); // Users input the digit as pressed key

		switch(digit)
		{
		// Every pressed key will generate 2 frequencies to Goertzel Algorithm Implementation.
		// The Alpha key pressed will include upper and lower case letters. But the output frequencies are the same.

			case '0': // The pressed key is key 0
				freq1 = 941, freq2 = 1336;break;

			case '1': // The pressed key is key 1
				freq1 = 697, freq2 = 1209;break;

			case '2': // The pressed key is key 2
				freq1 = 697, freq2 = 1336;break;

			case '3': // The pressed key is key 3
				freq1 = 697, freq2 = 1477;break;

			case '4': // The pressed key is key 4
				freq1 = 770, freq2 = 1209;break;

			case '5': // The pressed key is key 5
				freq1 = 770, freq2 = 1336;break;

			case '6': // The pressed key is key 6
				freq1 = 770, freq2 = 1477;break;

			case '7': // The pressed key is key 7
				freq1 = 852, freq2 = 1209;break;

			case '8': // The pressed key is key 8
				freq1 = 852, freq2 = 1336;break;

			case '9': // The pressed key is key 9
				freq1 = 852, freq2 = 1477;break;

			case '*' : // The pressed key is key *
				freq1 = 941, freq2 = 1209;break;

			case '#' : // The pressed key is key #
				freq1 = 941, freq2 = 1477;break;

			case 'A' : // The pressed key is key A
				freq1 = 697, freq2 = 1633;break;

			case 'a' : // The pressed key is key a
				freq1 = 697, freq2 = 1633;break;

			case 'B' : // The pressed key is key B
				freq1 = 770, freq2 = 1633;break;

			case 'b' : // The pressed key is key b
				freq1 = 770, freq2 = 1633;break;

			case 'C' : //The pressed key is key C
				freq1 = 852, freq2 = 1633;break;

			case 'c' : // The pressed key is key c
				freq1 = 852, freq2 = 1633;break;

			case 'D' : // The pressed key is key D
				freq1 = 941, freq2 = 1633;break;

			case 'd' : // The pressed key is key d
				freq1 = 941, freq2 = 1633;break;

			default:
				// Any other pressed keys which are not included in the detection domain.
				// The all output magnitudes and frequencies are 0.
				mag1 = 0, mag2 = 0 ,freq1 = 0, freq2 = 0;break;
		};
//        ____________________________________________________________________________________________________________
//       |                                                                                                            |
//       |              Print the information about input and show whether the input is successfully.                 |
//       |____________________________________________________________________________________________________________|

		// ��Print information about facts, and then come back to the initial input questions to let users input again.��
		if(freq1 == 0) // users press a undefined key.
		{
			System_printf("\n User input a undefiend key. \n");
			System_flush();
			System_printf("\n The transfered magnitudes and frequencies are\n");
			System_flush();
			System_printf("\n The magnitude 1: %d, the magnitude 2: %d\n",mag1,mag2);
			System_flush();
			System_printf("\n The frequency 1: %d, the frequency 2: %d\n",freq1,freq2);
			System_flush();
			System_printf("\n Please try to enter again. \n");
			System_flush();
		}
		// ��users press a defined key, start to GTZ calculation.��.
		else
		{
			System_printf("\n The transfered magnitudes and frequencies are\n");
			System_flush();
			System_printf("\n The magnitude 1: %d, the magnitude 2: %d\n",mag1,mag2);
			System_flush();
			System_printf("\n The frequency 1: %d, the frequency 2: %d\n",freq1,freq2);
			System_flush();
			System_printf("\n Please wait for some time. \n");
			System_flush();

			// ��measuring the start time stamp before GTZ calculation.��
			flt_sta = Timestamp_get32();
			fix_sta = Timestamp_get32();
//        __________________________________________________________________________________________________________________
//       |                                                                                                                  |
//       |  Determine if the GTZ transformation is complete, and if so, determine the pressed key based on the GTZ output.  |                                                                           .                               |
//       |__________________________________________________________________________________________________________________|

//       ��Wait until the calculations of Goertzel Algorithm are finished.��
			while(1) // process automatically without any conditions
			{
				// The dead loop of the while loop will be broken since the 2 time enables were triggered.

				if(flt_en >= 1 && fix_en >= 1) // the GTZ transformation was finished.
				{
					break; // Escape from dead loop.
				}

			}

			while (flt_en >= 1 && fix_en >= 1)
			{
				// i_a: the index of A frequency set: 697Hz, 770Hz, 852Hz, 941Hz.
				// i_b: the index of B frequency set: 1209Hz, 1336Hz, 1477Hz, 1633Hz.

				// max_flt_a: the max value of Goertzel output using the float coefficients in A frequency set.
				// max_flt_b: the max value of Goertzel output using the float coefficients in B frequency set.

				// max_fix_a: the max value of Goertzel output using the fixed coefficients in A frequency set.
				// max_fix_b: the max value of Goertzel output using the fixed coefficients in B frequency set.

				// flt_diff : the processing time of floating Goertzel Algorithm.
				// fix_diff : the processing time of fixed(Q15) Goertzel Algorithm.

				int i_a, i_b;
				int max_flt_a = gtz_out_flt[0],max_flt_b = gtz_out_flt[4];
				int max_fix_a = gtz_out_fix[0],max_fix_b = gtz_out_fix[4];

				flt_diff = flt_sto - flt_sta;
				fix_diff = fix_sto - fix_sta;

				//  ��To find the max value of 2 sets of frequency, and record their index of max value.��

				for(i_a = 0; i_a <= 3;i_a++) // A frequency set: 697Hz, 770Hz, 852Hz, 941Hz.
				{
					if(max_flt_a <= gtz_out_flt[i_a]) // Determine the max value of float GTZ output and index in frequency set A.
					{
					   max_flt_a = gtz_out_flt[i_a];
					   I_maxflt_a = i_a;
					};

					if(max_fix_a <= gtz_out_fix[i_a]) // Determine the max value of fixed GTZ output and index in frequency set A.
					{
					   max_fix_a = gtz_out_fix[i_a];
					   I_maxfix_a = i_a;
					};

				};//

				for(i_b = 4; i_b <= 7;i_b++) // B frequency set: 1209Hz, 1336Hz, 1477Hz, 1633Hz.
				{
					if(max_flt_b <= gtz_out_flt[i_b]) // Determine the max value of float GTZ output and index in frequency set B.
					{
					   max_flt_b = gtz_out_flt[i_b];
					   I_maxflt_b = i_b;
					};

					if(max_fix_b <= gtz_out_fix[i_b]) // Determine the max value of fixed GTZ output and index in frequency set B.
					{
					   max_fix_b = gtz_out_fix[i_b];
					   I_maxfix_b = i_b;
					};

				};//

//              __________________________________________________________________________________________________________________
//             |                                                                                                                  |
//             |                           Find the pressed key determined by float GTZ calculation.                              |                                                                           .                               |
//             |__________________________________________________________________________________________________________________|

				// ��The max values and index were found. Based on these results, to find the pressed key determined by GTZ calculation. ��
				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//697 Hz

				if (I_maxflt_a == 0)  // frequency 1: 697Hz.
				{
				  if(I_maxflt_b == 4) //frequency 2: 1209Hz
				  {
					System_printf("\n By float calculation, The pressed key is 1. \n");
					System_flush();
					flt_en = 0;
					//break;
				  }

				  else if(I_maxflt_b == 5) //frequency 2: 1336Hz
				  {
					System_printf("\n By float calculation, The pressed key is 2. \n");
					System_flush();
					flt_en = 0;
					//break;
				  }

				  else if(I_maxflt_b == 6) // frequency 2: 1477Hz
				  {
					System_printf("\n By float calculation, The pressed key is 3. \n");
					System_flush();
					flt_en = 0;
					//break;
				  }

				  else if(I_maxflt_b == 7) // frequency 2: 1633Hz
				  {
					System_printf("\n By float calculation, The pressed key is A or a. \n");
					System_flush();
					flt_en = 0;
					//break;
				  }
				}


				// frequency 1: 770Hz
				else if (I_maxflt_a == 1)
				 {
				  if(I_maxflt_b == 4) //frequency 2: 1209Hz
				  {
				    System_printf("\n By float calculation, The pressed key is 4. \n");
					System_flush();
					flt_en = 0;
					//break;
				   }

				  else if(I_maxflt_b == 5) // frequency 2: 1336Hz
				  {
				    System_printf("\n By float calculation, The pressed key is 5. \n");
				    System_flush();
				    flt_en = 0;
				    //break;
				   }

				  else if(I_maxflt_b == 6) // frequency 2: 1477Hz
				  {
				    System_printf("\n By float calculation, The pressed key is 6. \n");
					System_flush();
					flt_en = 0;
					//break;
				  }

				  else if(I_maxflt_b == 7) // frequency 2: 1633Hz
				  {
				    System_printf("\n By float calculation, The pressed key is B or b.\n");
					System_flush();
					flt_en = 0;
					//break;
				  }
				 }



				// frequency 1: 852 Hz
				else if (I_maxflt_a == 2)
				  {
					if(I_maxflt_b == 4) //frequency 2: 1209Hz
					{
					  System_printf("\n TBy float calculation, he pressed key is 7. \n");
					  System_flush();
					  flt_en = 0;
					  //break;
					}

					else if(I_maxflt_b == 5) // frequency 2: 1336Hz
					{
					  System_printf("\n By float calculation, The pressed key is 8. \n");
					  System_flush();
					  flt_en = 0;
					  //break;
					}

					else if(I_maxflt_b == 6) // frequency 2: 1477Hz
					{
					  System_printf("\n By float calculation, The pressed key is 9. \n");
					  System_flush();
					  flt_en = 0;
					  //break;
				    }

					else if(I_maxflt_b == 7) // frequency 2: 1633Hz
					{
					  System_printf("\n By float calculation, The pressed key is C or c. \n");
					  System_flush();
					  flt_en = 0;
					  //break;
					}
				  }


				// frequency 1: 941 Hz
				else if (I_maxflt_a == 3)
					{
					   if(I_maxflt_b == 4) //frequency 2: 1209Hz
					   {
						System_printf("\n By float calculation, The pressed key is *. \n");
						System_flush();
						flt_en = 0;
						//break;
					   }

					   else if(I_maxflt_b == 5) // frequency 2: 1336Hz
						{
						 System_printf("\n By float calculation, The pressed key is 0. \n");
						 System_flush();
						 flt_en = 0;
						 //break;
						 }

					   else if(I_maxflt_b == 6) // frequency 2: 1477Hz
						{
						 System_printf("\n By float calculation, The pressed key is #. \n");
						 System_flush();
						 flt_en = 0;
						 //break;
						}

					   else if(I_maxflt_b == 7) // frequency 2: 1633Hz
						{
						 System_printf("\n By float calculation, The pressed key is D or d. \n");
						 System_flush();
						 flt_en = 0;
						 //break;
						}

					}
			//}//while


//              __________________________________________________________________________________________________________________
//             |                                                                                                                  |
//             |                           Find the pressed key determined by Q15 GTZ calculation.                                |                                                                           .                               |
//             |__________________________________________________________________________________________________________________|

			if (I_maxfix_a== 0) // frequency 1: 697Hz
			{
			  if(I_maxfix_b == 4) //frequency 2: 1209Hz
			  {
				System_printf("\n By fixed calculation, The pressed key is 1. \n");
				System_flush();
				flt_en = 0;
				//break;
			  }

			  else if(I_maxfix_b == 5) // frequency 2: 1336Hz
			  {
				System_printf("\n By fixed calculation, The pressed key is 2. \n");
				System_flush();
				flt_en = 0;
				//break;
			  }

			  else if(I_maxfix_b == 6) // frequency 2: 1477Hz
			  {
				System_printf("\n By fixed calculation, The pressed key is 3. \n");
				System_flush();
				flt_en = 0;
				//break;
			  }

			  else if(I_maxfix_b == 7) // frequency 2: 1633Hz
			  {
				System_printf("\n By fixed calculation, The pressed key is A or a. \n");
				System_flush();
				flt_en = 0;
				//break;
			  }
			}


			// frequency 1: 770Hz
			else if (I_maxfix_a == 1)
			 {
			  if(I_maxfix_b == 4) //frequency 2: 1209Hz
			  {
			    System_printf("\n By fixed calculation, The pressed key is 4. \n");
				System_flush();
				flt_en = 0;
				//break;
			   }

			  else if(I_maxfix_b == 5) // frequency 2: 1336Hz
			  {
			    System_printf("\n By fixed calculation, The pressed key is 5. \n");
			    System_flush();
			    flt_en = 0;
			    //break;
			   }

			  else if(I_maxfix_b == 6) // frequency 2: 1477Hz
			  {
			    System_printf("\n By fixed calculation, The pressed key is 6. \n");
				System_flush();
				flt_en = 0;
				//break;
			  }

			  else if(I_maxfix_b == 7) // frequency 2: 1633Hz
			  {
			    System_printf("\n By fixed calculation, The pressed key is B or b.\n");
				System_flush();
				flt_en = 0;
				//break;
			  }
			 }



			// frequency 1: 852 Hz
			else if (I_maxfix_a == 2)
			  {
				if(I_maxfix_b == 4) // frequency 2: 1209Hz
				{
				  System_printf("\n TBy fixed calculation, he pressed key is 7. \n");
				  System_flush();
				  flt_en = 0;
				}

				else if(I_maxfix_b == 5) //frequency 2: 1336Hz
				{
				  System_printf("\n By fixed calculation, The pressed key is 8. \n");
				  System_flush();
				  flt_en = 0;
				}

				else if(I_maxfix_b == 6) // frequency 2: 1477Hz
				{
				  System_printf("\n By fixed calculation, The pressed key is 9. \n");
				  System_flush();
				  flt_en = 0;
			    }

				else if(I_maxfix_b == 7) // frequency 2: 1633Hz
				{
				  System_printf("\n By fixed calculation, The pressed key is C or c. \n");
				  System_flush();
				  flt_en = 0;
				}
			  }


			// frequency 1: 941 Hz
			else if (I_maxfix_a == 3)
				{
				   if(I_maxfix_b == 4) //frequency 2: 1209Hz
				   {
					System_printf("\n By fixed calculation, The pressed key is *. \n");
					System_flush();
					flt_en = 0;
				   }

				   else if(I_maxfix_b == 5) // frequency 2: 1336Hz
					{
					 System_printf("\n By fixed calculation, The pressed key is 0. \n");
					 System_flush();
					 flt_en = 0;
					 }

				   else if(I_maxfix_b == 6) // frequency 2: 1477Hz
					{
					 System_printf("\n By fixed calculation, The pressed key is #. \n");
					 System_flush();
					 flt_en = 0;
					}

				   else if(I_maxfix_b == 7) // frequency 2: 1633Hz
					{
					 System_printf("\n By fixed calculation, The pressed key is D or d. \n");
					 System_flush();
					 flt_en = 0;
					}

				}

//              __________________________________________________________________________________________________________________
//             |                                                                                                                  |
//             |                Print out the cycles of each calculations and determine which is more efficient.                  |                                                                           .                               |
//             |__________________________________________________________________________________________________________________|
			System_printf("\n %d cycles used in float calculation.\n",flt_diff);
			System_flush();
			System_printf("\n %d cycles used in fixed calculation.\n",fix_diff);
			System_flush();

			if(flt_diff < fix_diff)
			{
				System_printf("\n Using float coefficient is more efficient. \n");
				System_flush();
				System_printf("\n \n");
				System_flush();
				break;
			}
			else if(flt_diff > fix_diff)
			{
				System_printf("\n Using fixed(Q15) coefficient is more efficient. \n");
				System_flush();
				System_printf("\n \n");
				System_flush();
				break;
			}
			else if(flt_diff == fix_diff)
			{
				System_printf("\n 2 calculating ways are equally efficient. \n");
				System_flush();
				System_printf("\n \n");
				System_flush();
				break;
			}
			break;
			}}}}

