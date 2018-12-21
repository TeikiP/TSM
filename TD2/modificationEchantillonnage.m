%% Read file
[y,Fs] = audioread('Toms_diner.wav');

sound(y, Fs);
pause(2);
clear sound;

%% Read the first one and a half seconds
samples = [1,1.5*Fs];
[yShort, Fs] = audioread('Toms_diner.wav',samples);

sound(yShort, Fs);
pause(2);
clear sound;

%% Skip a step on two
yFaster = y(1:2:length(y));

sound(yFaster, Fs);
pause(2);
clear sound;

%% Write to file
audiowrite('./TraitementSonMusique/TD2/spedUp.wav',yFaster,Fs);

%% Twice as fast
sound(y, Fs*2);
pause(2);
clear sound;

%% Twice as slow
sound(y, Fs/2);
pause(2);
clear sound;

%% Resampling from 44kHz to 8kHz
yResampled = resample(y,8000,Fs);

sound(yResampled, 8000);
pause(2);
clear sound;
