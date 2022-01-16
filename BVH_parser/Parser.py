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


if __name__ == "__main__":
    FILE_NAME = "walk.bvh"
    bvh_file = open(FILE_NAME, 'r')

    print(parse_hierarchy_motion(bvh_file)[0])

    bvh_file.close()
