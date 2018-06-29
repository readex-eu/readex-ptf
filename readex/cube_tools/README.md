Tools:
1. Filter Region:
Filter the profile call tree based on given threshold value
Read from .cubex file after first run of the instrumented application.
Usage: scorep-auto-filter [-t 0.01] [-f autofilter] <profile.cubex>
[-t 0.01] threshold for granularity(optional)
[-f autofilter] Name of the generated filter file(optional)
<profile.cubex> the name of the profile output file(mandatory)
Output: Generate valid scorep.filt or specified name file, which will be used as filter file on the next run of the
instrumented application.

2. Dynamism Detection
Read from .cubex file after second run of the instrumented application.
Usage: readex-dyn-detect [-t 0.001] [-p phaseRegion] [-c 50] [-v 15] [-w 15] <profile.cubex>
[-t 0.001] threshold for granularity(optional)
[-p phaseRegion] the name of the progress loop(mandatory)
[-c 50] threshold for the compute instensity metric deviation(optional)
[-v 10] threshold for the time variation as % of mean region time (optional)
[-w 10] threshold for region importance/weight as % of phase execution time(optional)
<profile.cubex> the name of the profile output file(mandatory)

Output:
2.1 Detect non-nested significant regions

2.2 Compute intra-phase and inter-phase dynamism to detect tuning potential.

- The tool outputs Region name, min , max, % of time deviation, absolute value of compute intensity, weight as % of phase execution time.

There is intra-phase dynamism due to hight variation(>vt) of Execution time and high weigh
There is intra-phase dynamism due to high weigh(>vw) and high deviation of compute intensity metric across different signifcant regions

- The tool shows min, max, mean, % of time deviation, and variation of min and max values as % of mean execution time.

There is inter-phase dynamism due to high variation(>vt) across phases of execution time

