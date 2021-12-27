import BVH_class as bvh


def parse_hierarchy(file):
    iterator = 0
    H = []
    while(file.readline() != "HIERARCHY"):
        print("The hierarchy was found at line ", iterator)
        iterator += 1
    while(file.readline() != "MOTION"):
        list = file.readline().split()
        if(list[0] == "ROOT"):
            print("")


if __name__ == "__main__":
    FILE_NAME = "walk.bvh"
    bvh_file = open(FILE_NAME, 'r')
    print(list(bvh_file))
    bvh_f = bvh.BVH(list(bvh_file))

    bvh_file.close()
