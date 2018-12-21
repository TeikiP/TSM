%% Donnees standard
freqEchantillon = 512.0;
pi = 3.141592;
size = 128;
i = 1:size;

%% Definition du signal 1
signal1.amp = 1.0;
signal1.phase = 0.0;
signal1.freq = 100.0;

%% Definition du signal 2
signal2.amp = 1.0;
signal2.phase = 0.0;
signal2.freq = 156.0;

%% Echantillonage
signal1.sig = signal1.amp * sin(2.0 * pi * signal1.freq * i / freqEchantillon + signal1.phase);
signal2.sig = signal2.amp * sin(2.0 * pi * signal2.freq * i / freqEchantillon + signal2.phase);
signal3.sig = signal1.sig + signal2.sig; % Addition de deux signaux opposés = 0

%% Affichage graphique
close all
figure()
hold on

axis([0 size -1 1]);
plot(signal1.sig, ':r');
plot(signal2.sig, ':b');
plot(signal3.sig, '-m');

hold off