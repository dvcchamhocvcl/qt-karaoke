#include "Yin.h"
#include <math.h>
/* ------------------------------------------------------------------------------------------
--------------------------------------------------------------------------- PRIVATE FUNCTIONS
-------------------------------------------------------------------------------------------*/

/**
 * Step 1: Calculates the squared difference of the signal with a shifted version of itself.
 * @param buffer Buffer of samples to process. 
 *
 * This is the Yin algorithms tweak on autocorellation. Read http://audition.ens.fr/adc/pdf/2002_JASA_YIN.pdf
 * for more details on what is in here and why it's done this way.
 */
void Yin_difference(Yin *yin, uint8_t* buffer){
  const uint8_t* bufferIEnd = buffer + YIN_HALF_BUFFER_SIZE;
	/* Calculate the difference for difference shift values (tau) for the half of the samples */
    for(unsigned char tau = 0; tau < YIN_HALF_BUFFER_SIZE-1; tau++){

		/* Take the difference of the signal with a shifted version of itself, then square it.
		 * (This is the Yin algorithm's tweak on autocorellation) */ 
    uint32_t result = 0;
    uint8_t* bufferI = buffer;
    // not skipping anything - we want accuracy. See Video for details
    for (uint8_t* bufferI = buffer; bufferI<bufferIEnd; bufferI+=4) {
        result += pow(((int16_t)(*bufferI) - (int16_t)bufferI[tau]),2);
    } 
  	yin->yinBuffer[tau] = result >> 6;
	}
}


/**
 * Step 2: Calculate the cumulative mean on the normalised difference calculated in step 1
 * @param yin #Yin structure with information about the signal
 *
 * This goes through the Yin autocorellation values and finds out roughly where shift is which 
 * produced the smallest difference
 */
void Yin_cumulativeMeanNormalizedDifference(Yin *yin){
	uint32_t runningSum = 0;
	yin->yinBuffer[0] = 1;

	/* Sum all the values in the autocorellation buffer and nomalise the result, replacing
	 * the value in the autocorellation buffer with a cumulative mean of the normalised difference */
    for (unsigned char tau = 1; tau < YIN_HALF_BUFFER_SIZE-1; tau++) {
		runningSum += yin->yinBuffer[tau];
		yin->yinBuffer[tau] = (((uint32_t)yin->yinBuffer[tau] * tau) << 10 ) / runningSum;
	}
}

/**
 * Step 3: Search through the normalised cumulative mean array and find values that are over the threshold
 * @return Shift (tau) which caused the best approximate autocorellation. -1 if no suitable value is found over the threshold.
 */
int16_t Yin_absoluteThreshold(Yin *yin){
	unsigned char tau;

	/* Search through the array of cumulative mean values, and look for ones that are over the threshold 
	 * The first two positions in yinBuffer are always so start at the third (index 2) */
    for (tau = 2; tau < YIN_HALF_BUFFER_SIZE-1; tau++) {
		if (yin->yinBuffer[tau] < yin->threshold) {
            while (tau + 1 < YIN_HALF_BUFFER_SIZE && yin->yinBuffer[tau + 1] < yin->yinBuffer[tau]) {
				tau++;
			}
			break;
		}
	}

	/* if no pitch found */
    if (tau >= YIN_HALF_BUFFER_SIZE-1 || yin->yinBuffer[tau] >= yin->threshold) {
    return -1;
	}

	return tau;
}

/**
 * Step 5: Interpolate the shift value (tau) to improve the pitch estimate.
 * @param  yin         [description]
 * @param  tauEstimate [description]
 * @return             [description]
 *
 * The 'best' shift value for autocorellation is most likely not an interger shift of the signal.
 * As we only autocorellated using integer shifts we should check that there isn't a better fractional 
 * shift value.
 */
float Yin_parabolicInterpolation(Yin *yin, int16_t tauEstimate) {
	float betterTau;
	int16_t x0;
	int16_t x2;
	
	/* Calculate the first polynomial coeffcient based on the current estimate of tau */
	if (tauEstimate < 1) {
		x0 = tauEstimate;
	} 
	else {
		x0 = tauEstimate - 1;
	}

	/* Calculate the second polynomial coeffcient based on the current estimate of tau */
    if (tauEstimate + 1 < YIN_HALF_BUFFER_SIZE) {
		x2 = tauEstimate + 1;
	} 
	else {
		x2 = tauEstimate;
	}

	/* Algorithm to parabolically interpolate the shift value tau to find a better estimate */
	if (x0 == tauEstimate) {
		if (yin->yinBuffer[tauEstimate] <= yin->yinBuffer[x2]) {
			betterTau = tauEstimate;
		} 
		else {
			betterTau = x2;
		}
	} 
	else if (x2 == tauEstimate) {
		if (yin->yinBuffer[tauEstimate] <= yin->yinBuffer[x0]) {
			betterTau = tauEstimate;
		} 
		else {
			betterTau = x0;
		}
	} 
	else {
		float s0, s1, s2;
		s0 = yin->yinBuffer[x0];
		s1 = yin->yinBuffer[tauEstimate];
		s2 = yin->yinBuffer[x2];
		// fixed AUBIO implementation, thanks to Karl Helgason:
		// (2.0f * s1 - s2 - s0) was incorrectly multiplied with -1
		betterTau = tauEstimate + (s2 - s0) / (2 * (2 * s1 - s2 - s0));
	}


	return betterTau;
}





/* ------------------------------------------------------------------------------------------
---------------------------------------------------------------------------- PUBLIC FUNCTIONS
-------------------------------------------------------------------------------------------*/



/**
 * Initialise the Yin pitch detection object
 * @param yin        Yin pitch detection object to initialise
 * @param threshold  Allowed uncertainty (e.g 0.05 will return a pitch with ~95% probability)
 */     
void Yin_init(Yin *yin, float threshold){
	/* Initialise the fields of the Yin structure passed in */
	yin->threshold = threshold * 1024;
}

/**
 * Runs the Yin pitch detection algortihm
 * @param  yin    Initialised Yin object
 * @param  buffer Buffer of samples to analyse
 * @return        Fundamental frequency of the signal in Hz. Returns -1 if pitch can't be found
 */
float Yin_getPitch(Yin *yin, uint8_t* buffer){
	/* Step 1: Calculates the squared difference of the signal with a shifted version of itself. */
	Yin_difference(yin, buffer); // 1000
	
	/* Step 2: Calculate the cumulative mean on the normalised difference calculated in step 1 */
	Yin_cumulativeMeanNormalizedDifference(yin); // 16

	/* Step 3: Search through the normalised cumulative mean array and find values that are over the threshold */
	int16_t tauEstimate = Yin_absoluteThreshold(yin); // ~1

	/* Step 5: Interpolate the shift value (tau) to improve the pitch estimate. */
	if(tauEstimate != -1) return YIN_SAMPLING_RATE / Yin_parabolicInterpolation(yin, tauEstimate);
	
	return -1;
}
