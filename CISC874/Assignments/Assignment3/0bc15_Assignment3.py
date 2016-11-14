
class Node:
  def __init__(self):

class PCANetwork:
  def __init__(self):
      


class main:
  def __init__(self):
    import csv, numpy
    # Read sound.csv into one array for each channel
    SoundFile = open('sound.csv', 'rt', encoding = 'ascii')
    LineReader = csv.reader(SoundFile)
    ChannelZero = (zip(*LineReader))[0]
    ChanelOne = (zip(*LineReader))[1]

  def run(self):
    for data in ChannelZero:
      print(data)
