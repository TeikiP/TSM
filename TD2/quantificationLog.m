%% Parametres
N = 8;
mu = 255;
N_valeurs = 2^(N-1);
[s,Fs] = audioread('Toms_diner.wav');

%% Compression
s_comp = sign(s) .* (log(1+abs(s) .* mu) / log(1+mu));

%% Quantification lineaire
s_quant = round(s_comp .* N_valeurs) ./ N_valeurs;

%% Expansion
s_exp = (sign(s_quant) .* (1/mu)) .* (exp(abs(s_quant) .* (log(1+mu)))-1);