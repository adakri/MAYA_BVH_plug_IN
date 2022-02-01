import re
#from Parser import *
#import Skeleton as skeleton
import os.path

def parse_hierarchy_motion(file):
    hierarchy = []
    motion = []
    bvh_file = [hierarchy, motion]

    i = 0
    for line in file:

        if("MOTION" in line):
            i = 1
        bvh_file[i].append(line)
    return hierarchy, motion

# This maps the BVH naming convention to Maya
translationDict = {
    "Xposition": "translateX",
    "Yposition": "translateY",
    "Zposition": "translateZ",
    "Xrotation": "rotateX",
    "Yrotation": "rotateY",
    "Zrotation": "rotateZ"
}


import pymel.core as pm # use pm
import maya.cmds as mc # maya commands


class BvhNode:
    """
    Class to represent a node in the bvh file.
    """

    def __init__(self,  name, offset=[], channels=[], parent=None):
        self._offset = offset
        self._channels = channels
        self._children = []
        self._parent = parent
        self._name = name

    def __str__(self):
        return "Node: " + self._name + "\n" + str(self._offset) + "\n" + str(self._channels) + "\n" +"======================================================="


    def objPath(self):
        # return object path 
        if self._parent is not None:
            # Ah recusrsion, my old enemy
            # the object path I copied from somewhere
            # Would have never guessed it, it is a format
            return self._parent.objPath()+"|"+ self._name
        return self._name


class HIERARCHY():
    def __init__(self, filename):
        self._filename = filename
        self._root = BvhNode(None)
        self._frames = []
        self._node_stack = []

    """ For now, No regards to SE, I'll do the parsing in one
    function and sort it afterwards """

    def parse(self):
        rootNode = True  # root node

        current_parent = None
        # Turn true the time we find Motion
        motion_frame = False

        frame = 0 # frame counter

        rotation = 0 # variable to store rotations relative to the list below

        channel_list = [] # channels with regards to the node structure

        f = open(self._filename, 'r')

        if rootNode is None: # if we are here for the first time 
            group = pm.group(em=True,name="_mocap_"+self._filename+"_grp")
            group.scale.set(1.0, 1.0, 1.0) # scale 1.0

            # The group is now the 'root'
            curr_parent = BvhNode(str(group), None)
        else:
            curr_parent = BvhNode(str(rootNode), None)
            #self._clear_animation()


        CLOSE = False
        
        for line in f:
            if line.startswith("HIERARCHY"):
                print("=========== Found hierarchy ==========")
            else:
                pass
            
            # Start parsing, we are at Hierarchy here, I hope
            if not motion_frame:
                # root joint
                if line.startswith("ROOT"):
                    # update the tree 
                    print("Found root")
                    current_parent= BvhNode(line[5:].rstrip())
                    self._root = current_parent

                    
                
                if "MOTION" in line:
                    print("Found Motion")
                    # Break
                    motion_frame = True		

                if "CHANNELS" in line:
                    print("Found channels")
                    current_parent._channels = []
                    curr_chan = line.strip().split(" ")
                    # Adapt the channel rotations
                    chan_nb_param = int(curr_chan[1])
                    print(current_parent.objPath()) 
                    for nb in range(chan_nb_param):
                        current_parent._channels.append(curr_chan[nb+2])
                        #print(curr_chan[nb+2])
                        channel_list.append(current_parent.objPath()+"."+translationDict[curr_chan[nb+2]]) # create full name, used later 
                        #print("***",channel_list[-1])
                    #print(current_parent._channels)
                        
                if "OFFSET" in line:
                    print("Found offset")
                    curr_offset = line.strip().split(" ")
                    
                    current_parent._offset = [float(curr_offset[1]), float(curr_offset[2]), float(curr_offset[3])]
                    
                    # add end at the end with end site
                    if(CLOSE):
                        current_parent._name = current_parent._name + "_end" 
                    
                    
                    #===========================================
                    # expermenting with pymel here we are at the end of an objject, we have to instantiate it in maya (fingers crossed)
                    
                    objpath = current_parent.objPath()
    				# create object by specifying fullpath using pymel module (autodesk tutorials)
                    if mc.objExists(str(current_parent.objPath())):
                        # already exists, don't create it just translate it (if in ather frames, any better ideas, this seems not optimal)
                        curr_joint = pm.PyNode(objpath)
                        curr_joint.rotateOrder.set(rotation)
                        curr_joint.translate.set([float(current_parent._offset[-1]), float(current_parent._offset[-2]), float(current_parent._offset[-3])])
                        continue
                    # create the joint
                    curr_joint = pm.joint(name=current_parent._name, p=(0,0,0))
                    curr_joint.translate.set([float(current_parent._offset[0]), float(current_parent._offset[1]), float(current_parent._offset[2])])
                    curr_joint.rotateOrder.set(rotation)
                    
                if "JOINT" in line:
                    print("Found joint")
                    new_joint = line.split(" ") 
                    # initiate with old parent and add to children
                    old_parent  = current_parent
                    # switch to newest children
                    current_parent = BvhNode(new_joint[-1].rstrip())
                    current_parent._parent = old_parent
                    
                    old_parent._children.append(current_parent)

                if "End" in line:
                    print("Found end")
                    CLOSE = True 
                    
                # return 
                if "}" in line:
                    print("Found {}")
                    if CLOSE:
                        CLOSE = False
                        continue
                    # walk up  
                    if current_parent._parent is not None:
                        current_parent = current_parent._parent
                        if current_parent is not None: # if still not None select it
                            mc.select(current_parent.objPath())
            else:
                print("IN MOTION")
                
                if "Frame" not in line:
                    # the first two lines are not relevant to my knowledge
                    data = line.split(" ")
                    if len(data) > 0:
                        if data[0] == "":
                            data.pop(0) 
                    
                    print(channel_list)
                    print(data)
                    
                    # Set the values to channels
                    for x in range(0, len(data) - 1 ): # for all keyframes
                        mc.setKeyframe(channel_list[x],time=frame, value=float(data[x])) # add keyframe, easy in maya, need animcurve in c++                  
                    
                    frame = frame + 1
    def __str__(self):
        representation = ""
        print("================Printing the BVH file===============")
        pointer = self._root
        while(len(pointer._children) != 0):
            if(pointer == None):
                break
            else:
                representation += str(print(pointer))
                pointer = pointer._children[0]
        return representation

    def get_joints_names(self):
        joints = ""
        for i in self._nodes:
            joints += str(i._name)
        return joints
        


if __name__ == "__main__":

    FILE_NAME = "C:\\Users\\Arthur\\Documents\\ingenierie3d\\maya_bvh_plugin\\BVH_parser\\squelette4.txt"

    print(FILE_NAME)
    

    bvh_f = HIERARCHY(FILE_NAME)
    bvh_f.parse()
    
    #print(bvh_f)

