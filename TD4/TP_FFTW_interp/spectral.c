#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sndfile.h>
#include <math.h>
#include <complex.h>
#include <fftw3.h>

#include "gnuplot_i.h"

/* taille de la fenetre */
#define	FRAME_SIZE 1024
/* avancement */
#define HOP_SIZE 1024

static gnuplot_ctrl *h;

static void
print_usage (char *progname)
{
  printf ("\nUsage : %s <input file> \n", progname) ;
}

static fftw_plan
fft_init (double complex *input, double complex *output)
{
  return fftw_plan_dft_1d(FRAME_SIZE, input, output, FFTW_FORWARD, FFTW_ESTIMATE);
}

static void
fft_exit (fftw_plan plan)
{
  fftw_destroy_plan(plan);
}

static void
fft (fftw_plan plan)
{
  fftw_execute(plan);
}

static void
fill_buffer(double *buffer, double *new_buffer)
{
  int i;
  double tmp[FRAME_SIZE-HOP_SIZE];

  /* save */
  for (i=0;i<FRAME_SIZE-HOP_SIZE;i++)
    tmp[i] = buffer[i+HOP_SIZE];

  /* save offset */
  for (i=0;i<(FRAME_SIZE-HOP_SIZE);i++)
    {
      buffer[i] = tmp[i];
    }

  for (i=0;i<HOP_SIZE;i++)
    {
      buffer[FRAME_SIZE-HOP_SIZE+i] = new_buffer[i];
    }
}

static int
read_n_samples (SNDFILE * infile, double * buffer, int channels, int n)
{

  if (channels == 1)
    {
      /* MONO */
      int readcount ;

      readcount = sf_readf_double (infile, buffer, n);

      return readcount==n;
    }
  else if (channels == 2)
    {
      /* STEREO */
      double buf [2 * n] ;
      int readcount, k ;
      readcount = sf_readf_double (infile, buf, n);
      for (k = 0 ; k < readcount ; k++)
	buffer[k] = (buf [k * 2]+buf [k * 2+1])/2.0 ;

      return readcount==n;
    }
  else
    {
      /* FORMAT ERROR */
      printf ("Channel format error.\n");
    }

  return 0;
}

static int
read_samples (SNDFILE * infile, double * buffer, int channels)
{
  return read_n_samples (infile, buffer, channels, HOP_SIZE);
}

void
dft (double* s, double complex* S)
{
  int i, j;
  for(i=0; i<FRAME_SIZE; i++) {
    S[i] = 0;

    for(j=0; j<FRAME_SIZE; j++) {
      S[i] += s[j] * cexp((-I * 2 * M_PI * i * j)/FRAME_SIZE);
    }
  }
}

void
cartesian_to_polar (double complex *spectre, double *amp, double *phase)
{
  int i;
  for (i=0; i<FRAME_SIZE; i++) {
    amp[i] = cabs(spectre[i]);
    phase[i] = carg(spectre[i]);
  }
}

int
main (int argc, char * argv [])
{	char 		*progname, *infilename;
	SNDFILE	 	*infile = NULL ;
  SF_INFO	 	sfinfo ;

	progname = strrchr (argv [0], '/') ;
	progname = progname ? progname + 1 : argv [0] ;

	if (argc != 2)
	{	print_usage (progname) ;
		return 1 ;
		} ;

	infilename = argv [1] ;

	if ((infile = sf_open (infilename, SFM_READ, &sfinfo)) == NULL)
	{	printf ("Not able to open input file %s.\n", infilename) ;
		puts (sf_strerror (NULL)) ;
		return 1 ;
		} ;

	/* Read WAV */
	int nb_frames = 0;
	double new_buffer[HOP_SIZE];
	double buffer[FRAME_SIZE];

	/* Plot Init */
	h=gnuplot_init();
	gnuplot_setstyle(h, "lines");

	int i;
	for (i=0;i<(FRAME_SIZE/HOP_SIZE-1);i++)
	  {
	    if (read_samples (infile, new_buffer, sfinfo.channels)==1)
	      fill_buffer(buffer, new_buffer);
	    else
	      {
		printf("not enough samples !!\n");
		return 1;
	      }
	  }

	/* Info file */
	printf("sample rate %d\n", sfinfo.samplerate);
	printf("channels %d\n", sfinfo.channels);
	printf("size %d\n", (int)sfinfo.frames);

	/* FFTW */
	double complex *spectre = malloc(FRAME_SIZE * sizeof(double complex));
	double complex *samples = malloc(FRAME_SIZE * sizeof(double complex));
	double *phase = malloc(FRAME_SIZE * sizeof(double));
	double *amp = malloc(FRAME_SIZE * sizeof(double));
	
	fftw_plan plan = fft_init(samples, spectre);

	while (read_samples (infile, new_buffer, sfinfo.channels)==1)
	  {
	    /* hop size */
	    fill_buffer(buffer, new_buffer);
	    
	    for (i=0; i<FRAME_SIZE; i++)
		    samples[i] = buffer[i];
		    
	    /* Hann */
	    for (i=0; i<FRAME_SIZE; i++)
		    samples[i] = buffer[i] * (.5 - .5 * cos((2*M_PI*i)/FRAME_SIZE));

      /* FFT */
	    fft(plan);
	    cartesian_to_polar(spectre, amp, phase);
		  
	    /* Process Samples */
	    int maxIndex = 0;
	    double max = 0;
	    
	    for (i=0; i<FRAME_SIZE/2; i++)
	      if (amp[i] > max) {
	        max = amp[i];
	        maxIndex = i;
	      }
	      
		  /* Interpolation */
		  double a1 = 20 * log10(amp[maxIndex-1]);
		  double ac = 20 * log10(amp[maxIndex]);
		  double ar = 20 * log10(amp[maxIndex+1]);
		  
		  double d = .5 * ((a1-ar)/(a1-2*ac+ar));
		  double m = maxIndex + d;
		  double f = m * sfinfo.samplerate/FRAME_SIZE;
		  double a = ac - (a1 - ar) * (d/4);
	    
	    /* Affichage des informations */
	    printf("Processing frame %3d (max amp of %.2fdB reached at frequency of %f)\n", nb_frames, a, f);

	    /* PLOT */
	    gnuplot_resetplot(h);
	    gnuplot_plot_x(h,amp,FRAME_SIZE/2,"temporal frame");

	    sleep(1);

	    nb_frames++;
	  }


	fft_exit(plan);

	sf_close (infile) ;

	free(samples);
	free(spectre);
	free(amp);
	free(phase);

	return 0 ;
} /* main */

