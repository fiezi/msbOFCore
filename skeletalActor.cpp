

#include "skeletalActor.h"
#include "renderer.h"
#include "input.h"
#include "boneActor.h"
#include "meshData.h"

SkeletalActor::SkeletalActor(){

drawType=DRAW_VBOMESH;
sceneShaderID="color";
bUseShader=true;

vboMeshID="teapot";


boneTransforms=NULL;
invBoneTransforms=NULL;


bPhysicsBones=false;
bDelayedConvert=false;

bInit=false;

/*
menuType.push_back("12AssignButton");
menuProp.push_back("NULL");
menuIcon.push_back("icon_base");
menuName.push_back("make Physics");
*/

menuType.push_back("12AssignButton");
menuProp.push_back("NULL");
menuIcon.push_back("icon_base");
menuName.push_back("reset");

registerProperties();
}

SkeletalActor::~SkeletalActor(){}

void SkeletalActor::registerProperties(){

createMemberID("BONES",&bones,this);
createMemberID("BPHYSICSBONES",&bPhysicsBones,this);
Actor::registerProperties();
}


void SkeletalActor::postLoad(){


    //always remove bones here if we have any!
    //NO DON'T!! It will kill all references!
    /*
    for (int i=0;i<(int)bones.size();i++){
        bones[i]->remove();
    }
    bones.clear();
    */
    cout << "SkeletalActor Name: " << name << endl;

	if (renderer->vboList[vboMeshID]->boneCount>0){
		boneTransforms=new Matrix4f[renderer->vboList[vboMeshID]->boneCount];
		invBoneTransforms=new Matrix4f[renderer->vboList[vboMeshID]->boneCount];
	}
    int myPos=0;

    //find my position in actorList
    for (int i=0;i<(int)renderer->actorList.size();i++){
        if (renderer->actorList[i]==this){
            myPos=i;
        }
    }


    //spawn bones
    for (int i=0; i< renderer->vboList[vboMeshID]->boneCount; i++){

        boneTransforms[i].identity();
        invBoneTransforms[i]=*renderer->vboList[vboMeshID]->bones[i]->invBoneMatrix;
        if ((int)bones.size()==i){
            //insert before character?
            //renderer->actorList.insert(renderer->actorList.begin()+ myPos+i, bones[i]);
                bones.push_back(spawn("9BoneActor"));
                bones[i]->name=renderer->vboList[vboMeshID]->bones[i]->name;
            bones[i]->bRemoveable=false;
            bones[i]->drawType=DRAW_CUBE;
            //bones[i]->scale=bones[i]->scale*0.25;
            bones[i]->base=this;

            }

        bones[i]->originalMatrix=*renderer->vboList[vboMeshID]->bones[i]->boneMatrix;

        //cout << "created new bone: " << bones[i]->name << endl;

        }

//connect parents...
    for (int i=0; i< renderer->vboList[vboMeshID]->boneCount; i++){
        for (int j=0;j<renderer->vboList[vboMeshID]->boneCount;j++){
            if (renderer->vboList[vboMeshID]->bones[i]->parentBone){
                if (renderer->vboList[vboMeshID]->bones[j]==renderer->vboList[vboMeshID]->bones[i]->parentBone){
                    cout << "connecting " << bones[i]->name << "  with " << bones[j]->name <<endl;
                    bones[i]->base=bones[j];
                    j=100;
                }
            }
            else{
              bones[i]->base=this;
              //bones[i]->baseMatrix=bones[i]->originalMatrix*renderer->inverseCameraMatrix;
            }
        }
    }

    //setup all bones
    for (int i=0; i<(int)bones.size();i++){
        bones[i]->baseMatrix=calcMatrix(bones[i]);
        bones[i]->matrixToVectors();
        bones[i]->parent=this;
        ((BoneActor*)bones[i])->bDoIK=true;
    }

    if (bPhysicsBones){
        bDelayedConvert=true;
    }

    Actor::postLoad();
}

void SkeletalActor::setup(){

    Actor::setup();

    if (!bInit)
        postLoad();


}


void SkeletalActor::trigger(Actor * other){

    if (other && other->name=="make Physics")
        convertToPhysicsBones();

    if (other && other->name=="reset")
        reset();
}

void SkeletalActor::update(double deltaTime){

    Actor::update(deltaTime);

}

void SkeletalActor::updateShaders(){

    GLint uniform_location=0;

    //remove our own transformations from the bone transforms!
    Matrix4f initialMatrix = baseMatrix.inverse();

    //make sure we only update stuff if we actually have bones running!
    if (drawType==DRAW_PARTICLES || !renderer->vboList[vboMeshID]->bIsSkeletal){
        SkeletalDrawing::updateShaders();
        return;
    }


    for (uint i=0;i<bones.size();i++){

        boneTransforms[i]=  initialMatrix * bones[i]->baseMatrix * *renderer->vboList[vboMeshID]->bones[i]->invBoneMatrix * *renderer->vboList[vboMeshID]->bindShapeMatrix;
    }


    if (renderer->vboList[vboMeshID]){
        uniform_location = glGetUniformLocation(renderer->shaderList[sceneShaderID]->shader, "boneTransforms");
        glUniformMatrix4fv(uniform_location,renderer->vboList[vboMeshID]->boneCount,false,(GLfloat*)boneTransforms[0]);
        }

    Actor::updateShaders();


}


void SkeletalActor::start(){

    for (int i=0;i<(int)bones.size();i++){
        bones[i]->bHidden=true;
    }
}

void SkeletalActor::stop(){

    Actor::stop();


    for (int i=0;i<(int)bones.size();i++){
        bones[i]->bHidden=false;
    }
}

void SkeletalActor::reset(){

for (int i=0;i<(int)bones.size();i++)
    bones[i]->remove();

bones.clear();
bInit=false;
//bPhysicsBones=false;
setup();
}

void SkeletalActor::remove(){

    //are now removed by itself!


    for (int i=(int)bones.size()-1;i>=0;i--){
        if (bones[i])
            bones[i]->remove();
        }

    bones.clear();

    Actor::remove();
}


void SkeletalActor::convertToPhysicsBones(){

}

void SkeletalActor::create(){renderer->addActor(this);}
