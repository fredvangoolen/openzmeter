#Cad Studio Clock (2016-01-30)
# execfile("/home/doug/svn/Elektronik/KiCad/StudioClockFull/makeClock.py")
import codecs
import numpy
import pcbnew
import math 
import sys
import collections

pcb = pcbnew.GetBoard()
Radius = 44
OriginOffsetXY = [100, 100]
#Some Constants for better Readability
LayerBCu = 31
LayerFCu = 0
LayerEdgeCuts = 44
AngleInc=0.97
Width=10
Margin=1
LineNo=0
Offset=0.26

#calc rotation angle (rad) with Position: float -> float
def calcRad(pos):
	return (pos*math.pi)/180.0
#calc the Position(s) with the Radius and an Angle: float, float -> float/float/wxPoint
def calcX(radius, pos):
	return math.sin(calcRad(pos))*(radius)+OriginOffsetXY[0]
def calcY(radius, pos):
	return -math.cos(calcRad(pos))*radius+OriginOffsetXY[1]
def calcXY(radius, pos):
	return pcbnew.wxPointMM(calcX(radius,pos),calcY(radius,pos))
#add a track and add it: wxPoint, wxPoint, int, int -> Track
def addTrack(startPos, stopPos, net, layer):
	t = pcbnew.TRACK(Pcb)
	pcb.Add(t)
	t.SetStart(startPos)
	t.SetEnd(stopPos)
	t.SetNetCode(net)
	t.SetLayer(layer)
	return t

#Deleting all Nets and Drawings is esier than finding exisitng ones
for t in pcb.GetTracks():
	pcb.Delete(t)

for d in pcb.GetDrawings():
	pcb.Remove(d)

for i in numpy.arange(0,360,1):
	seg = pcbnew.DRAWSEGMENT(pcb)
	pcb.Add(seg)
	seg.SetStart(calcXY(Radius, i))
	seg.SetEnd(calcXY(Radius, i+1))
	seg.SetLayer(LayerEdgeCuts)
	
	seg = pcbnew.DRAWSEGMENT(pcb)
	pcb.Add(seg)
	seg.SetStart(calcXY(Radius-Width-2*Margin-Offset, i))
	seg.SetEnd(calcXY(Radius-Width-2*Margin-Offset, i+1))
	seg.SetLayer(LayerEdgeCuts)

	t = pcbnew.TRACK(pcb)
	pcb.Add(t)
	t.SetStart(calcXY(Radius-Width-Margin-Offset-0.53, i))
	t.SetEnd(calcXY(Radius-Width-Margin-Offset-0.53, i+1))
	t.SetLayer(LayerBCu)

for i in numpy.arange(0,360,AngleInc):
	t = pcbnew.TRACK(pcb)
	pcb.Add(t)
	t.SetStart(calcXY(Radius-Margin, i))
	if LineNo % 2 == 0:
		t.SetEnd(calcXY(Radius-Margin-Width-Offset, i+(AngleInc/2)))
	else:
		t.SetEnd(calcXY(Radius-Margin-Width+Offset, i+(AngleInc/2)))
	t.SetLayer(LayerFCu)
	
	v = pcbnew.VIA(pcb)
	pcb.Add(v)
	if LineNo % 2 == 0:
		v.SetPosition(calcXY(Radius-Margin-Width-Offset, i+(AngleInc/2)))
	else:
		v.SetPosition(calcXY(Radius-Margin-Width+Offset, i+(AngleInc/2)))
	v.SetViaType(pcbnew.VIA_THROUGH)
	v.SetDrill(pcbnew.FromMM(0.3))
	v.SetWidth(pcbnew.FromMM(0.6))
	v.SetLayerPair(LayerFCu, LayerBCu)

	v = pcbnew.VIA(pcb)
	pcb.Add(v)
	v.SetPosition(calcXY(Radius-Margin, i+(AngleInc)))
	v.SetViaType(pcbnew.VIA_THROUGH)
	v.SetDrill(pcbnew.FromMM(0.3))
	v.SetWidth(pcbnew.FromMM(0.6))
	v.SetLayerPair(LayerFCu, LayerBCu)

	t = pcbnew.TRACK(pcb)
	pcb.Add(t)
	if LineNo % 2 == 0:
		t.SetStart(calcXY(Radius-Margin-Width-Offset, i+(AngleInc/2)))
	else:
		t.SetStart(calcXY(Radius-Margin-Width+Offset, i+(AngleInc/2)))
	t.SetEnd(calcXY(Radius-Margin, i+AngleInc))
	t.SetLayer(LayerBCu)
	LineNo = LineNo+1
pcbnew.Refresh()
print '---The-End-----------------------------------------------------'














#calc ratation angle (deg) with Position: float -> float 
#def calcDeg(pos):
#	return math.degrees(calcRad(pos))
#add an arc of tracks with the Position Idices of the SecondsLeds: float, int, int, int, int -> Track
#def addTrackArc(radius, startPos, stopPos, net, layer):		
#	t = None
#	for i in range(startPos,stopPos-1):
#		t = addTrack(calcXY(radius, i), calcXY(radius, i+1), net, layer)
#	return t

#add a full Track ring with the desired Radius: float, int, int
#def addTrackRing(radius, net, layer):
#	addTrackArc(radius, 0, 61, net, layer)

#add a via at the Position: wxPoint -> Via
#def addVia(position, net):
#	v = pcbnew.VIA(pcb)
#	pcb.Add(v)
#	v.SetPosition(position)
#	#v.SetWidth(600000)
#	v.SetViaType(pcbnew.VIA_THROUGH)
#	v.SetLayerPair(LayerFCu, LayerBCu)
#	v.SetNetCode(net)
#	return v

#Some Constants for better Readability
#LayerBCu = 31
#LayerFCu = 0
#LayerEdgeCuts = 44

#Deleting all Nets and Drawings is esier than finding exisitng ones
#print '---------------------------------------------------------------'
#print '---Delete-Exisitng-Nets-and Drawings---------------------------'
#print '---------------------------------------------------------------'
#for t in pcb.GetTracks():
#        pcb.Delete(t)
#for d in pcb.GetDrawings():
#	pcb.Remove(d)

#Find all Diods and save often needed Information or Objects in Array
#Sort them into HourRing and Secods Ring
#Find all Nets add them to their Netgroup list.
#print '---------------------------------------------------------------'
#print '---Gathering-Diods-and-Info------------------------------------'
#print '---------------------------------------------------------------'
#for modu in pcb.GetModules():
#	ref = modu.GetReference().encode('utf-8')
#	if(ref.startswith('D')):
#		pos = int(ref.split('D')[-1])-1
#		pad1 = None	#kathode
#		net1 = None
#		pad2 = None	#anode
#		net2 = None
#		for pad in modu.Pads():
#			if int(pad.GetPadName()) == 1:
#				pad1 = pad
#				net1 = pad.GetNetCode()
#				if pos <= 59:
#					if net1 not in NetsB:
#						NetsB.append(net1)
#			else:
#				pad2 = pad
#				net2 = pad.GetNetCode()
#				if pos <= 59:
#					if net2 not in NetsA:
#						NetsA.append(net2)
#				elif pos <= 71 and NetC == None:
#					NetC = net2	
#		if pos <= 59:
#			MatSRing[pos] = [pos, modu, pad1, net1, pad2, net2, ref]
#			print 'Read: Second %s,  Position %d, Net1 %d, Net2 %d' % (MatSRing[pos][6], MatSRing[pos][0], net1, net2)
#		elif pos <= 71:
#			pos = pos%60
#			MatHRing[pos] = [pos, modu, pad1, net1, pad2, net2, ref]
#			print 'Read: Hour %s, Poition %d, Net1 %d, Net2 %d' % (ref, MatHRing[pos][0], net1, net2)
#print '-----------------------------------------------------'
#print 'NetsA:', NetsA
#print 'NetsB:', NetsB
#print 'NetC :', NetC
#print '---------------------------------------------------------------'
#print '---Build-Net---------------------------------------------------'
#print '---------------------------------------------------------------'
#print '---------------------------------------------------------------'
#print '---Set Edge Cut------------------------------------------------'
#print '---------------------------------------------------------------'
#print "Setting Board Dimensions too:"
#corners = [[-1,-1],[-1,1],[1,1],[1,-1]]
#l = Radius*1.2
#for i in range(4):
#	seg = pcbnew.DRAWSEGMENT(pcb)
#	pcb.Add(seg)
#	seg.SetStart(pcbnew.wxPointMM(corners[i][0]*l, corners[i][1]*l))
#	seg.SetEnd(pcbnew.wxPointMM(corners[(i+1)%4][0]*l, corners[(i+1)%4][1]*l))
#	seg.SetLayer(LayerEdgeCuts)
#	print "Corner:", seg.GetStart()
#
#print '---------------------------------------------------------------'
#print '---The-End-----------------------------------------------------'
#print '---------------------------------------------------------------'
