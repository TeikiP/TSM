%% Donnees standard
freqEchantillon = 44100.0;
pi = 3.141592;
size = 1024;
i = 1:size;

%% Definition du signal
signal.amp = 1.0;
signal.phase = 0.0;
signal.freq = 440.0;

%% Echantillonage
signal.sig = signal.amp * sin(2.0 * pi * signal.freq * i / freqEchantillon + signal.phase);

%% Affichage graphique
close all
figure()
hold on

axis([0 size -1 1]);
plot(signal.sig);

hold off