%% Donnees standard
freqEchantillon = 44100.0;
pi = 3.141592;
size = 1024;
i = 1:size;
bits = 2;

%% Definition du signal
signal.amp = 1.0;
signal.phase = 0.0;
signal.freq = 440.0;

%% Echantillonage
signal.sig = signal.amp * sin(2.0 * pi * signal.freq * i / freqEchantillon + signal.phase);

%% Quantification
N_valeurs = 2^(bits-1);
s_quant = (round(signal.sig*N_valeurs)/N_valeurs);

%% Calcul du bruit
bruit = signal.sig - s_quant;

%% Calcul des RMS
rms_bruit = sqrt(sum(bruit.^2)); % .^ = power
rms_signal = sqrt(sum(signal.sig.^2));

%% Calcul du SNR
% Importance du bruit par rapport a l'importance du signal
% Plus le snr est eleve, plus le bruit est faible
% Lineairement proportionnel au nombre de bits
% Unite = dB
snr = [];
snr = [snr 20.0*log10(rms_signal/rms_bruit)];

%% Affichage graphique
close all
figure()
hold on

axis([0 size -1 1]);

plot(bruit, '-g');
plot(signal.sig, '-b');
plot(s_quant, '-r');

hold off
