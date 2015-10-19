Start with people who have some antibody but are not infected

Give them OPV1

The number of people who get VDPV1 should be affected by this parameter

If neutralization_time_tau is higher, more people should get infected
If it is lower, less people should get infected

Newborn_maternal_decay_demographics only has 1 person, need a few thousand, probably


ADDED:
To minimize randomness, I went ahead and created another demographics file:
..\\polio_input\\Sandbox_kids_with_single_antibody_level_demographics.json

This has every kid with an antibody titer of 3.

I'm still seeing higher reported infections and paralytic cases with the higher tau, which seems implausible.
Checking in files to show this

