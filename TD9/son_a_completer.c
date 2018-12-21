#include <math.h>
#include "gnuplot_i.h"

#define PIVALUE 3.141592
#define SAMPLING_RATE 44100.0
#define CHANNELS_NUMBER 1
#define BITS 16
#define ENCODING "signed-integer"
#define N 1024

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
play_file(char *sound_file_name)
{
  char cmd[256];

  snprintf(cmd, 256, "play %s", sound_file_name);

  system(cmd);
}

void
check_args(int argc, const char *s) {
  if (CHANNELS_NUMBER == 1) {
    if(argc != 3) {
      fprintf(stderr, "Usage: %s <output> <freq>\n", s);
      exit(EXIT_FAILURE);
    }
  }
  
  else if (CHANNELS_NUMBER == 2) {
    if(argc != 4) {
      fprintf(stderr, "Usage: %s <output> <fGauche> <fDroite>\n", s);
      exit(EXIT_FAILURE);
    }  
  }
}

/******************************************************************************/

int
main (int argc, char *argv[])
{
  const double AMP = 1.0;
  
  FILE *input, *output;
  double s[N], gauche[N], droite[N];
  double phase = sin(2.0 * PIVALUE * atoi(argv[2]) * N/(float)SAMPLING_RATE);

  check_args(argc, argv[0]);
  
  output = sound_file_open_write();
  
  if (output == NULL)
    printf("Error opening output file.\n");

  if (CHANNELS_NUMBER == 1) {
    for (int j=0; j<100; j++) {
      for (int i=0;i<N; i++)
        s[i] = AMP * sin(2.0 * PIVALUE * atoi(argv[2]) * (float)(i/*+j*N*/)/(float)SAMPLING_RATE + j*phase);
      
      if (sound_file_write (output, s) != N)
        printf("Error writing in file.\n");
    }
  }
  
  else if (CHANNELS_NUMBER == 2) {
    for (int i=0;i<N; i++) {
      gauche[i] = AMP * sin(2.0 * PIVALUE * atoi(argv[2]) * (float)i/(float)SAMPLING_RATE);
      droite[i] = AMP * sin(2.0 * PIVALUE * atoi(argv[3]) * (float)i/(float)SAMPLING_RATE);
    }
    
    if (sound_file_write_stereo (output, gauche, droite) != N*2)
      printf("Error writing in file.\n");
  }

  sound_file_close_write (output, argv[1]);
  
  exit (EXIT_SUCCESS);
}
