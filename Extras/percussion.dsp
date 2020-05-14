import("stdfaust.lib");
ctrlFreq = 500;
q = 10;
gain = 0.7;

attack = 0.001;
decay = 0.2;
sustain = 0.7;
release = 1.2;
trigger = 0;

pitch = 80;

adsr = en.adsre(attack, decay, sustain, release, trigger);
process = os.osc(pitch) * adsr : fi.resonlp(ctrlFreq,q,gain);


  