import("stdfaust.lib");

attack = nentry("attack", 0.001, 0.001, 2, 0.001) : si.smoo;
decay = nentry("decay", 0.2, 0, 1, 0.1) : si.smoo;
sustain = nentry("sustain", 0.7, 0, 1, 0.1) : si.smoo;
release = nentry("release", 1.2, 0, 5, 0.1) : si.smoo;
trigger = nentry("trigger", 0, 0, 1, 1);

pitch = nentry("pitch", 80, 1, 1000, 0.1) : si.smoo;
velocity = nentry("velocity", 0.8, 0.01, 1.5, 0.01) : si.smoo;

adsr = en.adsre(attack, decay, sustain, release, trigger);
process = os.osc(pitch) * adsr * velocity;

/*
freq = nentry("freq",440,20,20000,0.01) : si.smoo;
gain = nentry("gain",1,0,1,0.01) : si.smoo;

process = os.sawtooth(freq)*gain;  
*/