import("stdfaust.lib");

attack_1 = nentry("attack_1", 0.001, 0.001, 2, 0.001) : si.smoo;
decay_1 = nentry("decay_1", 0.2, 0, 1, 0.1) : si.smoo;
sustain_1 = nentry("sustain_1", 0.7, 0, 1, 0.1) : si.smoo;
release_1 = nentry("release_1", 1.2, 0, 5, 0.1) : si.smoo;
trigger_1 = nentry("trigger_1", 0, 0, 1, 1);

pitch_1 = nentry("pitch_1", 80, 1, 1000, 0.1) : si.smoo;
velocity_1 = nentry("velocity_1", 0.8, 0.01, 1.5, 0.01) : si.smoo;

attack_2 = nentry("attack_2", 0.001, 0.001, 2, 0.001) : si.smoo;
decay_2 = nentry("decay_2", 0.2, 0, 1, 0.1) : si.smoo;
sustain_2 = nentry("sustain_2", 0.7, 0, 1, 0.1) : si.smoo;
release_2 = nentry("release_2", 1.2, 0, 5, 0.1) : si.smoo;
trigger_2 = nentry("trigger_2", 0, 0, 1, 1);

pitch_2 = nentry("pitch_2", 80, 1, 1000, 0.1) : si.smoo;
velocity_2 = nentry("velocity_2", 0.8, 0.01, 1.5, 0.01) : si.smoo;

adsr_1 = en.adsre(attack_1, decay_1, sustain_1, release_1, trigger_1);
adsr_2 = en.adsre(attack_2, decay_2, sustain_2, release_2, trigger_2);

process = os.osc(pitch_1) * adsr_1 * velocity_1 + os.sawtooth(pitch_2) * adsr_2 * velocity_2;
