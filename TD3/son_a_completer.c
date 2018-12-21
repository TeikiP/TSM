#include <math.h>
#include "gnuplot_i.h"

#define SAMPLING_RATE 44100.0
#define CHANNELS_NUMBER 1
#define BITS 16
#define ENCODING "signed-integer"
#define N 1024

char *RAW_FILE = "tmp-in.raw";

FILE *
sound_file_open_read (char *sound_file_name)
{
  char cmd[256];

  snprintf (cmd, 256,
	    "sox -c %d -r %d -e %s -b %d %s %s",
	    CHANNELS_NUMBER, (int)SAMPLING_RATE, ENCODING, BITS, sound_file_name, RAW_FILE);

  system (cmd);

  return fopen (RAW_FILE, "rb");
}

void
sound_file_close_read (FILE *fp)
{
  fclose (fp);
  remove(RAW_FILE);
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

void
play_file(char *sound_file_name)
{
  char cmd[256];

  snprintf(cmd, 256, "play %s", sound_file_name);

  system(cmd);
}

void
check_args(int argc, const char *s) {
  if(argc != 2) {
    fprintf(stderr, "Usage: %s <input> \n", s);
    exit(EXIT_FAILURE);
  }
}

int
main (int argc, char *argv[])
{
  FILE *input;
  double s[N], x[N];

  check_args(argc, argv[0]);

  gnuplot_ctrl *h = gnuplot_init();
  gnuplot_setstyle(h, "lines");

  input = sound_file_open_read(argv[1]);

  //play_file(argv[1]);

  int i=0;
  for (;i<N; i++)
    x[i] = i/SAMPLING_RATE;

  int amountValues = 0;
  while((amountValues = sound_file_read (input, s)))
    {

      // affichage
      gnuplot_cmd(h, "set yr [-1:1]");
      //gnuplot_cmd(h, "set xr [0:1024]");
      gnuplot_resetplot(h);

      gnuplot_plot_xy(h, x, s, amountValues, "Echantillon"); 

      usleep(N * 1000000.0 / SAMPLING_RATE);

      for(i=0; i<N; i++)
        x[i] += N/SAMPLING_RATE;
    }

  sound_file_close_read (input);
  exit (EXIT_SUCCESS);
}
