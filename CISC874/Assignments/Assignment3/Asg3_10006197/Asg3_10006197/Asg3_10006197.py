# CISC 874 Assignment #3 - Written by Ben Church - 10006197

# This short program defines a network class suitable to the problem at hand.
# Its architecture was hard-coded given the known application, and the
# resulting ease of implementation over a generalizable network.
# The networks only method is ReduceDimensionality. This trains the
# networks weight values on all the input samples, and keeps track of 
# the output resulting from each input, to be later written out.

# The network uses the weight update algorithm exactly as described in
# the class notes.

# The main program reads in and organizes the sound data, and then runs it
# throught the network, Python allows this to be done with 5 lines.
# The networks output history is then written to output.csv which can 
# be converted to a .wav file by MATLAB.

import csv, numpy, math

class Network:
    def __init__(self):
        # Architecture is prescribed by problem, initialize parameters
        self.Inputs = [0,0]
        self.Output = 0
        self.Weights = [1, 0]
        self.LearningRate = 0.1

        # Will contain network ouputs for each input, to be writtent to 'output.csv'
        self.OutputHistory = []

    def ReduceDimensionality(self, ChannelZero, ChannelOne):
        for Input in zip(ChannelZero, ChannelOne):
            # Reinitialize
            self.Output = 0
            # Feedforward
            for i, Weight in enumerate(self.Weights):
                self.Inputs[i] = float(Input[i])
                self.Output += self.Inputs[i] * Weight
            self.OutputHistory.append(self.Output)
            # Adjust weights
            self.Weights[0] += ((self.LearningRate) * (self.Output) * (self.Inputs[0]) - (self.Output * self.Output * self.Weights[0]))
            self.Weights[1] += ((self.LearningRate) * (self.Output) * (self.Inputs[1]) - (self.Output * self.Output * self.Weights[1]))

# Read sound.csv into one array for each channel
SoundFile = open('./sound.csv', 'rt')
LineReader = csv.reader(SoundFile)
(ChannelZero, ChannelOne) = list((zip(*LineReader)))

PCANetwork = Network()
PCANetwork.ReduceDimensionality(ChannelZero, ChannelOne)

with open('./output.csv', 'w') as csvfile:
    OutputWriter = csv.writer(csvfile, delimiter = ',', lineterminator = '\n')
    #OutputWriter.writerow(['Input sample #','Resulting output'])   --  MATLAB needs rows to be similar, header messes this up
    for i, Output in enumerate(PCANetwork.OutputHistory):
        #OutputWriter.writerow([str(i), str(Output)])       --      leave off indices rather than write MATLAB code
        OutputWriter.writerow([str(Output)])

for weight in PCANetwork.Weights:
    print(weight)