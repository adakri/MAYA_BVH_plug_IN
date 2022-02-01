import numpy as np
import math 
import os

def calcul_matrice(q0, q1, q2, q3):
    phi = math.atan2((2*(q0*q1 + q2*q3)),(1-2*(q1**2+q2**2)))
    theta = math.asin(2*(q0*q2-q3*q1))
    psi = math.atan2((2*(q0*q3 + q1*q2)),(1-2*(q2**2+q3**2)))
    
    rotx = np.array([[1,0,0],[0, math.cos(phi), -math.sin(phi)],[0, math.sin(phi), math.cos(phi)]])
    roty = np.array([[math.cos(theta), 0, math.sin(theta)], [0,1,0], [-math.sin(theta), 0, math.cos(theta)]])
    rotz = np.array([[math.cos(psi), -math.sin(psi), 0], [math.sin(psi), math.cos(psi), 0], [0,0,1]])
    rot = rotz.dot(roty).dot(rotx)
    return rot

def recuperation(r):
    a = r.split("\t")
    a[-1] = a[-1].strip()
    q0 = float(a[-4])
    q1 = float(a[-3])
    q2 = float(a[-2])
    q3 = float(a[-1])
    return calcul_matrice(q0,q1,q2,q3)
    

def decomposition(M):
    phi2 = math.atan2(-M[1][2], M[2][2])
    theta2 = math.atan2(M[0][2], math.sqrt(1-M[0][2]))
    psi2 = math.atan2((math.cos(phi2)*M[1][0] + math.sin(phi2)*M[2][0]), (math.cos(phi2)*M[1][1] + math.sin(phi2)*M[2][1]))
    phi2 = phi2*180/(math.pi)
    theta2 = theta2*180/(math.pi)
    psi2 = psi2*180/(math.pi)
    
    return (psi2, theta2, phi2)
    

class TEST():
    def __init__(self, files):
        self._files = files

    def parse(self):
         
        f0 = open(self._files[0], "r")
        f1 = open(self._files[1],"r")
        f2 = open(self._files[2], "r")
        f3 = open(self._files[3], "r")
        f4 = open(self._files[4], "r")
        f5 = open(self._files[5],"r")
        f6 = open(self._files[6], "r")
        f7 = open(self._files[7], "r")
        f8 = open(self._files[8], "r")
        f9 = open(self._files[9],"r")
        f10 = open(self._files[10], "r")
        f11 = open(self._files[11], "r")
        
        r0 = f0.readlines()
        r1 = f1.readlines()
        r2 = f2.readlines()
        r3 = f3.readlines()
        r4 = f4.readlines()
        r5 = f5.readlines()
        r6 = f6.readlines()
        r7 = f7.readlines()
        r8 = f8.readlines()
        r9 = f9.readlines()
        r10 = f10.readlines()
        r11 = f11.readlines()
        
        
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
                a = r3[i].split("\t")
                a[-1] = a[-1].strip()
                q0 = float(a[-4])
                q1 = float(a[-3])
                q2 = float(a[-2])
                q3 = float(a[-1])
                
                b = r2[i].split("\t")
                b[-1] = b[-1].strip()
                p0 = float(b[-4])
                p1 = float(b[-3])
                p2 = float(b[-2])
                p3 = float(b[-1])
                
                
                rot0 = recuperation(r0[i]) #avantbrasd
                rot1 = recuperation(r1[i]) # avantbrasg
                rot2 = recuperation(r2[i]) # bicepsd
                rot3 = recuperation(r3[i]) # bicepsg
                rot4 = recuperation(r4[i]) # cuissed
                rot5 = recuperation(r5[i]) # cuisseg
                rot6 = recuperation(r6[i]) # maind
                rot7 = recuperation(r7[i]) # maing
                rot8 = recuperation(r8[i]) # molletd
                rot9 = recuperation(r9[i]) # molletg
                rot10 = recuperation(r10[i]) # pelvis
                rot11 = recuperation(r11[i]) # tete
                  
                #biceps gauche rot3
                phi = math.atan2((2*(q0*q1 + q2*q3)),(1-2*(q1**2+q2**2)))
                theta = math.asin(2*(q0*q2-q3*q1))
                psi = math.atan2((2*(q0*q3 + q1*q2)),(1-2*(q2**2+q3**2)))
                
                phi = phi*180/math.pi
                theta = theta*180/math.pi
                psi = psi*180/math.pi
                
                #biceps droit rot2
                phi2 = math.atan2((2*(p0*p1 + p2*p3)),(1-2*(p1**2+p2**2)))
                theta2 = math.asin(2*(p0*p2-p3*p1))
                psi2 = math.atan2((2*(p0*p3 + p1*p2)),(1-2*(p2**2+p3**2)))
                
                phi2 = phi2*180/math.pi
                theta2 = theta2*180/math.pi
                psi2 = -psi2*180/math.pi
               
                rot0transpose = np.transpose(rot0)
                rot1transpose = np.transpose(rot1)
                rot2transpose = np.transpose(rot2)
                rot3transpose = np.transpose(rot3)
                rot6transpose = np.transpose(rot6)
                rot7transpose = np.transpose(rot7)
                
                M1 = rot2transpose.dot(rot0)
                M2 = rot0transpose.dot(rot6)
                M3 = rot3transpose.dot(rot1)
                M4 = rot1transpose.dot(rot7)
                
            
                
                A = [(psi2,theta2,phi2)]
                A.append(decomposition(M1))
                A.append(decomposition(M2))
                A.append((psi,theta,phi))
                A.append(decomposition(M3))
                A.append(decomposition(M4))
                
         
                
      
                
                file_to_write.write("0 0 0 0 0 0")
                file_to_write.write(" ")
                
                for element in A:
                    file_to_write.write(str(element[0]))
                    file_to_write.write(" ")
                    file_to_write.write(str(element[1]))
                    file_to_write.write(" ")
                    file_to_write.write(str(element[2]))
                    file_to_write.write(" ")
                
                
                file_to_write.write("\n")
            i+=1
        file_to_write.close()
        
        print(i)
        
if __name__ == "__main__":
    FILE_NAME="C:\\Users\\Arthur\\Documents\\ingenierie3d\\maya_bvh_plugin\\BVH_parser\\resultat_triple_capteur1.txt"
    FILE_NAME2="C:\\Users\\Arthur\\Documents\\ingenierie3d\\maya_bvh_plugin\\BVH_parser\\resultat_triple_capteur2.txt"
    FILE_NAME3 = "C:\\Users\\Arthur\\Documents\\ingenierie3d\\maya_bvh_plugin\\BVH_parser\\resultat_triple_capteur3.txt"
    files = []
    for i in os.listdir("squelette"):
        files.append("C:\\Users\\Arthur\\Documents\\ingenierie3d\\maya_bvh_plugin\\BVH_parser\\squelette\\" + i)
        
    go = TEST(files)
    go.parse()