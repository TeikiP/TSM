#include <math.h>

#define PIVALUE 3.141592
#define FECH 44100.0

#define AMP 1.0
#define FREQ 440
#define PHASE 0


void echantillonage () {
	int i=0;
	float temps = 0.0;
	float signal[(int)FECH];


	for (i=0; i<FECH; i++) {
		temps = (float)i/(float)FECH;
		signal[i] = AMP * sin(2.0*PIVALUE * FREQ * temps + PHASE);
	}
}
