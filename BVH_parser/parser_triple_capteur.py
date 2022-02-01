import numpy as np
import math 

def calcul_matrice(q0, q1, q2, q3):
    phi = math.atan2((2*(q0*q1 + q2*q3)),(1-2*(q1**2+q2**2)))
    theta = math.asin(2*(q0*q2-q3*q1))
    psi = math.atan2((2*(q0*q3 + q1*q2)),(1-2*(q2**2+q3**2)))
    
    rotx = np.array([[1,0,0],[0, math.cos(phi), -math.sin(phi)],[0, math.sin(phi), math.cos(phi)]])
    roty = np.array([[math.cos(theta), 0, math.sin(theta)], [0,1,0], [-math.sin(theta), 0, math.cos(theta)]])
    rotz = np.array([[math.cos(psi), -math.sin(psi), 0], [math.sin(psi), math.cos(psi), 0], [0,0,1]])
    rot = rotz.dot(roty).dot(rotx)
    return rot

def decomposition(M):
    phi2 = math.atan2(-M[1][2], M[2][2])
    theta2 = math.atan2(M[0][2], math.sqrt(1-M[0][2]))
    psi2 = math.atan2((math.cos(phi2)*M[1][0] + math.sin(phi2)*M[2][0]), (math.cos(phi2)*M[1][1] + math.sin(phi2)*M[2][1]))
    phi2 = phi2*180/(math.pi)
    theta2 = theta2*180/(math.pi)
    psi2 = psi2*180/(math.pi)
    
    return (psi2, theta2, phi2)
    

class TEST():
    def __init__(self, filename, filename2, filename3):
        self._filename = filename
        self._filename2 = filename2
        self._filename3 = filename3

    def parse(self):
        f = open(self._filename, "r")
        f2 = open(self._filename2,"r")
        f3 = open(self._filename3, "r")
        r1 = f.readlines()
        r2 = f2.readlines()
        r3 = f3.readlines()
        
        
        start = False
        file_to_write = open("futur_bvh.txt", "w")
        i = 0
        for line in r1:
            
            if line[0]=="P":
                start = True
                print("===FOUND START OF FILE===")
            else:
                pass
            if start and not line[0]=="P":
                
                
                a = r1[i].split("\t")
                a[-1] = a[-1].strip()
                
                
                b = r2[i].split("\t")
                b[-1] = b[-1].strip()
                
                c = r3[i].split("\t")
                c[-1] = c[-1].strip()
                
                
                q0 = float(a[-4])
                q1 = float(a[-3])
                q2 = float(a[-2])
                q3 = float(a[-1])
                
                p0 = float(b[-4])
                p1 = float(b[-3])
                p2 = float(b[-2])
                p3 = float(b[-1])
                
                s0 = float(c[-4])
                s1 = float(c[-3])
                s2 = float(c[-2])
                s3 = float(c[-1])
                
                rot3 = calcul_matrice(s0, s1, s2, s3)
                rot2 = calcul_matrice(p0, p1, p2, p3)
                rot1 = calcul_matrice(q0, q1, q2, q3)
                
                
        
#                phi2 = math.atan((2*(p0*p1 + p2*p3))/(1-2*(p1**2+p2**2)))
#                theta2 = math.asin(2*(p0*p2-p3*p1))
#                psi2 = math.atan((2*(p0*p3 + p1*p2))/(1-2*(p2**2+p3**2)))
#                
#                rotx2 = np.array([[1,0,0],[0, math.cos(phi2), -math.sin(phi2)],[0, math.sin(phi2), math.cos(phi2)]])
#                roty2 = np.array([[math.cos(theta2), 0, math.sin(theta2)], [0,1,0], [-math.sin(theta2), 0, math.cos(theta2)]])
#                rotz2 = np.array([[math.cos(psi2), -math.sin(psi2), 0], [math.sin(psi2), math.cos(psi2), 0], [0,0,1]])
#                rot2 = rotz2.dot(roty2).dot(rotx2)
#                rot2 = np.transpose(rot2)
                
                
                phi = math.atan2((2*(q0*q1 + q2*q3)),(1-2*(q1**2+q2**2)))
                theta = math.asin(2*(q0*q2-q3*q1))
                psi = math.atan2((2*(q0*q3 + q1*q2)),(1-2*(q2**2+q3**2)))
#                
#                rotx = np.array([[1,0,0],[0, math.cos(phi), -math.sin(phi)],[0, math.sin(phi), math.cos(phi)]])
#                roty = np.array([[math.cos(theta), 0, math.sin(theta)], [0,1,0], [-math.sin(theta), 0, math.cos(theta)]])
#                rotz = np.array([[math.cos(psi), -math.sin(psi), 0], [math.sin(psi), math.cos(psi), 0], [0,0,1]])
#                rot = rotz.dot(roty).dot(rotx)
#                
#                M = rot2.dot(rot)
#                if(i==300):
#                    print(M)
#                phi2 = math.atan2(-M[1][2], M[2][2])
#                theta2 = math.atan2(M[0][2], math.sqrt(1-M[0][2]))
#                psi2 = math.atan2((math.cos(phi2)*M[1][0] + math.sin(phi2)*M[2][0]), (math.cos(phi2)*M[1][1] + math.sin(phi2)*M[2][1]))
#                
                phi = phi*180/math.pi
                theta = theta*180/math.pi
                psi = psi*180/math.pi
#                
#                phi2 = phi2*180/(math.pi)
#                theta2 = theta2*180/(math.pi)
#                psi2 = psi2*180/(math.pi)
                rot1transpose = np.transpose(rot1)
                rot2transpose = np.transpose(rot2)
                rot3transpose = np.transpose(rot3)
                
                
                M1 = rot2transpose.dot(rot1)
                M2 = rot3transpose.dot(rot2)
                
                psi2, theta2, phi2 = decomposition(M1)
                psi3, theta3, phi3 = decomposition(M2)
                
                file_to_write.write("0 0 0 0 0 0")
                file_to_write.write(" ")
                file_to_write.write(str(psi))
                file_to_write.write(" ")
                file_to_write.write(str(theta))
                file_to_write.write(" ")
                file_to_write.write(str(phi))
                file_to_write.write(" ")
                file_to_write.write(str(psi2))
                file_to_write.write(" ")
                file_to_write.write(str(theta2))
                file_to_write.write(" ")
                file_to_write.write(str(phi2))
                file_to_write.write(" ")
                file_to_write.write(str(psi3))
                file_to_write.write(" ")
                file_to_write.write(str(theta3))
                file_to_write.write(" ")
                file_to_write.write(str(phi3))
                file_to_write.write(" ")
                file_to_write.write("0 0 0")
                file_to_write.write("\n")
            i+=1
        file_to_write.close()
        f2.close()
        f.close()
        print(i)
        
if __name__ == "__main__":
    FILE_NAME="C:\\Users\\Arthur\\Documents\\ingenierie3d\\maya_bvh_plugin\\BVH_parser\\resultat_triple_capteur1.txt"
    FILE_NAME2="C:\\Users\\Arthur\\Documents\\ingenierie3d\\maya_bvh_plugin\\BVH_parser\\resultat_triple_capteur2.txt"
    FILE_NAME3 = "C:\\Users\\Arthur\\Documents\\ingenierie3d\\maya_bvh_plugin\\BVH_parser\\resultat_triple_capteur3.txt"
    
    go = TEST(FILE_NAME, FILE_NAME2, FILE_NAME3)
    go.parse()