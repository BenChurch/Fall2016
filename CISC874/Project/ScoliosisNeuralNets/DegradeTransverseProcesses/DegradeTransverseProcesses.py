import os
import unittest
import vtk, qt, ctk, slicer, numpy
from slicer.ScriptedLoadableModule import *
import logging

#
# DegradeTransverseProcesses
#

class DegradeTransverseProcesses(ScriptedLoadableModule):

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Degrade Transverse Processes"
    self.parent.categories = ["Scoliosis"]
    self.parent.dependencies = []
    self.parent.contributors = ["Ben Church"]
    self.parent.helpText = """
    This is an example of scripted loadable module bundled in an extension.
    It performs a simple thresholding on the input volume and optionally captures a screenshot.
    """
    self.parent.acknowledgementText = """
    This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
    and Steve Pieper, Isomics, Inc. and was partially funded by NIH grant 3P41RR013218-12S1.
""" # replace with organization, grant and thanks.

#
# DegradeTransverseProcessesWidget
#

class DegradeTransverseProcessesWidget(ScriptedLoadableModuleWidget):

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    # Instantiate and connect widgets ...

    #
    # Parameters Area
    #
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Parameters"
    self.layout.addWidget(parametersCollapsibleButton)

    # Layout within the dummy collapsible button
    parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

    # reload button
    self.reloadButton = qt.QPushButton('Restart module')
    self.layout.addWidget(self.reloadButton)
    
    #
    # input volume selector
    #
    # Don't use this - iterate through entire batch of TrXFiducials loaded
    self.inputSelector = slicer.qMRMLNodeComboBox()
    self.inputSelector.nodeTypes = ["vtkMRMLMarkupsFiducialNode"]
    self.inputSelector.selectNodeUponCreation = True
    self.inputSelector.addEnabled = False
    self.inputSelector.removeEnabled = False
    self.inputSelector.noneEnabled = False
    self.inputSelector.showHidden = False
    self.inputSelector.showChildNodeTypes = False
    self.inputSelector.setMRMLScene( slicer.mrmlScene )
    self.inputSelector.setToolTip( "Pick the input to the algorithm." )
    #parametersFormLayout.addRow("Input Volume: ", self.inputSelector)

    #
    # noise standard deviation
    #
    self.StdDevSliderWidget = ctk.ctkSliderWidget()
    self.StdDevSliderWidget.singleStep = 0.01
    self.StdDevSliderWidget.minimum = 0
    self.StdDevSliderWidget.maximum = 10
    self.StdDevSliderWidget.value = 1
    self.StdDevSliderWidget.setToolTip("Set standard deviation (mm) of noise to introduce to all points.")
    parametersFormLayout.addRow("Noise standard deviation", self.StdDevSliderWidget)

    #
    # deletion fraction
    #
    self.DeletionSliderWidget = ctk.ctkSliderWidget()
    self.DeletionSliderWidget.singleStep = 0.01
    self.DeletionSliderWidget.minimum = 0
    self.DeletionSliderWidget.maximum = 1
    self.DeletionSliderWidget.value = 0.5
    self.DeletionSliderWidget.setToolTip("Set fraction of points from original sets to delete.")
    parametersFormLayout.addRow("Deletion fraction", self.DeletionSliderWidget)

    #
    # displacement fraction
    #
    self.DisplacementSliderWidget = ctk.ctkSliderWidget()
    self.DisplacementSliderWidget.singleStep = 0.01
    self.DisplacementSliderWidget.minimum = 0
    self.DisplacementSliderWidget.maximum = 1
    self.DisplacementSliderWidget.value = 0.5767
    self.DisplacementSliderWidget.setToolTip("Set fraction of points from original data set to misplace.")
    parametersFormLayout.addRow("Displacement fraction", self.DisplacementSliderWidget)
    
    #
    # check box to trigger taking screen shots for later use in tutorials
    #
    self.enableScreenshotsFlagCheckBox = qt.QCheckBox()
    self.enableScreenshotsFlagCheckBox.checked = 0
    self.enableScreenshotsFlagCheckBox.setToolTip("If checked, take screen shots for tutorials. Use Save Data to write them to disk.")
    #parametersFormLayout.addRow("Enable Screenshots", self.enableScreenshotsFlagCheckBox)

    #
    # Degrade Button
    #
    self.DegradeButton = qt.QPushButton("Degrade point sets")
    self.DegradeButton.toolTip = "Apply noise, delete fraction of random points, and misplace fraction of random points."
    self.DegradeButton.enabled = True
    parametersFormLayout.addRow(self.DegradeButton)

    # connections
    self.DegradeButton.connect('clicked(bool)', self.onDegradeButton)
    self.reloadButton.connect('clicked(bool)',self.onReloadButton)
    #self.inputSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)
    #self.outputSelector.connect("currentNodeChanged(vtkMRMLNode*)", self.onSelect)

    # Add vertical spacer
    self.layout.addStretch(1)

    # Refresh Apply button state
    #self.onSelect()

  def cleanup(self):
    pass

  #def onSelect(self):
   # self.applyButton.enabled = self.inputSelector.currentNode() and self.outputSelector.currentNode()

  def onDegradeButton(self):
    self.NoiseStdDev = self.StdDevSliderWidget.value
    self.DeletionFraction = self.DeletionSliderWidget.value
    self.DisplacementFraction = self.DisplacementSliderWidget.value
    logic = DegradeTransverseProcessesLogic()
    #enableScreenshotsFlag = self.enableScreenshotsFlagCheckBox.checked
    #imageThreshold = self.imageThresholdSliderWidget.value
    logic.DegradeInputData(self.NoiseStdDev, self.DeletionFraction, self.DisplacementFraction)
    
  def onReloadButton(self):
    self.MarkupPoints = slicer.util.getNodesByClass('vtkMRMLMarkupsFiducialNode')
    for PointSet in range(self.MarkupPoints.__len__()):
      if (self.MarkupPoints.__getitem__(PointSet).GetNthFiducialLabel(0)[-1] == "~"):
        slicer.mrmlScene.RemoveNode(self.MarkupPoints.__getitem__(PointSet))
    slicer.util.reloadScriptedModule('DegradeTransverseProcesses')

#
# DegradeTransverseProcessesLogic
#

class DegradeTransverseProcessesLogic(ScriptedLoadableModuleLogic):

  def __init__(self):
    self.LandmarkPointSets = []      # Will contain tuples: (TrXFiducial point labels, TrXFiducial### point sets)
    self.DeletedLandmarkLabels = []   # Will ensure we don't try to misplace deleted points or vice-versa
    self.MisplacedLandmarkLabels = []
    

  def DegradeInputData(self, NoiseStdDev, DeletionFraction, DisplacementFraction):
    self.InputData = slicer.util.getNodesByClass('vtkMRMLMarkupsFiducialNode')
    for InputSet in range(self.InputData.__len__()):
      CurrentLandmarkSet = self.InputData.__getitem__(InputSet)
      self.LandmarkPointSets.append([])
      for InputPoint in range(CurrentLandmarkSet.GetNumberOfFiducials()):
        self.LandmarkPointSets[InputSet].append((CurrentLandmarkSet.GetNthFiducialLabel(InputPoint),CurrentLandmarkSet.GetMarkupPointVector(InputPoint,0)))
        
    for InputSet in range(self.LandmarkPointSets.__len__()):
      print " "   #empty line
      print "Landmark set #" + str(InputSet)
      CurrentLandmarkSet = self.LandmarkPointSets[InputSet]
      for InputPoint in range(CurrentLandmarkSet.__len__()):
        print self.LandmarkPointSets[InputSet][InputPoint]
    
    # Randomly select points for deletion and misplacement
    for InputSet in range(self.LandmarkPointSets.__len__()):
      print InputSet
      DeletionAmount = (int)(self.LandmarkPointSets[InputSet].__len__() * DeletionFraction)
      DisplacementAmount = (int)(self.LandmarkPointSets[InputSet].__len__() * DisplacementFraction)
      # could go into infinite loop if there are very few points?
      while (DeletionAmount > 0 and DisplacementAmount > 0):
        DeletionIndex = (int)(numpy.random.random_sample() * (self.LandmarkPointSets[InputSet].__len__()))
        self.LandmarkPointSets[InputSet].__delitem__(DeletionIndex)
        DeletionAmount -= 1
        # Misplace points
        DisplacementIndex = (int)(numpy.random.random_sample() * (self.LandmarkPointSets[InputSet].__len__()))
        DisplacementPoint = self.LandmarkPointSets[InputSet][DisplacementIndex]
        # Need to estimate direction of symmetric neighbor to this, and extrapolate
        # '-> use point labels? Lax assumption?
        RightLeftVector = self.EstimateRightLeftVector(DisplacementPoint,self.LandmarkPointSets[InputSet])
        DisplacementAmount -= 1
    
    # Displace points systematically away from symmetric neighbor plus anterior offset, simulating misplacing a landmark on a rib
    for InputSet in range(self.LandmarkPointSets.__len__()):
      CurrentLandmarkSet = self.LandmarkPointSets[InputSet]
    
    # Add noise to point locations  
    for InputSet in range(self.LandmarkPointSets.__len__()):
      CurrentLandmarkSet = self.LandmarkPointSets[InputSet]
      for InputPoint in range(CurrentLandmarkSet.__len__()):
        CurrentLandmarkPoint = CurrentLandmarkSet[InputPoint][1]
        for dim in range(3):      # for each of the point's spatial dimensions
          CurrentLandmarkPoint[dim] += numpy.random.standard_normal() * NoiseStdDev
    
    # Create new FiducialMarkupNodes for Slicer scene
    for InputSet in range(self.LandmarkPointSets.__len__()):
      NewMarkupsNode = slicer.vtkMRMLMarkupsFiducialNode()
      NewMarkupsNode.SetName(self.InputData.__getitem__(InputSet).GetName() + "~")
      CurrentLandmarkSet = self.LandmarkPointSets[InputSet]
      for InputPoint in range(CurrentLandmarkSet.__len__()):
        CurrentLandmarkPoint = CurrentLandmarkSet[InputPoint][1]
        NewMarkupsNode.AddFiducial(CurrentLandmarkPoint[0], CurrentLandmarkPoint[1], CurrentLandmarkPoint[2])
        NewPointLabel = self.LandmarkPointSets[InputSet][InputPoint][0] + "~"
        NewMarkupsNode.SetNthFiducialLabel(InputPoint, NewPointLabel)
      slicer.mrmlScene.AddNode(NewMarkupsNode)
           
    return True

  # returns vector pointing from symmetric neighbor to DisplacementPoint, estimates it from nearby points if symmetric neighbor is missing
  def EstimateRightLeftVector(self, DisplacementPoint, LandmarkSet):
    for LabelPoint in LandmarkSet:
      if((DisplacementPoint[0][:-1] == LabelPoint[0][:-1]) and (DisplacementPoint[0] != LabelPoint[0])):
        # The DisplacementPoint has a symmetric partner
        for dim in range(3):
          LeftRightVector[dim] = DisplacementPoint[1][dim] - LabelPoint[1][dim]
        return LeftRightVector
    # ASSERT that DisplacementPoint has no symmetric neighbor
      
        
class DegradeTransverseProcessesTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_DegradeTransverseProcesses1()

  def test_DegradeTransverseProcesses1(self):
    """ Ideally you should have several levels of tests.  At the lowest level
    tests should exercise the functionality of the logic with different inputs
    (both valid and invalid).  At higher levels your tests should emulate the
    way the user would interact with your code and confirm that it still works
    the way you intended.
    One of the most important features of the tests is that it should alert other
    developers when their changes will have an impact on the behavior of your
    module.  For example, if a developer removes a feature that you depend on,
    your test should break so they know that the feature is needed.
    """

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=5767', 'FA.nrrd', slicer.util.loadVolume),
        )

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        logging.info('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        logging.info('Loading %s...' % (name,))
        loader(filePath)
    self.delayDisplay('Finished with download and loading')

    volumeNode = slicer.util.getNode(pattern="FA")
    logic = DegradeTransverseProcessesLogic()
    self.assertIsNotNone( logic.hasImageData(volumeNode) )
    self.delayDisplay('Test passed!')
