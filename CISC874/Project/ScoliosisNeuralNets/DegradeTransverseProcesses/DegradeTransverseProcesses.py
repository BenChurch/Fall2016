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
    self.DisplacementSliderWidget.value = 0.5
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
    
    anglePanel = ctk.ctkCollapsibleButton()
    anglePanel.text = "Angle measurement"
    self.layout.addWidget(anglePanel)
    
    outputVerticalLayout = qt.QGridLayout(anglePanel)

    TableHeaders = ["Landmark set", "Max angle", "Vertebra 1", "Vertebra 2"]
    self.angleTable = qt.QTableWidget((slicer.mrmlScene.GetNodesByClass('vtkMRMLMarkupsFiducialNode').GetNumberOfItems()), 4)
    self.angleTable.sortingEnabled = False
    self.angleTable.setEditTriggers(0)
    self.angleTable.setMinimumHeight(self.angleTable.verticalHeader().length() + 25)
    self.angleTable.horizontalHeader().setResizeMode(qt.QHeaderView.Stretch)
    outputVerticalLayout.addWidget(self.angleTable, 2, 0, 1, 5)
    self.angleTable.setHorizontalHeaderLabels(TableHeaders)
    
    self.CalculateAnglesButton = qt.QPushButton("Calculate angles")
    self.savePointsWithAnglesButton = qt.QPushButton("Save altered data + angles")
    outputVerticalLayout.addWidget(self.CalculateAnglesButton, 0, 0, 1, 3)
    outputVerticalLayout.addWidget(self.savePointsWithAnglesButton, 0, 3, 1, 3)

    # reload button
    self.reloadButton = qt.QPushButton('Restart module')
    self.layout.addWidget(self.reloadButton)
    
    # connections
    self.DegradeButton.connect('clicked(bool)', self.onDegradeButton)
    self.CalculateAnglesButton.connect('clicked(bool)', self.onCalculateAnglesButton)
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
    
  def onCalculateAnglesButton(self):
    MarkupPoints = slicer.util.getNodesByClass('vtkMRMLMarkupsFiducialNode')
    logic = CalculateAnglesLogic()
    Angles = logic.CalculateAngles()
    (MaxAngles, MaxVertebrae) = logic.FindMaxCoronalAngles()
    self.PopulateAnglesTable(MaxAngles, MaxVertebrae, MarkupPoints)
    
  def onReloadButton(self):
    self.MarkupPoints = slicer.util.getNodesByClass('vtkMRMLMarkupsFiducialNode')
    for PointSet in range(self.MarkupPoints.__len__()):
      if (self.MarkupPoints.__getitem__(PointSet).GetNthFiducialLabel(0)[-1] == "~"):
        slicer.mrmlScene.RemoveNode(self.MarkupPoints.__getitem__(PointSet))
    slicer.util.reloadScriptedModule('DegradeTransverseProcesses')

  def PopulateAnglesTable(self, MaxAngles, MaxVertebrae, MarkupPoints):
    for i, Angle in enumerate(MaxAngles):
      CurrentLandmarkSet = MarkupPoints.__getitem__(i)
      self.angleTable.setItem(i, 0, qt.QTableWidgetItem())
      self.angleTable.setItem(i, 1, qt.QTableWidgetItem())
      self.angleTable.setItem(i, 2, qt.QTableWidgetItem())
      self.angleTable.setItem(i, 3, qt.QTableWidgetItem())
      self.angleTable.item(i, 0).setText(CurrentLandmarkSet.GetName())
      self.angleTable.item(i, 1).setText(str(Angle))
      self.angleTable.item(i, 2).setText(MaxVertebrae[0][i])
      self.angleTable.item(i, 3).setText(MaxVertebrae[1][i])
#
# DegradeTransverseProcessesLogic
#

class DegradeTransverseProcessesLogic(ScriptedLoadableModuleLogic):

  def __init__(self):
    self.LandmarkPointSets = []      # Will contain tuples: (TrXFiducial point labels, TrXFiducial### point sets)
    self.DeletedLandmarkLabels = []   # Will ensure we don't try to misplace deleted points or vice-versa
    self.MisplacedLandmarkLabels = []
    self.RightLeftScale = 1000          # Length of vector in coronal plane used to misplace point to behind a rib
    self.AntPostScale = 20   # Length of vector in parasagital plane used to misplace point from behind rib onto rib
    

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
    
    # Randomly select points for deletion and displacement
    for InputSet in range(self.LandmarkPointSets.__len__()):
      DeletionAmount = (int)(self.LandmarkPointSets[InputSet].__len__() * DeletionFraction)
      DisplacementAmount = (int)(self.LandmarkPointSets[InputSet].__len__() * DisplacementFraction)
      # Must keep track of already displaced points to not get them again
      # '-> Means previously displaced points can still be deleted
      AlreadyDisplaced = []
      # could go into infinite loop if there are very few points?
      while (DeletionAmount > 0 or DisplacementAmount > 0):
        if(DeletionAmount > 0):
          print "DELETING"
          # ** As it stands, displaced points CAN be deleted **
          DeletionIndex = (int)(numpy.random.random_sample() * (self.LandmarkPointSets[InputSet].__len__()))
          self.LandmarkPointSets[InputSet].__delitem__(DeletionIndex)
          DeletionAmount -= 1
        if(DisplacementAmount > 0):
          # Misplace points
          DisplacementIndex = (int)(numpy.random.random_sample() * (self.LandmarkPointSets[InputSet].__len__()))
          # This search is blind - pretty slow
          while(self.LandmarkPointSets[InputSet][DisplacementIndex][0] in AlreadyDisplaced):
            DisplacementIndex = (int)(numpy.random.random_sample() * (self.LandmarkPointSets[InputSet].__len__()))
          DisplacementPoint = self.LandmarkPointSets[InputSet][DisplacementIndex]
          # Need to estimate direction of symmetric neighbor to this, and extrapolate
          # '-> use point labels? Lax assumption?
          RightLeftVector = self.EstimateRightLeftVector(DisplacementPoint, self.LandmarkPointSets[InputSet], self.RightLeftScale)
          AntPostVector = self.EstimateAntPostVector(DisplacementPoint, self.LandmarkPointSets[InputSet], self.AntPostScale)
          for dim in range(3):
            self.LandmarkPointSets[InputSet][DisplacementIndex][1][dim] += RightLeftVector[dim] + AntPostVector[dim]
          AlreadyDisplaced.append(self.LandmarkPointSets[InputSet][DisplacementIndex][0])
          DisplacementAmount -= 1
    
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
  def EstimateRightLeftVector(self, DisplacementPoint, LandmarkSet, RightLeftScale):
    for LabelPoint in LandmarkSet:
      #print DisplacementPoint[0]
      #print LabelPoint[0]
      #print " "
      if((DisplacementPoint[0][:-1] == LabelPoint[0][:-1]) and (DisplacementPoint[0] != LabelPoint[0])):
        # The DisplacementPoint has a symmetric partner
        
        RightLeftVector = [0,0,0]
        RightLeftVectorNorm = 0
        for dim in range(3):
          RightLeftVector[dim] = DisplacementPoint[1][dim] - LabelPoint[1][dim]
          RightLeftVectorNorm += (RightLeftVector[dim]) ** 2
        LeftRightVectorNorm = numpy.sqrt(RightLeftVectorNorm)
        for dim in range(3):
          RightLeftVector[dim] = RightLeftScale * (RightLeftVector[dim] / RightLeftVectorNorm)
        #print RightLeftVector
        return RightLeftVector
    # ASSERT that DisplacementPoint has no symmetric neighbor
  
  def EstimateAntPostVector(self, DisplacementPoint, LandmarkSet, AntPostScale):
    # Need normalized RightLeftVector for cross-product
    RightLeftVector = self.EstimateRightLeftVector(DisplacementPoint, LandmarkSet, 1)
    print " "
    print DisplacementPoint[0]
    #print " "
    LandmarkLabels = []
    for LandmarkPoint in range(LandmarkSet.__len__()):
      LandmarkLabels.append(LandmarkSet[LandmarkPoint][0]) 
    DisplacementPointIndex = LandmarkLabels.index(DisplacementPoint[0])
    AntPostVector = [0, 0, 0]
    InfSupVector = [0, 0, 0]
    if (not self.IsTopPoint(DisplacementPoint, LandmarkSet)):
      AbovePoint = self.ReturnNeighborAbove(DisplacementPoint, LandmarkSet)
      SupVector = [DisplacementPoint[1][0] - AbovePoint[1][0], DisplacementPoint[1][1] - AbovePoint[1][1], DisplacementPoint[1][2] - AbovePoint[1][2]]
      LengthSupVector = self.FindVectorLength(SupVector)
      for dim in range(3):
        SupVector[dim] = SupVector[dim] / LengthSupVector
      if(not self.IsBottomPoint(DisplacementPoint, LandmarkSet)):
        BelowPoint = self.ReturnNeighborBelow(DisplacementPoint, LandmarkSet)
        InfVector = [BelowPoint[1][0] - DisplacementPoint[1][0], BelowPoint[1][1] - DisplacementPoint[1][1], BelowPoint[1][2] - DisplacementPoint[1][2]]
        LengthInfVector = self.FindVectorLength(InfVector)
        for dim in range(3):
          InfVector[dim] = InfVector[dim] / LengthInfVector
          InfSupVector[dim] = (SupVector[dim] + InfVector[dim]) / 2
        AntPostVector = numpy.cross(RightLeftVector, InfSupVector)
        if(AntPostVector[1] < 0):   # if the offset is into the posterior direction (RAS = 012)
          for dim in range(3):
            AntPostVector[dim] = (-1) * AntPostVector[dim]  # Reflect the vector
        # Ensure normalization
        LengthAntPostVector = self.FindVectorLength(AntPostVector)
        for dim in range(3):
          AntPostVector[dim] = AntPostVector[dim] / LengthAntPostVector
        return AntPostVector * AntPostScale
      else:   # Displacement point is at bottom, but not top
        AntPostVector = numpy.cross(RightLeftVector, SupVector)
        if(AntPostVector[1] < 0):   # if the offset is into the posterior direction (RAS = 012)
          for dim in range(3):
            AntPostVector[dim] = (-1) * AntPostVector[dim]  # Reflect the vector
        # Ensure normalization
        LengthAntPostVector = self.FindVectorLength(AntPostVector)
        for dim in range(3):
          AntPostVector[dim] = AntPostVector[dim] / LengthAntPostVector
        return AntPostVector * AntPostScale
    else:   # Displacement point is at top, not necessarily bottom
      if(self.IsBottomPoint(DisplacementPoint, LandmarkSet)):  # DisplacementPoint is only one on its side
        print "ERROR - only one point on one side of spine, cannot compute anterior direction"
        print "   returning zero-AntPostVector"
        return [0,0,0]
      else: # DisplacementPoint is top point and not bottom point
        BelowPoint = self.ReturnNeighborBelow(DisplacementPoint, LandmarkSet)
        InfVector = [BelowPoint[1][0] - DisplacementPoint[1][0], BelowPoint[1][1] - DisplacementPoint[1][1], BelowPoint[1][2] - DisplacementPoint[1][2]]
        LengthInfVector = self.FindVectorLength(InfVector)
        for dim in range(3):
          InfVector[dim] = InfVector[dim] / LengthInfVector
        AntPostVector = numpy.cross(RightLeftVector, InfVector)
        if(AntPostVector[1] < 0):   # if the offset is into the posterior direction (RAS = 012)
          for dim in range(3):
            AntPostVector[dim] = (-1) * AntPostVector[dim]  # Reflect the vector
        # Ensure normalization
        LengthAntPostVector = self.FindVectorLength(AntPostVector)
        for dim in range(3):
          AntPostVector[dim] = AntPostVector[dim] / LengthAntPostVector
        return AntPostVector * AntPostScale
    
  def FindVectorLength(self, Vector):
    VectorNorm = 0 # initially
    for Elem in range(Vector.__len__()):
      VectorNorm += (Vector[Elem]) ** 2
    VectorNorm = numpy.sqrt(VectorNorm)
    return VectorNorm
    
    
  # Is___Point functions used to check for boundary conditions, to avoid referencing non-existent points in creating vectors  
  def IsTopPoint(self, Point, PointSet):
    LandmarkLabels = []
    for LandmarkPoint in range(PointSet.__len__()):
      LandmarkLabels.append(PointSet[LandmarkPoint][0]) 
    PointIndex = LandmarkLabels.index(Point[0])
    for PotentialNeighbor in range(0,PointIndex):
      if(PointSet[PotentialNeighbor][0][-1] == Point[0][-1]):   # If 'L' == 'L' or 'R' == 'R'
        return False
    return True
        
  def IsBottomPoint(self, Point, PointSet):
    LandmarkLabels = []
    for LandmarkPoint in range(PointSet.__len__()):
      LandmarkLabels.append(PointSet[LandmarkPoint][0]) 
    PointIndex = LandmarkLabels.index(Point[0])
    for PotentialNeighbor in range(PointIndex+1,PointSet.__len__()):
      if(PointSet[PotentialNeighbor][0][-1] == Point[0][-1]):   # If 'L' == 'L' or 'R' == 'R'
        return False
    return True
     

  # It is assumed that Point is known to have this neighbor if these functions are called
  def ReturnNeighborAbove(self, Point, PointSet):
    LandmarkLabels = []
    for LandmarkPoint in range(PointSet.__len__()):
      LandmarkLabels.append(PointSet[LandmarkPoint][0]) 
    PointIndex = LandmarkLabels.index(Point[0])
    PotentialNeighbor = PointSet[PointIndex-1]
    PotentialNeighborIndex = PointIndex - 1
    while(PotentialNeighbor[0][-1] != Point[0][-1]):
      PotentialNeighborIndex -= 1
      PotentialNeighbor = PointSet[PotentialNeighborIndex]
    # ASSERT PotentialNeighbor.label[-1] == Point.label[-1]
    return PotentialNeighbor
    
  def ReturnNeighborBelow(self, Point, PointSet):
    LandmarkLabels = []
    for LandmarkPoint in range(PointSet.__len__()):
      LandmarkLabels.append(PointSet[LandmarkPoint][0]) 
    PointIndex = LandmarkLabels.index(Point[0])
    PotentialNeighbor = PointSet[PointIndex + 1]
    PotentialNeighborIndex = PointIndex + 1
    while(PotentialNeighbor[0][-1] != Point[0][-1]):
      PotentialNeighborIndex += 1
      PotentialNeighbor = PointSet[PotentialNeighborIndex]
    # ASSERT PotentialNeighbor.label[-1] == Point.label[-1]
    return PotentialNeighbor

# Assumed to be used on undegraded LandmarkSet    
class CalculateAnglesLogic(ScriptedLoadableModuleLogic):
  import numpy
  
  def __init__(self):
    self.MarkupsNodes = slicer.util.getNodesByClass('vtkMRMLMarkupsFiducialNode')
    self.LandmarkLabels = []
    self.LandmarkPoints = []
    for LandmarkSet in range(self.MarkupsNodes.__len__()):
      CurrentLandmarkSet = self.MarkupsNodes.__getitem__(LandmarkSet)
      self.LandmarkLabels.append([])
      self.LandmarkPoints.append([])
      for LandmarkPoint in range(CurrentLandmarkSet.GetNumberOfFiducials()):
        self.LandmarkLabels[LandmarkSet].append(CurrentLandmarkSet.GetNthFiducialLabel(LandmarkPoint))
        self.LandmarkPoints[LandmarkSet].append(CurrentLandmarkSet.GetMarkupPointVector(LandmarkPoint,0))
    self.Angles = []
    self.MaxAngles = []
    self.MaxVertebrae = ""
      
  def CalculateAngles(self):
    for LandmarkSet in range(self.MarkupsNodes.__len__()):
      CurrentLandmarkSet = self.MarkupsNodes.__getitem__(LandmarkSet)
      self.Angles.append([])
      print LandmarkSet
      for Vertebra in range(0, CurrentLandmarkSet.GetNumberOfFiducials(), 2):
        VertebraLeft = CurrentLandmarkSet.GetMarkupPointVector(Vertebra, 0)
        VertebraRight = CurrentLandmarkSet.GetMarkupPointVector(Vertebra + 1, 0)
        LtoRVector = [VertebraRight[0] - VertebraLeft[0], 0,VertebraRight[2] - VertebraLeft[2]]
        LtoRVectorLength = self.FindVectorLength(LtoRVector)
        for dim in range(3):
          LtoRVector[dim] = LtoRVector[dim] / LtoRVectorLength
        Angle = numpy.arccos(numpy.dot(LtoRVector,[1,0,0])) * (180/numpy.pi)
        if(VertebraLeft[2] > VertebraRight[2]):
          Angle = -Angle  
        print Angle
        self.Angles[LandmarkSet].append(Angle)
    return self.Angles
      
  # returns maximum angle between any two vertbrae, measured from the left transvserse process 
  def FindMaxCoronalAngles(self):
    self.MinVertebra = []
    self.MaxVertebra = []
    self.MaxAngles = []
    for LandmarkSet in range(self.MarkupsNodes.__len__()):
      CurrentLandmarkSet = self.MarkupsNodes.__getitem__(LandmarkSet)
      MinAngle = 180
      MaxAngle = -180
      for Vertebra in range(0, CurrentLandmarkSet.GetNumberOfFiducials(), 2):
        if(self.Angles[LandmarkSet][Vertebra/2] < MinAngle):
          MinVertebra = self.LandmarkLabels[LandmarkSet][Vertebra][:-1]
          MinAngle = self.Angles[LandmarkSet][Vertebra/2]
        if(self.Angles[LandmarkSet][Vertebra/2] > MaxAngle):
          MaxVertebra = self.LandmarkLabels[LandmarkSet][Vertebra][:-1]
          MaxAngle = self.Angles[LandmarkSet][Vertebra/2]
      self.MinVertebra.append(MinVertebra)
      self.MaxVertebra.append(MaxVertebra)
      self.MaxAngle = MaxAngle - MinAngle
      self.MaxAngles.append(self.MaxAngle)
    self.MaxVertebrae = (self.MinVertebra, self.MaxVertebra)
    return (self.MaxAngles, self.MaxVertebrae)
  
  def FindVectorLength(self, Vector):
    VectorNorm = 0 # initially
    for Elem in range(Vector.__len__()):
      VectorNorm += (Vector[Elem]) ** 2
    VectorNorm = numpy.sqrt(VectorNorm)
    return VectorNorm
    
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
