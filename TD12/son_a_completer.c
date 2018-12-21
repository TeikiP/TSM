#include <math.h>
#include "gnuplot_i.h"

#define PIVALUE 3.141592
#define SAMPLING_RATE 44100.0
#define CHANNELS_NUMBER 1
#define BITS 16
#define ENCODING "signed-integer"
#define N 44100*4
#define TVALUE (float)i/(float)SAMPLING_RATE

char *RAW_IN_FILE = "tmp-in.raw";
char *RAW_OUT_FILE = "tmp-out.raw";

/******************************************************************************/

FILE *
sound_file_open_read (char *sound_file_name)
{
  char cmd[256];

  snprintf (cmd, 256,
	    "sox -c %d -r %d -e %s -b %d %s %s",
	    CHANNELS_NUMBER, (int)SAMPLING_RATE, ENCODING, BITS, sound_file_name, RAW_IN_FILE);

  system (cmd);

  return fopen (RAW_IN_FILE, "rb");
}

void
sound_file_close_read (FILE *fp)
{
  fclose (fp);
  remove(RAW_IN_FILE);
}

int
sound_file_read (FILE *fp, double *s)
{
  short fileValues[N];
  int readAmount = fread(fileValues, BITS/8, N, fp);

  int i=0;
  for (;i<N; i++)
    s[i] = (double) fileValues[i] / (double) pow(2, BITS-1);

  return readAmount;
}

/******************************************************************************/

FILE *
sound_file_open_write ()
{
  return fopen (RAW_OUT_FILE, "wb");
}

void
sound_file_close_write (FILE *fp, char *sound_file_name)
{
  fclose (fp);
  
  char cmd[256];

  snprintf (cmd, 256,
	    "sox -c %d -r %d -e %s -b %d %s %s",
	    CHANNELS_NUMBER, (int)SAMPLING_RATE, ENCODING, BITS, RAW_OUT_FILE, sound_file_name);

  system (cmd);
  
  remove(RAW_OUT_FILE);
}

int
sound_file_write (FILE *fp, double *s)
{
  short fileValues[N];

  int i=0;
  for (;i<N; i++)
    fileValues[i] = s[i] * ((double) pow(2, BITS-1) - 1);
    
  int writeAmount = fwrite(fileValues, BITS/8, N, fp);

  return writeAmount;
}

int
sound_file_write_stereo (FILE *fp, double *gauche, double *droite)
{
  short fileValues[N*2];

  int i=0;
  for (;i<N; i++) {
    fileValues[i*2] = gauche[i] * ((double) pow(2, BITS-1) - 1);
    fileValues[i*2+1] = droite[i] * ((double) pow(2, BITS-1) - 1);
  }
    
  int writeAmount = fwrite(fileValues, BITS/8, N*2, fp);

  return writeAmount;
}

/******************************************************************************/

void
play_file (char *sound_file_name)
{
  char cmd[256];

  snprintf(cmd, 256, "play %s", sound_file_name);

  system(cmd);
}

void
check_args (int argc, const char *s) {
  if(argc != 2) {
    fprintf(stderr, "Usage: %s <output>\n", s);
    exit(EXIT_FAILURE);
  }
}

/******************************************************************************/

void
synthese_additive (double* s) {
  double a[] = {.5, .5};
  double f[] = {440, 700};
  
  for (int i=0;i<N; i++)
    s[i] = a[0] * sin(2.0 * PIVALUE * f[0] * TVALUE)
         + a[1] * sin(2.0 * PIVALUE * f[1] * TVALUE);
}

void
synthese_am (double* s) {
  double c = .5;
  double a[] = {.5, .5};
  double f[] = {440, 10};
  
  for (int i=0;i<N; i++)
    s[i] = (c + a[1] * sin(2.0 * PIVALUE * f[1] * TVALUE)) 
                     * sin(2.0 * PIVALUE * f[0] * TVALUE);
}

void
synthese_fm (double* s) {
  double a = .8;
  double fc = 440;
  double fm = 110;
  double mod = 3;
  
  for (int i=0;i<N; i++)
    s[i] = a * sin(2.0 * PIVALUE * fc * TVALUE + mod * sin(2.0 * PIVALUE * fm * TVALUE));;
}

void
synthese_fm_variable (double* s) {
  double a[N];
  double fc[N];
  double fm[N];
  double mod[N];
  
  for (int i=0;i<N; i++) {
    a[i] = .8;
    fc[i] = 440 + (float) i * 440.0 / (float) N;
    fm[i] = 110 + (float) i * 110.0 * 4 / (float) N;
    mod[i] = 5;
    
    s[i] = a[i] * sin(2.0 * PIVALUE * fc[i] * TVALUE + mod[i] * sin(2.0 * PIVALUE * fm[i] * TVALUE));;
  }
}


/******************************************************************************/

int
main (int argc, char *argv[])
{
  const double AMP = 1.0;
  
  FILE *output;
  double s[N] = {0};

  check_args(argc, argv[0]);
  
  output = sound_file_open_write();
  
  if (output == NULL)
    printf("Error opening output file.\n");

  //synthese_additive(s);
  //synthese_am(s);
  //synthese_fm(s);
  synthese_fm_variable(s);
  
  if (sound_file_write (output, s) != N)
    printf("Error writing in file.\n");

  sound_file_close_write (output, argv[1]);
  
  exit (EXIT_SUCCESS);
}
