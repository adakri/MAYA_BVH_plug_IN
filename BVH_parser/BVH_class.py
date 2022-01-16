import re
from Parser import *
from Skeleton import *
import os.path


class BvhNode:
    """
    Class torepresent a node in the bvh file.
    """

    def __init__(self,  name, offset=[], channels=[], parent=None):
        self._offset = offset
        self._channels = channels
        self._children = []
        self._parent = parent
        self._name = name
        if self._parent:
            self._parent.add_child(self)

    def add_child(self, item):
        item._parent = self
        self._children.append(item)

    def __iter__(self):
        for child in self._children:
            yield child

    def __getitem__(self, key):
        for child in self._children:
            for index, item in enumerate(child.value):
                if item == key:
                    if index + 1 >= len(child.value):
                        return None
                    else:
                        return child.value[index + 1:]
        raise IndexError('key {} not found'.format(key))

    def __repr__(self):
        return str(' '.join(self.value))

    def objPath(self):
        # return object path 
        if self._parent is not None:
            # Ah recusrsion, my old enemy
            # the object path I copied from somewhere
            # Would have never guessed it
            return "%s|%s" % (self.parent.objPath(), self.__str__())
        return str(self.name)


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

        frame = 0

        rotOrder = 0

        channel_list = []

        f = open(self._filename, 'r')

        if f.next().startswith("HIERARCHY"):
            print("=========== Found hierarchy ==========")
        else:
            pass

        CLOSE = False
        
        for line in f:
            # Start parsing, we are at Hierarchy here, I hope
            if not motion_frame:
                # root joint
                if line.startswith("ROOT"):
                    # update the tree 
                    self._root = BvhNode(line[5:].rstrip())
                    current_parent = self._root

                if "MOTION" in line:
                    # Break
                    motion_frame = True		

                if "CHANNELS" in line:
                    curr_chan = line.strip().split(" ")
                    # Adapt the channel rotations
                    chan_nb_param = int(curr_chan[1]) 
                    for nb in range(chan_nb_param):
                        current_parent._channels.append(curr_chan[nb + 1])
                        
                if "OFFSET" in line:
                    curr_offset = line.strip().split(" ")
                    joint_name = str(current_parent)
                    if CLOSE:
                        joint_name = joint_name + "_end" # add _end to print
                    
                    current_parent._offset = [float(curr_offset[1]), float(curr_offset[2]), float(curr_offset[3])]

                if "JOINT" in line:
                    new_joint = line.split(" ") 
                    # initiate with old parent
                    current_parent = BvhNode(new_joint[-1].rstrip(), current_parent)

                if "End" in line:
                    CLOSE = True 
                    
                # return 
                if "}" in line:
                    if CLOSE:
                        CLOSE = False
                        continue
                        
                    if current_parent._parent is not None:
                        current_parent = current_parent._parent
            else:
                if "Frame" not in line:
                            data = line.split(" ")
                            if len(data) > 0:
                                if data[0] == "":
                                    data.pop(0) 
                            
                            
                            frame = frame + 1

    def __repr__(self):
        representation = ""
        for i in self._children:
            representation = representation + str(repr(i))
        return representation

    def get_joints_names(self):
        joints = []

        def iterate_joints(joint):
            joints.append(joint.value[1])
            for child in joint.filter('JOINT'):
                iterate_joints(child)
        iterate_joints(next(self.root.filter('ROOT')))
        return joints


if __name__ == "__main__":

    data_folder = os.path.join("C:", "Users", "icasi","Documents", "ANIMATION_3D", "ANIMATION_3D_project" )

    print(data_folder)

    FILE_NAME = os.path.join(data_folder, "walk.bvh")

    print(FILE_NAME)
    

    bvh_f = HIERARCHY(FILE_NAME)
    bvh_f.parse()

    print(bvh_f)

    # list = bvh_f.get_joints_names()

    bvh_file.close()
