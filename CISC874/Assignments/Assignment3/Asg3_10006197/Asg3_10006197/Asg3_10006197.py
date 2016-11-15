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

        #self.CoVarMat = [[0,0],[0,0]]  # [[x1x1, x1y2], [x2y1, y2y2]]

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
SoundFile = open('./sound.csv', 'rt', encoding = 'ascii')
LineReader = csv.reader(SoundFile)
(ChannelZero, ChannelOne) = list((zip(*LineReader)))

PCANetwork = Network()
PCANetwork.ReduceDimensionality(ChannelZero, ChannelOne)

with open('./output.csv', 'w') as csvfile:
    OutputWriter = csv.writer(csvfile, delimiter = ',', lineterminator = '\n')
    OutputWriter.writerow(['Input sample #','Resulting output'])
    for i, Output in enumerate(PCANetwork.OutputHistory):
        OutputWriter.writerow([str(i), str(Output)])
for weight in PCANetwork.Weights:
    print(weight)