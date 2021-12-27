import pymel.core as pm
import maya.cmds as mc
import os

# This maps the BVH naming convention to Maya
translationDict = {
    "Xposition": "translateX",
    "Yposition": "translateY",
    "Zposition": "translateZ",
    "Xrotation": "rotateX",
    "Yrotation": "rotateY",
    "Zrotation": "rotateZ"
}

print("here")
