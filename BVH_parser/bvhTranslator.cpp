//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

////////////////////////////////////////////////////////////////////////
// DESCRIPTION:
//
// Adds the new file format Bvh to the file manipulation dialogs.
// 
// As soon as this plug-in is loaded, the new file format will be available in
// the "Open", "Import, and "Export" dialogs.
//
// The icon that is displayed in the file selection boxes is contained in the
// file "BvhTranslator.rgb", which is also located in the example
// plug-in directory. Maya will find this icon as long as the path to the
// directory that contains it is included in the FILE_ICON_PATH environment variable.
//
// A "Bvh" file is an ASCII file with a first line of "<Bvh>".
// The remainder of the file contains MEL commands that create one of
// these primitives: nurbsSphere, nurbsCone, and nurbsCylinder, as well as move
// commands to position them.
//
// When writing the file, only primitives of these three types will be created
// along with their positions in 3D space. The reader routine will actually handle
// more MEL commands than these, but only this limited set of types will be written.
//
// Additionally, this example demonstrates how to utilize file options.
// When saving a file, if you click on the option box beside the
// File > Export All menu item, a dialog appears that contains two radio boxes asking
// whether to "Write Positions". The default is true, and if false is selected, then the
// move commands for primitives will not be written to the output file. This dialog is
// implemented by the MEL script "BvhTranslatorOpts.mel", which is also located in
// the plug-in directory.
//
// A sample input file is supplied in the example plug-in directory as "BvhTranslator.Bvh".
//  
// This example plugin demonstrates how to implement a Maya File Translator.
// The Bvh files can be referenced by Maya files.
//  
// Note that this is a simple example.  Hence, there are limitations.
// For example, every geometry saved will have its values reset to default,
// except their translation if the option "Show Position" has been turned on. To find what 
// geometries we can export, we search them by name. Hence, if a polygon cube contains in its 
// name the string "nurbsSphere", it will be written out as a nurbs sphere.
//
////////////////////////////////////////////////////////////////////////


#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MFnPlugin.h>
#include <maya/MString.h>
#include <maya/MVector.h>
#include <maya/MStringArray.h>
#include <maya/MPxFileTranslator.h>
#include <maya/MGlobal.h>
#include <maya/MItDag.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MItSelectionList.h>
#include <maya/MSelectionList.h>
#include <maya/MFileIO.h>
#include <maya/MFnTransform.h>
#include <maya/MNamespace.h>
// the ones added
#include <maya/MFnIkJoint.h> 
#include <maya/MFnAnimCurve.h>
#include <maya/MDagPath.h>


#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <vector>
#include <ios>

//This is the backbone for creating a MPxFileTranslator
class BvhTranslator : public MPxFileTranslator {
public:

    //Constructor
    BvhTranslator() {};
    //Destructor
    ~BvhTranslator() override {};

    //This tells maya that the translator can read files.
    //Basically, you can import or load with your translator.
    bool haveReadMethod() const override { return true; }

    //This tells maya that the translator can write files.
    //Basically, you can export or save with your translator.
    bool haveWriteMethod() const override { return true; }

    //If this method returns true, and the Bvh file is referenced in a scene, the write method will be
    //called when a write operation is performed on the parent file.  This use is for users who wish
    //to implement a custom file referencing system.
    //For this example, we will return false as we will use Maya's file referencing system.
    bool haveReferenceMethod() const override { return false; }

    //If this method returns true, it means we support namespaces.
    bool haveNamespaceSupport()    const override { return true; }

    //This method is used by Maya to create instances of the translator.
    static void* creator();

    //This returns the default extension ".Bvh" in this case.
    MString defaultExtension() const override;

    //If this method returns true it means that the translator can handle opening files 
    //as well as importing them.
    //If the method returns false then only imports are handled. The difference between 
    //an open and an import is that the scene is cleared(e.g. 'file -new') prior to an 
    //open, which may affect the behaviour of the translator.
    bool canBeOpened() const override { return true; }

    //Maya will call this method to determine if our translator
    //is capable of handling this file.
    MFileKind identifyFile(const MFileObject& fileName,
        const char* buffer,
        short size) const override;

    //This function is called by maya when import or open is called.
    MStatus reader(const MFileObject& file,
        const MString& optionsString,
        MPxFileTranslator::FileAccessMode mode) override;

    //This function is called by maya when export or save is called.
    MStatus writer(const MFileObject& file,
        const MString& optionsString,
        MPxFileTranslator::FileAccessMode mode) override;

private:
    //The magic string to verify it's a Bvh file
    //simply "<Bvh>"
    static MString const magic;
};

//Creates one instance of the BvhTranslator
void* BvhTranslator::creator()
{
    return new BvhTranslator();
}

// Initialize our magic string
MString const BvhTranslator::magic("HIERARCHY");


//Uisng the notation conversion fromp the Maya python API
std::string maya_convert_notation(std::string str) 
{
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
    if (str == "Xposition") {
        return "translateX";
    }
    else if (str == "Yposition") {
        return "translateY";
    }
    else if (str == "Zposition") {
        return "translationZ";
    }
    else if (str == "Xrotation") {
        return "rotateX";
    }
    else if (str == "Yrotation") {
        return "rotateY";
    }
    else if (str == "Zrotation") {
        return "rotateZ";
    }
    return "";
}

// Understanding chars or strings seems to fail
// Let's do this manually
// Convert the Mstring to an Mstring (takes an Mstring as input)
// Tkaen from a C tutorial for stripping words from a string
MStringArray split_space(const MStringArray& array)
{
    MString tmp_string;
    const char* curr_chaine = NULL;
    unsigned int curr_length = 0;
    MStringArray ret(array.length(), MString(""));

    for (unsigned int i = 0; i < array.length(); i++) {

        std::string curr_string(array[i].asChar());
        std::string final;
        for (int j = 0; j < curr_string.length(); j++) {
            if ((curr_string[j] != ' ') && (curr_string[j] != '\n') &&
                (curr_string[j] != '\0') && (curr_string[j] != '\t')) {
                final += curr_string[j];
            }
        }
        ret[i] = MString(final.c_str());
    }
    return ret;
}

//  Try comparing arrays to mstring
bool contains_mstring(std::string str1, std::string str2) {
    if (str1.find(str2) != std::string::npos) {
        return true;
    }
    return false;
}


#define dump(a) std::cout << #a " : " <<"|"<<(a)<<"|" << std::endl;
#define detail(a) std::cout << "["; for (auto s : line){ std::cout << s << "|";} std::cout << "]\n";

bool parseJoint1(const MFileObject& file) 
{
    const MString fname = file.fullName();
    
    // No std vectors, have to include them
    std::vector<std::string> buffer;

    std::ifstream inputfile(fname.asChar(), ios::in);

    // skip first line (hierarchy)
    std::string line;

    std::getline(inputfile, line);

    bool isMotion = false;

    // premature Stopping conditions

    //if (line != "HIERARCHY ")
    if(line.find("HIERARCHY ") != std::string::npos)
    {
        cerr << fname << ":does not contain a hierarchy" << std::endl;
        return MS::kFailure;
    }

    // open failed
    if (!inputfile) {
        cerr << fname << ":does not exist or was imposisible to open"<<std::endl;
        return MS::kFailure;
    }

   

    // todo : pile de parent pour garder en mémoire les anciens

    MStringArray channel_array;
    std::string value;
    double offx, offy, offz;

    std::string xPos("Xposition"), yPos("Yposition"), zPos("Zposition"), xRot("Xrotation"), yRot("Yrotation"), zRot("Zrotation");

    bool close_flag = false;
    bool motion_frame = false;

    MObject rootNode = MObject::kNullObj;
    MObject current_parent = MObject::kNullObj;

    //Using the same logic as the python script
    std::vector<MString> channel_lst;

    // The temp variables
    MStatus ret;
    MFnIkJoint mfn_joint;
    int time_frame = 0;
    //https://download.autodesk.com/us/maya/2011help/API/class_m_fn_anim_curve.html
    MFnAnimCurve animcurve_tab[96]; // 96 columns in bvh, did not find array structire with append-like method


    int i(0);


    std:: cout << "============== Started the reading ==============="<<std::endl;
    while (std::getline(inputfile, line)) {
        
        cerr<<"=="<<std::endl;
        //cerr<<line<<std::endl;

        // Just to test cuz I could not find variable size of line to work
        char buffer[2000];
        strcpy(buffer, line.c_str());

        // Cannot set non chars
        MString cmdString;
        cmdString.set(buffer);

        MStringArray curr_line_array;
        cmdString.split(' ', curr_line_array);
        curr_line_array = split_space(curr_line_array);

        //dump(curr_line_array)

        bool end_site = false;
        
        // Surpress this
        /*
        std::istringstream isub(line);
        std::string label;
        // Get the label
        isub >> label;
        detail(label)
        */

        if (!isMotion)
        {
             //dump("In hierarchy")

             //dump(curr_line_array[0])

            //detail(curr_line_array[0])
            
            
            // In the same fashion of the pytho script
            if (curr_line_array[0] == "ROOT")
            {
                dump("inside root")

                mfn_joint.create(MObject::kNullObj);

                // change all strings to Mstring (does not accept it otherwise) Maya is bizzare
                mfn_joint.setName(curr_line_array[1]);
                
                current_parent = mfn_joint.object();
                
                rootNode = mfn_joint.object();
            }
            else if (curr_line_array[0] == "Frames:")
            {
                dump("inside motion")
                isMotion = true; // stop for now
            
            }
            else if (curr_line_array[0] == "OFFSET") 
            {
                offx = atof(curr_line_array[1].asChar());
                offy = atof(curr_line_array[2].asChar());
                offz = atof(curr_line_array[2].asChar());

                dump("inside offset")

                dump(offx)
                dump(offy)
                dump(offz)

                mfn_joint.setObject(current_parent);

                MString joint_name = mfn_joint.name();
                
                if (close_flag) 
                {
                    joint_name = joint_name + MString("_end");
                }

                if (!current_parent.isNull()) 
                {
                    mfn_joint.setRotationOrder(MTransformationMatrix::RotationOrder::kXYZ, false);
                    mfn_joint.setTranslation(MVector(offx, offy, offz), MSpace::kTransform);
                }
            }
            else if (curr_line_array[0] == "CHANNELS")
            {
                dump("inside channels")

                dump(curr_line_array)

                int n(atof(curr_line_array[1].asChar()));

                dump(n)

                // degrees of freedom

                bool dofX(false), dofY(false), dofZ(false), dofXr(false), dofYr(false), dofZr(false);

                for (int i = 0; i < curr_line_array.length() - 2; i++)
                {
                    std::string dofName = curr_line_array[i+2].asChar();

                    //dofName.erase(std::remove(dofName.begin(), dofName.end(), ' '), dofName.end());

                    dump(dofName)

                    bool found = false;

                    if (dofName == "Xrotation") {
                        dofXr = true;
                        found = true;
                    }
                    else if (dofName == yRot) {
                        dofYr = true;
                        found = true;
                    }
                    else if (dofName == zRot) {
                        dofZr = true;
                        found = true;
                    }
                    if (dofName == xPos) {
                        dofX = true;
                        found = true;
                    }
                    else if (dofName == yPos) {
                        dofY = true;
                        found = true;
                    }
                    else if (dofName == zPos) {
                        dofZ = true;
                        found = true;
                    }

                    if (found) {
                        dump(maya_convert_notation(dofName))
                        dump(MString(maya_convert_notation(dofName).c_str()))
                        channel_lst.push_back(MString(maya_convert_notation(dofName).c_str()));
                    }
                    else {
                        dofName = "Xrotation";
                        dump(maya_convert_notation(dofName))
                        dump(MString(maya_convert_notation(dofName).c_str()))
                        channel_lst.push_back(MString(maya_convert_notation(dofName).c_str()));
                    }
                }
                mfn_joint.setDegreesOfFreedom(dofX, dofY, dofZ);
            }
            else if(curr_line_array[0] == "JOINT")
            {
                dump("inside joint")

                i = 0;

                mfn_joint.create(current_parent, &ret);
                mfn_joint.setName(curr_line_array[1]);
                //set inside channel
                channel_array.append(curr_line_array[1]);
                current_parent = mfn_joint.object();


                // To convert the current parent sub-object to joint
                MFnIkJoint joint_handle(mfn_joint.parent(0));
                MString name = joint_handle.name();

                std::cout << "============***==============================================" << std::endl;
                std::cout << "The joint " << mfn_joint.name() << " has parent " << name << std::endl;
                std::cout << "============***==============================================" << std::endl;


            }
             /*
            else if (curr_line_array[0] == "END")
            {
                end_site = true;
            }
            */
            else if (*curr_line_array[0].asChar() == '}')
            //else if (end_site)
            {
                dump("inside }")

                if (close_flag) {
                    close_flag = false; // signal the end and the recursive ascend (or la pile, at the time of me writin gthis I don't know what I used
                    continue;
                }

                MFnIkJoint joint_handle(current_parent);
                MObject temp = joint_handle.parent(0);
                MFnIkJoint temp_joint(temp);

                //if(joint_handle.parent(0) != MObject::kNullObj) 
                //if (*temp_joint.name().asChar() != '\0') //empty char
                if (*temp_joint.name().asChar() != '\0' && i<4) //empty char
                { // if not none
                    // we go one                    
                    current_parent = joint_handle.parent(0); // this assumes only one parent which is normal, I hope
                    
                    MFnIkJoint j1(current_parent);
                    MFnIkJoint j2(j1.parent(0));

                    std::cout<< joint_handle.name()<<" has parent "<< j1.name()<<" whose parent is "<< j2.name()<<std::endl;
                        
                    i += 1;

                    dump(i)

                    //MFnIkJoint joint_handle1(current_parent);

                    /*
                    if (joint_handle1.parent(1) != MObject::kNullObj)
                    {
                        current_parent = joint_handle1.parent(1);
                    }
                    */

                }
                std::cout << joint_handle.name() << "is the current parent" << std::endl;
                end_site = false;

            }
            else {
                cerr << "Unknown label" << std::endl;
                dump(curr_line_array[0].asChar())
            }
        }
        else {
            std::cout << "In motion" << std::endl;
            // skip first two lines of motion section
            if (curr_line_array[0] == "Frames:")
            {
                continue;
            }
            if (curr_line_array[0] == "Frames:")
            {
                continue;
            }
            MTime maya_time((double)(time_frame), MTime::kFilm);
            // Start reading
            MSelectionList curr_selection; // buff to store object we select using their names
            MObject curr_attribute; // buff store attribute for the current object considered (ie translationX)
            MObject curr_attribute_rotate;

            //Testign
            MDagPath node;
            MObject component; //buff
            MFnDagNode nodeFn; // buff

            dump(curr_line_array)
            dump(curr_line_array.length())

            dump(channel_lst.size())
            dump(channel_array.length())


            for(auto e : channel_array)
            {
                dump(e.asChar())
            }
            ;

            for(int i =0; i < curr_line_array.length(); i++)
            {
                int itercount = 0;


                std::string name = "";
                name = channel_array[itercount].asChar();
                dump(channel_array[itercount])


                // for each channel and this for one frame
                cerr << "selecting by name object " << name << endl;

                //Strip all 
                name.erase(std::remove_if(name.begin(), name.end(), isspace), name.end());
                MString name_m(name.c_str());

                ret = MGlobal::getSelectionListByName(name_m, curr_selection);

                MItSelectionList iter(curr_selection, MFn::kJoint, &ret);

                curr_selection.getDagPath(i, node, component);
                nodeFn.setObject(node);
                cout << nodeFn.name().asChar() << " is selected" << endl;
                

                MFnIkJoint mfn_test;
                MObject curr_obj;



                iter.getDependNode(curr_obj); // get the MObject node (result in curr_obj) that correspond to current iterator value
                mfn_test.setObject(curr_obj); // bind MFnIkJoint function set on curr_obj, used to retrieve the attribute by their names

                
                // iterate through the selection (active list)
                for (; !iter.isDone(); iter.next())
				{
                    dump("inside iter")
					

                    dump(mfn_test.name())

					if (i < 3)
                    { 
                        // Root has 6 channels, the rest only 3

						if ((i % 3) == 0) {
						
							if (time_frame == 0) {

								curr_attribute = mfn_test.attribute("translateX", &ret);
								if (ret != MStatus::kSuccess) {
									cerr << "FAILED TO RETRIEVE ATTRIBUTE TRANSLATEX OF HIP" << endl;
								}

								// if first time we do the keyframe processing (ie time_frame = 0)
								// then we have not yet created the animcurves
								animcurve_tab[i].create(curr_obj, curr_attribute, NULL, &ret);
								if (ret != MStatus::kSuccess) {
									cerr << "FAILED TO CREATE ATTRIBUTE TRANSLATEX OF HIP" << endl;
								}
							}

							animcurve_tab[i].addKeyframe(maya_time, atof(curr_line_array[i].asChar()));
						}
						else if ((i % 3) == 1) {

							if (time_frame == 0) {
								curr_attribute = mfn_test.attribute("translateY", &ret);
								animcurve_tab[i].create(curr_obj, curr_attribute, NULL, &ret);
							}

							animcurve_tab[i].addKeyframe(maya_time, atof(curr_line_array[i].asChar()));
						}
						else if ((i % 3) == 2) {
							
							if (time_frame == 0) {
								curr_attribute = mfn_test.attribute("translateZ", &ret);
								animcurve_tab[i].create(curr_obj, curr_attribute, NULL, &ret);
							}

							MStatus test_status = animcurve_tab[i].addKeyframe(maya_time, atof(curr_line_array[i].asChar()));
							if (test_status != MStatus::kSuccess) {
								cerr << "ERROR SETTING TZ KEYFRAME" << endl;
							}
						}
					}
					else {
						if ((i % 3) == 0) {

							if (time_frame == 0) {
								curr_attribute_rotate = mfn_test.attribute("rotateZ", &ret);
								if (ret != MStatus::kSuccess) {
									cerr << "FAILED TO RETRIEVE ATTRIBUTE ROTATEZ" << endl;
								}

								animcurve_tab[i].create(curr_obj, curr_attribute_rotate, NULL, &ret);
							}

							MStatus rz_status = animcurve_tab[i].addKeyframe(maya_time, atof(curr_line_array[i].asChar())*(3.14 / 180));

							if (rz_status != MStatus::kSuccess) {
								cerr << "ERROR SETTING RZ KEYFRAME" << endl;

							}
						}
						else if ((i % 3) == 1) {

							if (time_frame == 0) {
								curr_attribute_rotate = mfn_test.attribute("rotateY", &ret);
								animcurve_tab[i].create(curr_obj, curr_attribute_rotate, NULL, &ret);
							}

							animcurve_tab[i].addKeyframe(maya_time, atof(curr_line_array[i].asChar())*(3.14 / 180));
						}
						else if ((i % 3) == 2) {

							if (time_frame == 0) {
								curr_attribute_rotate = mfn_test.attribute("rotateX", &ret);
								animcurve_tab[i].create(curr_obj, curr_attribute_rotate, NULL, &ret);
							}

							animcurve_tab[i].addKeyframe(maya_time, atof(curr_line_array[i].asChar())*( 3.14 / 180));
						}
					}
					itercount += 1;
				}
				
                /*
                if (itercount > 1) {
					// for our code to work, we for now suppose each joint
					// has a different name since we do not use the
					// fullPathName to retrieve them, but only their
					// .name() (for example "l_hip"
					cerr << "==== ERROR TWO OBJECTS HAVE SAME NAME " << endl;
				}
                */
                
			}
			time_frame += 1; // increase time
        }


       
        

    }
}


/*
MFnIkJoint* parseJoint(std::ifstream& inputfile) {
    MStatus status;
    MFnIkJoint* root = new MFnIkJoint();
    MObject transform = current->create(MObject::kNullObj, &status);
    std::istringstream isub(line);

    std::string nextword << isub;

    std::string line;
    std::getline(inputfile, line);

    while (std::getline(inputfile, line)) {
        std::istringstream isub(line);
        std::string label;
        isub >> label;

        if (label == std::string("OFFSET")) {
            double x, y, z;
            isub >> x >> y >> z;
            current->setTranslation(MVector(x, y, z), MSpace::kObject);
        }
        else if (label == std::string("CHANNELS")) {
            int n;
            isub >> n;
            for (int i = 0; i < n; i++) {
                //animation
            }
        }
        else if (label == std::string("JOINT")) {
            std::string name;
            isub >> name;
            MString nameMaya = name.c_str();
            MObject childTransform = child->create(MObject::kNullObj, &status);
            status = current->addChild(childTransform, MFnDagNode::kNextPos, false);
        }
        else if (label == std::string("}")) {
            return current;
        }
        else {
            throw std::runtime_error("Unknown label");
        }

    }
}
*/


// An Bvh file is an ascii whose first line contains the string .
//
MStatus BvhTranslator::reader(const MFileObject& file,
    const MString& options,
    MPxFileTranslator::FileAccessMode mode)
{
    const MString fname = file.expandedFullName();

    std:: cout << fname << std:: endl;

    MString test;
    test.set("{");
    dump(test.asChar())
    dump("{")
    bool b = strcmp(test.asChar(), "}");
    dump(b)

    std::cout << "==========================================================" << std::endl;
    std::cout << "==========================================================" << std::endl;
    std::cout << "==========================================================" << std::endl;
    std::cout << "==========================================================" << std::endl;
    std::cout << "============***==============================================" << std::endl;
    std::cout << "============***==============================================" << std::endl;
    std::cout << "============***==============================================" << std::endl;
    std::cout << "============***==============================================" << std::endl;
    std::cout << "============***==============================================" << std::endl;
    std::cout << "============***==============================================" << std::endl;
    std::cout << "============***==============================================" << std::endl;
    std::cout << "============***==============================================" << std::endl;
    std::cout << "============***==============================================" << std::endl;
    std::cout << "============***==============================================" << std::endl;
    std::cout << "============***==============================================" << std::endl;
    std::cout << "============***==============================================" << std::endl;
    std::cout << "============***==============================================" << std::endl;
    std::cout << "============***==============================================" << std::endl;
    std::cout << "============***==============================================" << std::endl;
    std::cout << "============***==============================================" << std::endl;
    std::cout << "============***==============================================" << std::endl;

    MStatus rval(MS::kSuccess);
    // TODO
    bool ret = parseJoint1(file);

    dump("The reading is done")


    return rval;
}

// The currently recognised primitives.
const char* primitiveStrings[] = {
    "nurbsSphere",
    "nurbsCone",
    "nurbsCylinder",
};
const unsigned numPrimitives = sizeof(primitiveStrings) / sizeof(char*);

// Corresponding commands to create the primitives
const char* primitiveCommands[] = {
    "sphere",
    "cone",
    "cylinder",
};

//The writer simply goes gathers all objects from the scene.
//We will check if the object has a transform, if so, we will check
//if it's either a nurbsSphere, nurbsCone or nurbsCylinder.  If so,
//we will write it out.
MStatus BvhTranslator::writer(const MFileObject& file,
    const MString& options,
    MPxFileTranslator::FileAccessMode mode)
{
    MStatus status;
    bool showPositions = false;
    unsigned int  i;
    const MString fname = file.expandedFullName();

    std::ofstream newf(fname.asChar(), std::ios::out);
    if (!newf) {
        // open failed
        std::cerr << fname << ": could not be opened for reading\n";
        return MS::kFailure;
    }
    newf.setf(std::ios::unitbuf);

    if (options.length() > 0) {
        // Start parsing.
        MStringArray optionList;
        MStringArray theOption;
        options.split(';', optionList);    // break out all the options.

        for (i = 0; i < optionList.length(); ++i) {
            theOption.clear();
            optionList[i].split('=', theOption);
            if (theOption[0] == MString("showPositions") &&
                theOption.length() > 1) {
                if (theOption[1].asInt() > 0) {
                    showPositions = true;
                }
                else {
                    showPositions = false;
                }
            }
        }
    }

    // output our magic number
    newf << "HIERARCHY\n";

    MItDag dagIterator(MItDag::kBreadthFirst, MFn::kInvalid, &status);

    if (!status) {
        status.perror("Failure in DAG iterator setup");
        return MS::kFailure;
    }

    MSelectionList selection;
    MGlobal::getActiveSelectionList(selection);
    MItSelectionList selIterator(selection, MFn::kDagNode);

    bool done = false;
    while (true)
    {
        MObject currentNode;
        switch (mode)
        {
        case MPxFileTranslator::kSaveAccessMode:
        case MPxFileTranslator::kExportAccessMode:
            if (dagIterator.isDone())
                done = true;
            else {
                currentNode = dagIterator.currentItem();
                dagIterator.next();
            }
            break;
        case MPxFileTranslator::kExportActiveAccessMode:
            if (selIterator.isDone())
                done = true;
            else {
                selIterator.getDependNode(currentNode);
                selIterator.next();
            }
            break;
        default:
            std::cerr << "Unrecognized write mode: " << mode << std::endl;
            break;
        }
        if (done)
            break;

        //We only care about nodes that are transforms
        MFnTransform dagNode(currentNode, &status);
        if (status == MS::kSuccess)
        {
            MString nodeNameNoNamespace = MNamespace::stripNamespaceFromName(dagNode.name());
            for (i = 0; i < numPrimitives; ++i) {
                if (nodeNameNoNamespace.indexW(primitiveStrings[i]) >= 0) {
                    // This is a node we support
                    newf << primitiveCommands[i] << " -n " << nodeNameNoNamespace << std::endl;
                    if (showPositions) {
                        MVector pos;
                        pos = dagNode.getTranslation(MSpace::kObject);
                        newf << "move " << pos.x << " " << pos.y << " " << pos.z << std::endl;
                    }
                }
            }
        }//if (status == MS::kSuccess)
    }//while loop

    newf.close();
    return MS::kSuccess;
}

// Whenever Maya needs to know the preferred extension of this file format,
// it calls this method. For example, if the user tries to save a file called
// "test" using the Save As dialog, Maya will call this method and actually
// save it as "test.Bvh". Note that the period should *not* be included in
// the extension.
MString BvhTranslator::defaultExtension() const
{
    return "Bvh";
}


//This method is pretty simple, maya will call this function
//to make sure it is really a file from our translator.
//To make sure, we have a little magic number and we verify against it.
MPxFileTranslator::MFileKind BvhTranslator::identifyFile(
    const MFileObject& fileName,
    const char* buffer,
    short size) const
{
    // Check the buffer for the "Bvh" magic number, the
    // string "<Bvh>\n"

    MFileKind rval = kNotMyFileType;

    if ((size >= (short)magic.length()) &&
        (0 == strncmp(buffer, magic.asChar(), magic.length())))
    {
        rval = kIsMyFileType;
    }
    return rval;
}

MStatus initializePlugin(MObject obj)
{
    MStatus   status;
    MFnPlugin plugin(obj, PLUGIN_COMPANY, "3.0", "Any");

    // Register the translator with the system
    // The last boolean in this method is very important.
    // It should be set to true if the reader method in the derived class
    // intends to issue MEL commands via the MGlobal::executeCommand 
    // method.  Setting this to true will slow down the creation of
    // new objects, but allows MEL commands other than those that are
    // part of the Maya Ascii file format to function correctly.
    status = plugin.registerFileTranslator("Bvh",
        "BvhTranslator.rgb",
        BvhTranslator::creator,
        "BvhTranslatorOpts",
        "showPositions=1",
        true);
    if (!status)
    {
        status.perror("registerFileTranslator");
        return status;
    }

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus   status;
    MFnPlugin plugin(obj);

    status = plugin.deregisterFileTranslator("Bvh");
    if (!status)
    {
        status.perror("deregisterFileTranslator");
        return status;
    }

    return status;
}


