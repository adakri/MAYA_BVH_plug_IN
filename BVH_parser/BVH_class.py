import re


class BvhNode:
    """
    Class to represent a node in the bvh file.
    """

    def __init__(self, name : str = None, offset : list = [], channels : list = [], parent=None):
        self._name = name
        self._offset = offset
        self._channels = channels
        self._children = []
        self._parent = parent
        if self._parent:
            self._parent.add_child(self)

    def add_child(self, item): # add another node
        item.parent = self
        self._children.append(item)

    def __iter__(self):
        for child in self.children:
            yield child

    def __getitem__(self, key : str):
        for child in self.children:
            for names in enumerate(child._name):
                if item == key:
                    if index + 1 >= len(child.value):
                        return None
                    else:
                        return child.value[index + 1:]
        raise IndexError('key {} not found'.format(key))

    def __repr__(self):
        return "Node: " + self._name


class HIERARCHY():
    def __init__(self, data):
        self._data = data
        self._root = BvhNode()
        self._nodes = []
        self._frames = []

    def read_listfile(self):
        first_round = []
        accumulator = ''
        print(self._data)
        for char in self._data:
            print(char)
            if char not in ('\n', '\r'):
                accumulator += char
                #print(accumulator)
            elif accumulator:
                first_round.append(re.split('\\s+', accumulator.strip()))
                #print(first_round[-1])
                accumulator = ''
        node_stack = [self._root]
        frame_time_found = False
        node = None
        for item in first_round:
            if frame_time_found:
                self.frames.append(item)
                continue
            key = item[0]
            if key == '{':
                node_stack.append(node)
            elif key == '}':
                node_stack.pop()
            else:
                node = BvhNode(item)
                node_stack[-1].add_child(node)
            if item[0] == 'Frame' and item[1] == 'Time:':
                frame_time_found = True

    def get_joints_names(self):
        joints = ""
        for i in self._nodes:
            joints += str(i._name)
        return joints
        


if __name__ == "__main__":
    FILE_NAME = "walk.bvh"
    bvh_file = open(FILE_NAME, 'r')
    bvh_f = HIERARCHY(list(bvh_file))
    bvh_f.read_listfile()

    bvh_file.close()
