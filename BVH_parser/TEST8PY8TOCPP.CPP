// Initialize our magic string
MString const LepTranslator::magic("HIERARCHY");


// Maybe do a function of contain contains_mstring(MStringArray &, std::string)


// Do not forget the maya notation MString maya_notation(MString )

// How to maje a koint
/*
MStatus make_joint(MObject &parent, MObject &new_obj, MString name)
{
	MStatus ret;
	MFnIkJoint mfn_joint;
	mfn_joint.create(parent, &ret);
	mfn_joint.setName(name);
	new_obj = mfn_joint.object();
	return ret;
}
*/


/*
while going through lines
    if not motion frame
        if curr_parent != MObject::kNullObj
            if root
                mfn_joint.create(MObject::kNullObj);
				mfn_joint.setName(curr_line_array[1]);
				curr_parent = mfn_joint.object();
				rootNode = mfn_joint.object();
            if joint
                mfn_joint.create(curr_parent, &ret);
				mfn_joint.setName(curr_line_array[curr_line_array.length() - 1]);
				curr_parent = mfn_joint.object();
            if channels
                mfn_util.setObject(curr_parent);
                MString tmp = mfn_util.name(); 

                fullPathName() + MString(".") + MString(maya_notation(curr_line_array[i + 2]))
                
                channel_lst <--- tmp
            if offset
                mfn_joint.setObject(curr_parent)
				MString joint_name = mfn_joint.name()

                if(closed flag) : add end to name

                if(current parent not null): 
                    mfn_joint.setRotationOrder(MTransformationMatrix::RotationOrder::kXYZ, false); // reorder = false?
					mfn_joint.setTranslation(MVector, MSpace::kTransform);
            if }
                close flag
    if motion
        for each line
            MSelectionList curr_selection;
				MGlobal::getSelectionListByName //find by name (not fullPathName?
				MItSelectionList iter(curr_selection, MFn::kJoint, &ret);
				MFnIkJoint mfn_test;
				MObject curr_obj;
				int itercount = 0;

				for (; !iter.isDone(); iter.next())
				{
					iter.getDependNode(curr_obj); // get the MObject node (result in curr_obj) that correspond to current iterator value
					mfn_test.setObject(curr_obj); // bind MFnIkJoint function set on curr_obj, used to retrieve the attribute by their names

					if (i < 3) { // because root node has 6 channels, the 3 first being translations, the first 3 are translations
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

							MStatus rz_status = animcurve_tab[i].addKeyframe(maya_time, atof(curr_line_array[i].asChar())*(3.1415 / 180));
							if (rz_status != MStatus::kSuccess) {
								cerr << "ERROR SETTING RZ KEYFRAME" << endl;

							}
						}
						else if ((i % 3) == 1) {

							if (time_frame == 0) {
								curr_attribute_rotate = mfn_test.attribute("rotateY", &ret);
								animcurve_tab[i].create(curr_obj, curr_attribute_rotate, NULL, &ret);
							}

							animcurve_tab[i].addKeyframe(maya_time, atof(curr_line_array[i].asChar())*(3.1415/180));
						}
						else if ((i % 3) == 2) {

							if (time_frame == 0) {
								curr_attribute_rotate = mfn_test.attribute("rotateX", &ret);
								animcurve_tab[i].create(curr_obj, curr_attribute_rotate, NULL, &ret);
							}

							animcurve_tab[i].addKeyframe(maya_time, atof(curr_line_array[i].asChar())*( 3.1415 / 180));
						}
					}
*/