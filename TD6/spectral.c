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
#define FRAME_SIZE 3528
/* avancement */
#define HOP_SIZE 3528

static gnuplot_ctrl* h;

static char keys[100];
static int keysPressed;

static void keyRegister(double freq[2], int samplerate) {
  const double freqStar[2] = {941, 1209};
  const double freqPound[2] = {941, 1477};
  const double freqNb[10][2] = {
      {941, 1336}, {697, 1209}, {697, 1336}, {697, 1477}, {770, 1209},
      {770, 1336}, {770, 1477}, {852, 1209}, {852, 1336}, {852, 1477}};

  const char charStar = '*';
  const char charPound = '#';
  const char charNb[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

  double tauxErreur =
      (double)samplerate / (double)FRAME_SIZE;  // taux d'erreur frequentiel

  if (freq[0] > freqStar[0] - tauxErreur &&
      freq[0] < freqStar[0] + tauxErreur &&
      freq[1] > freqStar[1] - tauxErreur &&
      freq[1] < freqStar[1] + tauxErreur) {
    keys[keysPressed] = charStar;
    keysPressed++;
  }

  else if (freq[0] > freqPound[0] - tauxErreur &&
           freq[0] < freqPound[0] + tauxErreur &&
           freq[1] > freqPound[1] - tauxErreur &&
           freq[1] < freqPound[1] + tauxErreur) {
    keys[keysPressed] = charPound;
    keysPressed++;
  }

  else {
    for (int i = 0; i < 10; i++) {
      if (freq[0] > freqNb[i][0] - tauxErreur &&
          freq[0] < freqNb[i][0] + tauxErreur &&
          freq[1] > freqNb[i][1] - tauxErreur &&
          freq[1] < freqNb[i][1] + tauxErreur) {
        keys[keysPressed] = charNb[i];
        keysPressed++;
        break;
      }
    }
  }
}

static void printKeys() {
  printf("Phone number: ");

  for (int i = 0; i < keysPressed; i++) {
    if (i % 2 == 0)
      printf("%c", keys[i]);
      
    else
      printf("%c ", keys[i]);
  }

  printf("\n");
}

static double getEnergy(double* amp) {
  double energy = 0;

  for (int i = 0; i < FRAME_SIZE; i++)
    energy += amp[i] * amp[i];

  energy /= FRAME_SIZE;

  return energy;
}

static void print_usage(char* progname) {
  printf("\nUsage : %s <input file> \n", progname);
}

static fftw_plan fft_init(double complex* input, double complex* output) {
  return fftw_plan_dft_1d(FRAME_SIZE, input, output, FFTW_FORWARD,
                          FFTW_ESTIMATE);
}

static void fft_exit(fftw_plan plan) {
  fftw_destroy_plan(plan);
}

static void fft(fftw_plan plan) {
  fftw_execute(plan);
}

static void fill_buffer(double* buffer, double* new_buffer) {
  int i;
  double tmp[FRAME_SIZE - HOP_SIZE];

  /* save */
  for (i = 0; i < FRAME_SIZE - HOP_SIZE; i++)
    tmp[i] = buffer[i + HOP_SIZE];

  /* save offset */
  for (i = 0; i < (FRAME_SIZE - HOP_SIZE); i++) {
    buffer[i] = tmp[i];
  }

  for (i = 0; i < HOP_SIZE; i++) {
    buffer[FRAME_SIZE - HOP_SIZE + i] = new_buffer[i];
  }
}

static int read_n_samples(SNDFILE* infile,
                          double* buffer,
                          int channels,
                          int n) {
  if (channels == 1) {
    /* MONO */
    int readcount;

    readcount = sf_readf_double(infile, buffer, n);

    return readcount == n;
  } else if (channels == 2) {
    /* STEREO */
    double buf[2 * n];
    int readcount, k;
    readcount = sf_readf_double(infile, buf, n);
    for (k = 0; k < readcount; k++)
      buffer[k] = (buf[k * 2] + buf[k * 2 + 1]) / 2.0;

    return readcount == n;
  } else {
    /* FORMAT ERROR */
    printf("Channel format error.\n");
  }

  return 0;
}

static int read_samples(SNDFILE* infile, double* buffer, int channels) {
  return read_n_samples(infile, buffer, channels, HOP_SIZE);
}

/*static void
dft (double* s, double complex* S)
{
  int i, j;
  for(i=0; i<FRAME_SIZE; i++) {
    S[i] = 0;

    for(j=0; j<FRAME_SIZE; j++) {
      S[i] += s[j] * cexp((-I * 2 * M_PI * i * j)/FRAME_SIZE);
    }
  }
}*/

static void cartesian_to_polar(double complex* spectre,
                               double* amp,
                               double* phase) {
  int i;
  for (i = 0; i < FRAME_SIZE; i++) {
    amp[i] = cabs(spectre[i]);
    phase[i] = carg(spectre[i]);
  }
}

int main(int argc, char* argv[]) {
  char *progname, *infilename;
  SNDFILE* infile = NULL;
  SF_INFO sfinfo;

  progname = strrchr(argv[0], '/');
  progname = progname ? progname + 1 : argv[0];

  if (argc != 2) {
    print_usage(progname);
    return 1;
  };

  infilename = argv[1];

  if ((infile = sf_open(infilename, SFM_READ, &sfinfo)) == NULL) {
    printf("Not able to open input file %s.\n", infilename);
    puts(sf_strerror(NULL));
    return 1;
  };

  /* Read WAV */
  keysPressed = 0;
  int nb_frames = 0;
  int wasSilenced = 1;
  double new_buffer[HOP_SIZE];
  double buffer[FRAME_SIZE];

  /* Plot Init */
  h = gnuplot_init();
  gnuplot_setstyle(h, "lines");

  int i;
  for (i = 0; i < (FRAME_SIZE / HOP_SIZE - 1); i++) {
    if (read_samples(infile, new_buffer, sfinfo.channels) == 1)
      fill_buffer(buffer, new_buffer);
    else {
      printf("not enough samples !!\n");
      return 1;
    }
  }

  /* Info file */
  printf("sample rate %d\n", sfinfo.samplerate);
  printf("channels %d\n", sfinfo.channels);
  printf("size %d\n", (int)sfinfo.frames);

  /* FFTW */
  double complex* spectre = malloc(FRAME_SIZE * sizeof(double complex));
  double complex* samples = malloc(FRAME_SIZE * sizeof(double complex));
  double* phase = malloc(FRAME_SIZE * sizeof(double));
  double* amp = malloc(FRAME_SIZE * sizeof(double));

  fftw_plan plan = fft_init(samples, spectre);

  while (read_samples(infile, new_buffer, sfinfo.channels) == 1) {
    /* hop size */
    fill_buffer(buffer, new_buffer);

    /* Hann */
    for (i = 0; i < FRAME_SIZE; i++)
      samples[i] = buffer[i] * (.5 - .5 * cos((2 * M_PI * i) / FRAME_SIZE));

    /* FFT */
    fft(plan);
    cartesian_to_polar(spectre, amp, phase);

    /* Affichage des informations */
    printf("Processing frame %2d", nb_frames);

    /* Calcul de l'energie */
    const double energyThreshold = .01;
    double energy = getEnergy(buffer);

    if (energy > energyThreshold) {
      if (wasSilenced == 1) {
        /* Boolean pour la detection des silence */
        wasSilenced = 0;
        printf(" Signal detecte");

        /* Recherche d'evenements */
        int maxLocauxIndex[2] = {0, 0};
        double minAmp = 10;

        for (i = 1; i < FRAME_SIZE / 2 - 1; i++)
          if (amp[i] >= minAmp && amp[i] >= amp[i - 1] && amp[i] > amp[i + 1]) {
            if (maxLocauxIndex[0] == 0)
              maxLocauxIndex[0] = i;

            else
              maxLocauxIndex[1] = i;
          }

        /* Interpolation parabolique */
        double eventFreq[2] = {0, 0};
        for (i = 0; i < 2; i++) {
          if (maxLocauxIndex[i] != 0) {
            double a1 = 20 * log10(amp[maxLocauxIndex[i] - 1]);
            double ac = 20 * log10(amp[maxLocauxIndex[i]]);
            double ar = 20 * log10(amp[maxLocauxIndex[i] + 1]);

            double d = .5 * ((a1 - ar) / (a1 - 2 * ac + ar));
            double m = maxLocauxIndex[i] + d;
            eventFreq[i] = m * sfinfo.samplerate / FRAME_SIZE;
            printf(" (%.0f)", eventFreq[i]);
          }
        }

        keyRegister(eventFreq, sfinfo.samplerate);
      }
      else {
        printf(" Repetition");
      }
    }

    else {
      wasSilenced = 1;
      printf(" Silence");
    }

    printf("\n");

    /* PLOT */
    /*gnuplot_resetplot(h);
    gnuplot_plot_x(h,amp,800,"temporal frame");*/

    // sleep(1);

    nb_frames++;
  }

  printKeys();

  fft_exit(plan);

  sf_close(infile);

  free(samples);
  free(spectre);
  free(amp);
  free(phase);

  return 0;
} /* main */

