The final weight values when I run the Python program are: w0 = -0.2134333789591386, w1 = 0.2333385447732124.

This file contains both a description of the program with how to run it, and 
a quick explanation of the results.

-------------------------------------------

To run the Python program, use the command: 
	python Asg3_10006197.py.
This prints the weight values to the console, and generates output.csv.

The output.wav file was generated using MATLAB. In the appropriate directory,
run the commands:
	load output.csv
	audiowrite('output.wav', output, 9000)

The program is written in Python3.5 with standard math and csv libraries.
It also uses Numpy, which I assume is fine since the assignment PDF mentions it.

-------------------------------------------

I suspect that the doubling of the .wav file size compared to the originals
provided with the assignment is just a result of MATLAB writing the file
with one more bit per sample, since it appears to be a doubling in size.

Nonetheless, I am convince the program works because the sound file produced
by MATLAB has the speaking voice, now, speaking clearly over the singing voice.
Whereas in the original files, the singing voice is distinct and loud, it is
reduced in the output, although apparently becomes clearer where it most
resembles the spoken voice.