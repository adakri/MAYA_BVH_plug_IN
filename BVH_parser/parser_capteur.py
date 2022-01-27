import numpy as np
import math 


class TEST():
    def __init__(self, filename):
        self._filename = filename

    def parse(self):
        f = open(self._filename, "r")
        start = False
        file_to_write = open("futur_bvh.txt", "w")
        i = 0
        for line in f:
            if line.startswith("PacketCounter"):
                start = True
                print("===FOUND START OF FILE===")
            else:
                pass
            if start and not line.startswith("PacketCounter"):
                
                line = line.split('\t')
                line[-1] = line[-1].strip()
                print(line)
                q0 = float(line[-4])
                q1 = float(line[-3])
                q2 = float(line[-2])
                q3 = float(line[-1])
                phi = math.atan((2*(q0*q1 + q2*q3))/(1-2*(q1**2+q2**2)))
                theta = math.asin(2*(q0*q2-q3*q1))
                psi = math.atan((2*(q0*q3 + q1*q2))/(1-2*(q2**2+q3**2)))
                phi = phi*180/math.pi
                theta = theta*180/math.pi
                psi = psi*180/math.pi
                file_to_write.write("0 0 0 0 0 0")
                file_to_write.write(" ")
                file_to_write.write(str(phi))
                file_to_write.write(" ")
                file_to_write.write(str(theta))
                file_to_write.write(" ")
                file_to_write.write(str(psi))
                file_to_write.write(" ")
                file_to_write.write("0 0 0")
                file_to_write.write("\n")
                i+=1
        file_to_write.close()
        f.close()
        





if __name__ == "__main__":
    FILE_NAME="D:\\téléchargements\\Ensimag\\maya\\resultat_capteur_1.txt"
    go = TEST(FILE_NAME)
    go.parse()